#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include <Arduino.h>

class SystemManager {
private:
  bool initialized;
  
public:
  SystemManager();
  
  // Core system functions
  void begin();
  void update();
  
  // System status
  bool isOnline() const { return initialized; }
  String getStatusString() const;
  
private:
  void initializeWiFi();
  void initializeComponents();
  void registerCommands();
};

// Global instance
extern SystemManager systemManager;

#endif
