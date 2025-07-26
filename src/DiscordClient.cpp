#include "DiscordClient.h"
#include "CommandSystem.h"
#include "config.h"

// Global instance
DiscordClient discordClient;

DiscordClient::DiscordClient() : lastPollTime(0) {}

void DiscordClient::begin() {
  client.setInsecure(); // Skip SSL certificate verification for testing
  Serial.println("Discord client initialized");
}

void DiscordClient::update() {
  if (millis() - lastPollTime >= POLL_INTERVAL) {
    pollMessages();
    lastPollTime = millis();
  }
}

bool DiscordClient::sendMessage(const String& message) {
  String url = String(DISCORD_API_URL) + String(DISCORD_CHANNEL_ID) + "/messages";
  
  Serial.println("Sending message to: " + url);
  Serial.println("Message content: " + message);
  
  HTTPClient http;
  http.begin(client, url);
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
    Serial.println("Full Discord API Response: " + response);
    
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

void DiscordClient::pollMessages() {
  String url = String(DISCORD_API_URL) + String(DISCORD_CHANNEL_ID) + "/messages?limit=1";
  
  Serial.println("Polling Discord messages...");

  HTTPClient http;
  http.begin(client, url);
  http.addHeader("Authorization", "Bot " + String(DISCORD_BOT_TOKEN));
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(10000);

  int httpCode = http.GET();
  Serial.println("Poll HTTP Response Code: " + String(httpCode));
  
  if (httpCode == 200) {
    String response = http.getString();
    Serial.println("Poll Response Length: " + String(response.length()));
    
    String messageId, content, authorId;
    bool isBot;
    
    if (parseMessage(response, messageId, content, authorId, isBot)) {
      Serial.println("Parsed Message ID: " + messageId);
      Serial.println("Parsed Content: '" + content + "'");
      Serial.println("Author ID: " + authorId);
      Serial.println("Is Bot: " + String(isBot ? "true" : "false"));
      Serial.println("Last Message ID: " + lastMessageId);
      
      if (messageId != lastMessageId && messageId.length() > 0 && !isBot) {
        Serial.println("Processing new message from user: " + content);
        processNewMessage(content);
        lastMessageId = messageId;
      } else {
        if (messageId == lastMessageId) {
          Serial.println("Message already processed");
        } else if (isBot) {
          Serial.println("Ignoring bot message");
        } else if (messageId.length() == 0) {
          Serial.println("Empty message ID");
        }
      }
    } else {
      Serial.println("Failed to parse message");
    }
  } else {
    Serial.println("Error polling messages: " + String(httpCode));
  }
  
  http.end();
}

bool DiscordClient::parseMessage(const String& response, String& messageId, String& content, String& authorId, bool& isBot) {
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, response);
  
  if (error) {
    Serial.println("JSON parsing failed: " + String(error.c_str()));
    return false;
  }
  
  if (doc.size() == 0) {
    Serial.println("No messages found");
    return false;
  }
  
  messageId = doc[0]["id"].as<String>();
  content = doc[0]["content"].as<String>();
  authorId = doc[0]["author"]["id"].as<String>();
  isBot = doc[0]["author"]["bot"].as<bool>();
  
  return true;
}

void DiscordClient::processNewMessage(const String& message) {
  Serial.println("Processing message for commands: '" + message + "'");
  commandSystem.executeCommand(message);
}
