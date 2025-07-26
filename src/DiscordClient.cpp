#include "DiscordClient.h"
#include "CommandSystem.h"
#include "config.h"

// Global instance
DiscordClient discordClient;
DiscordClient* DiscordClient::instance = nullptr;

DiscordClient::DiscordClient() : 
  sequenceNumber(0),
  lastHeartbeat(0),
  heartbeatInterval(45000), // Default 45 seconds
  lastReconnectAttempt(0),
  lastReadyTime(0),
  connectionAttempts(0),
  lastConnectionTime(0),
  isConnected(false),
  isAuthenticated(false) {
  instance = this; // Set static instance for callback
}

void DiscordClient::begin() {
  httpClient.setInsecure(); // Skip SSL certificate verification for testing
  Serial.println("Discord client initialized");
  
  // Get Gateway URL from Discord API
  getGatewayUrl();
  
  // Connect to WebSocket
  connectWebSocket();
}

void DiscordClient::update() {
  webSocket.loop();
  
  // Send heartbeat if needed (send slightly before interval to avoid timeout)
  if (isConnected && isAuthenticated && millis() - lastHeartbeat >= (heartbeatInterval * 0.9)) {
    sendHeartbeat();
  }
  
  // TEMPORARILY DISABLE AUTO-RECONNECTION to avoid Discord rate limiting
  // Only reconnect manually if we've been disconnected for more than 5 minutes
  /*
  if (!isConnected && 
      millis() - lastReconnectAttempt >= 300000) { // 5 minutes
    Serial.println("Long-term reconnect attempt...");
    connectWebSocket();
    lastReconnectAttempt = millis();
  }
  */
}

void DiscordClient::getGatewayUrl() {
  Serial.println("Getting Discord Gateway URL...");
  
  HTTPClient http;
  http.begin(httpClient, "https://discord.com/api/v10/gateway");
  http.addHeader("Authorization", "Bot " + String(DISCORD_BOT_TOKEN));
  http.setTimeout(10000);
  
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String response = http.getString();
    Serial.println("Gateway response: " + response);
    
    JsonDocument doc;
    if (deserializeJson(doc, response) == DeserializationError::Ok) {
      gatewayUrl = doc["url"].as<String>();
      Serial.println("Gateway URL: " + gatewayUrl);
    } else {
      Serial.println("Failed to parse gateway response");
      gatewayUrl = "wss://gateway.discord.gg"; // Fallback
    }
  } else {
    Serial.println("Failed to get gateway URL, using default");
    gatewayUrl = "wss://gateway.discord.gg"; // Fallback
  }
  
  http.end();
}

void DiscordClient::connectWebSocket() {
  connectionAttempts++;
  lastConnectionTime = millis();
  
  Serial.print("Connecting to Discord Gateway... (Attempt #");
  Serial.print(connectionAttempts);
  Serial.println(")");
  
  // Prevent too frequent connection attempts
  if (connectionAttempts > 1) {
    unsigned long minInterval = 5000 * connectionAttempts; // Progressive delay
    if (minInterval > 300000) minInterval = 300000; // Cap at 5 minutes
    
    if (millis() - lastConnectionTime < minInterval) {
      Serial.print("Too many connection attempts, waiting ");
      Serial.print((minInterval - (millis() - lastConnectionTime)) / 1000);
      Serial.println(" more seconds...");
      return;
    }
  }
  
  // Extract host and path from gateway URL
  String host = gatewayUrl;
  host.replace("wss://", "");
  
  webSocket.beginSSL(host, 443, "/?v=10&encoding=json");
  webSocket.onEvent([](WStype_t type, uint8_t * payload, size_t length) {
    if (DiscordClient::instance) {
      DiscordClient::instance->handleWebSocketEvent(type, payload, length);
    }
  });
  
  // Set WebSocket timeouts and ping settings for better stability
  webSocket.setReconnectInterval(5000);
  webSocket.enableHeartbeat(15000, 3000, 2); // ping every 15s, timeout 3s, disconnect after 2 failures
}

void DiscordClient::handleWebSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED: {
      Serial.print("WebSocket Disconnected - Code: ");
      Serial.println((int)type);
      Serial.print("Payload length: ");
      Serial.println(length);
      if (length > 0) {
        Serial.print("Disconnect reason: ");
        Serial.write(payload, length);
        Serial.println();
      }
      Serial.println("Connection was active for: " + String(millis() - lastHeartbeat) + "ms since last heartbeat");
      isConnected = false;
      isAuthenticated = false;
      sequenceNumber = 0;
      lastHeartbeat = 0;
      
      // Add exponential backoff to prevent rate limiting
      static unsigned long reconnectDelay = 15000; // Start with 15 seconds (more conservative)
      
      // Check if this is a Discord-initiated disconnect (code 1 with no payload)
      if (length == 0) {
        Serial.println("Discord closed connection - possible reasons:");
        Serial.println("1. All guilds unavailable (Discord outage or bot not properly added)");
        Serial.println("2. Invalid intents (check Discord Developer Portal)");
        Serial.println("3. Rate limiting from too many connections");
        Serial.println("4. Bot token issues or revoked permissions");
        reconnectDelay = 60000; // Wait 1 minute for Discord-initiated disconnects
      }
      
      Serial.print("Waiting ");
      Serial.print(reconnectDelay / 1000);
      Serial.println(" seconds before reconnect attempt...");
      
      // Use non-blocking delay to prevent watchdog reset
      unsigned long waitStart = millis();
      while (millis() - waitStart < reconnectDelay) {
        delay(100); // Small delays to feed watchdog
        yield(); // Allow other tasks to run
      }
      
      reconnectDelay = min(reconnectDelay * 2, 600000UL); // Cap at 10 minutes
      break;
    }
      
    case WStype_CONNECTED: {
      Serial.println("WebSocket Connected to Discord Gateway");
      isConnected = true;
      connectionAttempts = 0; // Reset connection attempts counter on successful connection
      lastHeartbeat = millis(); // Initialize heartbeat timer
      break;
    }
      
    case WStype_TEXT: {
      String message = String((char*)payload);
      Serial.println("Received: " + message);
      
      JsonDocument doc;
      if (deserializeJson(doc, message) == DeserializationError::Ok) {
        int opcode = doc["op"];
        
        // Update sequence number if present
        if (!doc["s"].isNull()) {
          sequenceNumber = doc["s"].as<int>();
        }
        
        switch(opcode) {
          case 10: // Hello
            heartbeatInterval = doc["d"]["heartbeat_interval"].as<unsigned long>();
            Serial.println("Heartbeat interval: " + String(heartbeatInterval) + "ms");
            // Send immediate heartbeat after getting interval
            sendHeartbeat();
            
            // Try to resume if we have a valid session, otherwise identify
            if (sessionId.length() > 0 && sequenceNumber > 0) {
              Serial.println("Attempting to resume session...");
              sendResume();
            } else {
              sendIdentify();
            }
            break;
            
          case 11: // Heartbeat ACK
            Serial.println("Heartbeat acknowledged");
            break;
            
          case 0: { // Dispatch
            if (!doc["s"].isNull()) {
              sequenceNumber = doc["s"].as<int>();
            }
            String eventType = doc["t"].as<String>();
            JsonDocument eventData = doc["d"];
            
            Serial.println("Event: " + eventType);
            handleDiscordMessage(eventType, eventData);
            break;
          }
          
          case 7: // Reconnect
            Serial.println("Discord requested reconnect");
            isConnected = false;
            break;
            
          case 9: // Invalid Session
            Serial.println("Invalid session detected");
            // Check if we can resume (resumable field in payload)
            bool canResume = doc["d"].as<bool>();
            if (!canResume) {
              Serial.println("Session not resumable, clearing session data");
              sessionId = "";
              sequenceNumber = 0;
            }
            isConnected = false;
            isAuthenticated = false;
            break;
        }
      }
      break;
    }
    
    case WStype_ERROR:
      Serial.printf("WebSocket Error: %s\n", payload);
      break;
      
    case WStype_PONG:
      Serial.println("WebSocket Pong received");
      break;
      
    default:
      break;
  }
}

void DiscordClient::sendHeartbeat() {
  if (!isConnected) {
    Serial.println("Cannot send heartbeat - not connected");
    return;
  }
  
  JsonDocument heartbeat;
  heartbeat["op"] = 1;
  heartbeat["d"] = (sequenceNumber > 0) ? sequenceNumber : JsonVariant();
  
  String heartbeatStr;
  serializeJson(heartbeat, heartbeatStr);
  
  if (webSocket.sendTXT(heartbeatStr)) {
    lastHeartbeat = millis();
    Serial.println("Heartbeat sent");
  } else {
    Serial.println("Failed to send heartbeat");
  }
}

void DiscordClient::sendIdentify() {
  JsonDocument identify;
  identify["op"] = 2;
  identify["d"]["token"] = DISCORD_BOT_TOKEN;
  identify["d"]["intents"] = 33280; // GUILD_MESSAGES (512) + MESSAGE_CONTENT (32768) = 33280
  identify["d"]["properties"]["$os"] = "ESP32";
  identify["d"]["properties"]["$browser"] = "ESP32-Discord-Bot";
  identify["d"]["properties"]["$device"] = "ESP32";
  
  String identifyStr;
  serializeJson(identify, identifyStr);
  
  Serial.println("Sending IDENTIFY with intents: 33280 (GUILD_MESSAGES + MESSAGE_CONTENT)");
  Serial.println("MESSAGE_CONTENT_INTENT should now be enabled in Developer Portal!");
  
  webSocket.sendTXT(identifyStr);
  isAuthenticated = true;
  Serial.println("Identify sent");
}

void DiscordClient::sendResume() {
  JsonDocument resume;
  resume["op"] = 6; // Resume opcode
  resume["d"]["token"] = DISCORD_BOT_TOKEN;
  resume["d"]["session_id"] = sessionId;
  resume["d"]["seq"] = sequenceNumber;
  
  String resumeStr;
  serializeJson(resume, resumeStr);
  
  webSocket.sendTXT(resumeStr);
  Serial.println("Resume sent for session: " + sessionId + " with seq: " + String(sequenceNumber));
}

void DiscordClient::handleDiscordMessage(const String& eventType, JsonDocument& data) {
  if (eventType == "MESSAGE_CREATE") {
    processMessage(data);
  } else if (eventType == "READY") {
    sessionId = data["session_id"].as<String>();
    lastReadyTime = millis();
    Serial.println("Bot is ready! Session ID: " + sessionId);
    
    // Print bot user info
    String botUsername = data["user"]["username"].as<String>();
    String botId = data["user"]["id"].as<String>();
    Serial.println("Bot user: " + botUsername + " (ID: " + botId + ")");
    
    // Check if guilds are available
    JsonArray guilds = data["guilds"];
    int unavailableCount = 0;
    Serial.println("=== GUILD STATUS ===");
    for (JsonObject guild : guilds) {
      String guildId = guild["id"].as<String>();
      bool unavailable = guild["unavailable"].as<bool>();
      if (unavailable) {
        unavailableCount++;
        Serial.println("Guild " + guildId + ": UNAVAILABLE (Discord outage or bot not in guild)");
      } else {
        Serial.println("Guild " + guildId + ": AVAILABLE");
      }
    }
    Serial.println("Guilds: " + String(guilds.size()) + " total, " + String(unavailableCount) + " unavailable");
    
    if (unavailableCount == guilds.size()) {
      Serial.println("WARNING: ALL GUILDS UNAVAILABLE - This may cause Discord to disconnect the bot");
    }
    
    Serial.println("Target channel ID: " + String(DISCORD_CHANNEL_ID));
    Serial.println("Waiting for GUILD_CREATE events...");
    
  } else if (eventType == "GUILD_CREATE") {
    String guildId = data["id"].as<String>();
    String guildName = data["name"].as<String>();
    Serial.println("Guild available: " + guildName + " (" + guildId + ")");
  }
}

void DiscordClient::processMessage(JsonDocument& messageData) {
  String channelId = messageData["channel_id"].as<String>();
  String messageId = messageData["id"].as<String>();
  String content = messageData["content"].as<String>();
  String authorId = messageData["author"]["id"].as<String>();
  bool isBot = messageData["author"]["bot"].as<bool>();
  String username = messageData["author"]["username"].as<String>();
  
  Serial.println("Message from " + username + " in channel " + channelId + ": " + content);
  
  // Only process messages from our target channel, from non-bots, that are new
  if (channelId == String(DISCORD_CHANNEL_ID) && 
      !isBot && 
      messageId != lastMessageId && 
      content.length() > 0) {
    
    Serial.println("Processing new message: " + content);
    processNewMessage(content);
    lastMessageId = messageId;
  }
}

void DiscordClient::processNewMessage(const String& message) {
  Serial.println("Processing message for commands: '" + message + "'");
  commandSystem.executeCommand(message);
}

bool DiscordClient::sendMessage(const String& message) {
  String url = String(DISCORD_API_URL) + String(DISCORD_CHANNEL_ID) + "/messages";
  
  Serial.println("Sending message to: " + url);
  Serial.println("Message content: " + message);
  
  HTTPClient http;
  http.begin(httpClient, url);
  http.addHeader("Authorization", "Bot " + String(DISCORD_BOT_TOKEN));
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(10000);

  // Create proper JSON using ArduinoJson library
  JsonDocument doc;
  doc["content"] = message;
  String payload;
  serializeJson(doc, payload);
  
  Serial.println("JSON Payload: " + payload);

  int httpCode = http.POST(payload);
  bool success = false;
  
  Serial.println("HTTP Response Code: " + String(httpCode));
  
  if (httpCode > 0) {
    String response = http.getString();
    
    if (httpCode == 200 || httpCode == 201) {
      Serial.println("Message sent successfully");
      success = true;
    } else {
      Serial.println("Discord API returned error code: " + String(httpCode));
      Serial.println("Error response: " + response);
    }
  } else {
    Serial.println("HTTP request failed with code: " + String(httpCode));
  }
  
  http.end();
  return success;
}
