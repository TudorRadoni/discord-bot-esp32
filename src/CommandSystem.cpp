#include "CommandSystem.h"
#include "DiscordClient.h"
#include "NeoPixelManager.h"
#include "SystemManager.h"

// Global instance
CommandSystem commandSystem;

CommandSystem::CommandSystem() : commandCount(0) {}

bool CommandSystem::addCommand(const String& name, const String& description, CommandCallback callback) {
  if (commandCount >= MAX_COMMANDS) {
    Serial.println("Error: Maximum number of commands reached");
    return false;
  }
  
  commands[commandCount] = {name, description, callback};
  commandCount++;
  Serial.println("Command registered: " + name);
  return true;
}

void CommandSystem::executeCommand(const String& command) {
  String cmd = command;
  cmd.toLowerCase();
  cmd.trim();
  
  // Remove leading slash if present
  if (cmd.startsWith("/")) {
    cmd = cmd.substring(1);
  }
  
  if (cmd.length() == 0) {
    return;
  }
  
  Serial.println("Executing command: " + cmd);
  
  // Search for command in command table
  bool commandFound = false;
  for (int i = 0; i < commandCount; i++) {
    if (commands[i].name.equals(cmd)) {
      Serial.println("Command found: " + commands[i].name);
      commands[i].callback();
      commandFound = true;
      break;
    }
  }
  
  if (!commandFound) {
    Serial.println("Unknown command: " + cmd);
    discordClient.sendMessage("❌ Unknown command: `" + cmd + "`. Type `help` to see available commands.");
  }
}

String CommandSystem::getHelpText() const {
  String helpText = "🤖 **Available Commands:**\n\n";
  
  for (int i = 0; i < commandCount; i++) {
    helpText += "`" + commands[i].name + "` - " + commands[i].description + "\n";
  }
  
  helpText += "\n💡 **Tips:**\n";
  helpText += "• Commands are case-insensitive\n";
  helpText += "• You can use commands with or without `/`\n";
  helpText += "• LED starts in rainbow mode by default";
  
  return helpText;
}

// Command implementations
void CommandSystem::statusCommand() {
  String status = "✅ **System Status:**\n";
  status += "🖥️ PC: " + String(systemManager.isOnline() ? "Online" : "Offline") + "\n";
  status += "💡 LED: " + String(neoPixelManager.isEnabled() ? "Enabled" : "Disabled") + "\n";
  status += "🌈 Mode: " + String(neoPixelManager.isRainbowMode() ? "Rainbow" : "Static");
  
  discordClient.sendMessage(status);
}

void CommandSystem::turnOnCommand() {
  discordClient.sendMessage("🔌 **PC Turn On Command Executed**\n*Note: This is a simulation. Connect actual hardware for real control.*");
  neoPixelManager.flashColor(0, 255, 0); // Flash green
}

void CommandSystem::turnOffCommand() {
  discordClient.sendMessage("🔴 **PC Turn Off Command Executed**\n*Note: This is a simulation. Connect actual hardware for real control.*");
  neoPixelManager.flashColor(255, 0, 0); // Flash red
}

void CommandSystem::rainbowCommand() {
  neoPixelManager.setRainbowMode(true);
  neoPixelManager.setEnabled(true);
  discordClient.sendMessage("🌈 **Rainbow mode enabled!**");
  Serial.println("Rainbow mode activated");
}

void CommandSystem::redCommand() {
  neoPixelManager.setRed();
  discordClient.sendMessage("🔴 **LED set to red**");
  Serial.println("LED set to red");
}

void CommandSystem::greenCommand() {
  neoPixelManager.setGreen();
  discordClient.sendMessage("🟢 **LED set to green**");
  Serial.println("LED set to green");
}

void CommandSystem::blueCommand() {
  neoPixelManager.setBlue();
  discordClient.sendMessage("🔵 **LED set to blue**");
  Serial.println("LED set to blue");
}

void CommandSystem::whiteCommand() {
  neoPixelManager.setWhite();
  discordClient.sendMessage("⚪ **LED set to white**");
  Serial.println("LED set to white");
}

void CommandSystem::offCommand() {
  neoPixelManager.turnOff();
  discordClient.sendMessage("⚫ **LED turned off**");
  Serial.println("LED turned off");
}

void CommandSystem::helpCommand() {
  String helpText = commandSystem.getHelpText();
  discordClient.sendMessage(helpText);
  Serial.println("Help command executed");
}
