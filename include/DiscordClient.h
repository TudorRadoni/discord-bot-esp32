#ifndef DISCORD_CLIENT_H
#define DISCORD_CLIENT_H

#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>

class DiscordClient {
private:
  WiFiClientSecure client;
  String lastMessageId;
  unsigned long lastPollTime;
  static const unsigned long POLL_INTERVAL = 10000; // 10 seconds
  
  // Helper methods
  bool parseMessage(const String& response, String& messageId, String& content, String& authorId, bool& isBot);
  
public:
  DiscordClient();
  
  // Core functions
  void begin();
  void update();
  bool sendMessage(const String& message);
  
  // Message handling
  void pollMessages();
  
private:
  void processNewMessage(const String& message);
};

// Global instance
extern DiscordClient discordClient;

#endif
