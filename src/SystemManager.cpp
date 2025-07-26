#include "SystemManager.h"
#include "DiscordClient.h"
#include "NeoPixelManager.h"
#include "CommandSystem.h"
#include "config.h"
#include <WiFi.h>

// Global instance
SystemManager systemManager;

SystemManager::SystemManager() : initialized(false) {}

void SystemManager::begin() {
  Serial.begin(115200);
  Serial.println("Starting Discord Bot ESP32...");
  
  initializeComponents();
  initializeWiFi();
  registerCommands();
  
  initialized = true;
  Serial.println("System initialization complete!");
  
  // WebSocket connection is initiated in DiscordClient::begin()
  Serial.println("Discord WebSocket connection initiated");
}

void SystemManager::update() {
  if (!initialized) return;
  
  neoPixelManager.update();
  discordClient.update();
}

String SystemManager::getStatusString() const {
  return initialized ? "Online" : "Offline";
}

void SystemManager::initializeWiFi() {
  Serial.println("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println();
  Serial.println("WiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void SystemManager::initializeComponents() {
  // Initialize all components
  neoPixelManager.begin();
  discordClient.begin();
  
  Serial.println("All components initialized");
}

void SystemManager::registerCommands() {
  // Register all available commands
  commandSystem.addCommand("status", "Check system status", CommandSystem::statusCommand);
  commandSystem.addCommand("turn_on", "Turn on the PC", CommandSystem::turnOnCommand);
  commandSystem.addCommand("turn_off", "Turn off the PC", CommandSystem::turnOffCommand);
  commandSystem.addCommand("rainbow", "Enable rainbow LED mode", CommandSystem::rainbowCommand);
  commandSystem.addCommand("red", "Set LED to red", CommandSystem::redCommand);
  commandSystem.addCommand("green", "Set LED to green", CommandSystem::greenCommand);
  commandSystem.addCommand("blue", "Set LED to blue", CommandSystem::blueCommand);
  commandSystem.addCommand("white", "Set LED to white", CommandSystem::whiteCommand);
  commandSystem.addCommand("off", "Turn off LED", CommandSystem::offCommand);
  commandSystem.addCommand("help", "Show available commands", CommandSystem::helpCommand);
  
  Serial.println("Commands registered successfully");
}
