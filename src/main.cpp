#include "SystemManager.h"

void setup() {
  systemManager.begin();
}

void loop() {
  systemManager.update();
  delay(50); // Small delay for smooth operation
}