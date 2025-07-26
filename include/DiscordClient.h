#ifndef DISCORD_CLIENT_H
#define DISCORD_CLIENT_H

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>

class DiscordClient {
private:
  WiFiClientSecure httpClient;
  WebSocketsClient webSocket;
  String lastMessageId;
  String sessionId;
  String gatewayUrl;
  int sequenceNumber;
  unsigned long lastHeartbeat;
  unsigned long heartbeatInterval;
  unsigned long lastReconnectAttempt;
  int connectionAttempts;
  unsigned long lastConnectionTime;
  unsigned long lastReadyTime;
  bool isConnected;
  bool isAuthenticated;
  
  // Helper methods
  void getGatewayUrl();
  void connectWebSocket();
  void sendHeartbeat();
  void sendIdentify();
  void sendResume();
  void handleWebSocketEvent(WStype_t type, uint8_t * payload, size_t length);
  void handleDiscordMessage(const String& eventType, JsonDocument& data);
  void processMessage(JsonDocument& messageData);
  
  // Static callback for WebSocket events
  static void webSocketEvent(WStype_t type, uint8_t * payload, size_t length);
  static DiscordClient* instance; // For static callback
  
public:
  DiscordClient();
  
  // Core functions
  void begin();
  void update();
  bool sendMessage(const String& message);
  
  // Connection management
  bool isWebSocketConnected() const { return isConnected && isAuthenticated; }
  
private:
  void processNewMessage(const String& message);
};

// Global instance
extern DiscordClient discordClient;

#endif
