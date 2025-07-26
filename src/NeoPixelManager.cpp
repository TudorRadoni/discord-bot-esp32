#include "NeoPixelManager.h"

// Global instance
NeoPixelManager neoPixelManager;

NeoPixelManager::NeoPixelManager() 
  : strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800),
    lastRainbowUpdate(0),
    rainbowStep(0),
    rainbowMode(true),
    enabled(true) {
}

void NeoPixelManager::begin() {
  Serial.println("Initializing NeoPixel...");
  pinMode(LED_ENABLE_PIN, OUTPUT);
  digitalWrite(LED_ENABLE_PIN, LOW); // Enable NeoPixel power (LOW = enable)
  Serial.println("LED_ENABLE_PIN set to LOW");
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(DEFAULT_BRIGHTNESS);
  
  Serial.println("NeoPixel initialized with brightness: " + String(DEFAULT_BRIGHTNESS));
  
  // Test the LED with a quick flash
  Serial.println("Testing LED with red flash...");
  strip.setPixelColor(0, strip.Color(255, 0, 0));
  strip.show();
  delay(500);
  strip.setPixelColor(0, strip.Color(0, 0, 0));
  strip.show();
  Serial.println("LED test complete");
}

void NeoPixelManager::update() {
  if (!enabled) {
    return;
  }
  
  if (rainbowMode) {
    // Rainbow animation
    if (millis() - lastRainbowUpdate >= 50) { // Update every 50ms
      uint32_t color = strip.ColorHSV(rainbowStep * 256);
      strip.setPixelColor(0, color);
      strip.show();
      
      rainbowStep = (rainbowStep + 1) % 256;
      lastRainbowUpdate = millis();
      
      // Debug output every 256 steps (one full cycle)
      if (rainbowStep == 0) {
        Serial.println("Rainbow cycle complete, step: " + String(rainbowStep));
      }
    }
  }
}

void NeoPixelManager::setColor(uint8_t red, uint8_t green, uint8_t blue) {
  rainbowMode = false;
  enabled = true;
  strip.setPixelColor(0, strip.Color(red, green, blue));
  strip.show();
}

void NeoPixelManager::setRainbowMode(bool enable) {
  rainbowMode = enable;
  if (enable) {
    enabled = true;
  }
}

void NeoPixelManager::setEnabled(bool enable) {
  enabled = enable;
  if (!enable) {
    rainbowMode = false;
    strip.setPixelColor(0, strip.Color(0, 0, 0));
    strip.show();
  }
}

void NeoPixelManager::flashColor(uint8_t red, uint8_t green, uint8_t blue, int duration) {
  bool wasRainbow = rainbowMode;
  rainbowMode = false;
  
  strip.setPixelColor(0, strip.Color(red, green, blue));
  strip.show();
  delay(duration);
  
  if (wasRainbow) {
    rainbowMode = true;
  } else {
    // Return to previous color (you might want to store previous state)
    strip.show();
  }
}
