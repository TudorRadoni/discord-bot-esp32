#ifndef NEOPIXEL_MANAGER_H
#define NEOPIXEL_MANAGER_H

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

class NeoPixelManager {
private:
  static const int LED_PIN = 2;
  static const int LED_ENABLE_PIN = 4;
  static const int LED_COUNT = 1;
  static const int DEFAULT_BRIGHTNESS = 50;
  
  Adafruit_NeoPixel strip;
  unsigned long lastRainbowUpdate;
  unsigned long rainbowStep;
  bool rainbowMode;
  bool enabled;
  
public:
  NeoPixelManager();
  
  // Core functions
  void begin();
  void update();
  
  // Color control
  void setColor(uint8_t red, uint8_t green, uint8_t blue);
  void setRainbowMode(bool enable);
  void setEnabled(bool enable);
  void flashColor(uint8_t red, uint8_t green, uint8_t blue, int duration = 500);
  
  // Getters
  bool isEnabled() const { return enabled; }
  bool isRainbowMode() const { return rainbowMode; }
  
  // Predefined colors
  void setRed() { setColor(255, 0, 0); }
  void setGreen() { setColor(0, 255, 0); }
  void setBlue() { setColor(0, 0, 255); }
  void setWhite() { setColor(255, 255, 255); }
  void turnOff() { setEnabled(false); }
};

// Global instance
extern NeoPixelManager neoPixelManager;

#endif
