#ifndef COMMAND_SYSTEM_H
#define COMMAND_SYSTEM_H

#include <Arduino.h>

// Command callback function type
typedef void (*CommandCallback)(void);

// Command structure
struct Command {
  String name;
  String description;
  CommandCallback callback;
};

class CommandSystem {
private:
  static const int MAX_COMMANDS = 20;
  Command commands[MAX_COMMANDS];
  int commandCount;
  
public:
  CommandSystem();
  
  // Command management
  bool addCommand(const String& name, const String& description, CommandCallback callback);
  void executeCommand(const String& command);
  String getHelpText() const;
  
  // Command implementations
  static void statusCommand();
  static void turnOnCommand();
  static void turnOffCommand();
  static void rainbowCommand();
  static void redCommand();
  static void greenCommand();
  static void blueCommand();
  static void whiteCommand();
  static void offCommand();
  static void helpCommand();
};

// Global instance
extern CommandSystem commandSystem;

#endif
