#ifndef BACKLIGHT_MANAGER_H
#define BACKLIGHT_MANAGER_H

#include <Arduino.h>
#include <BH1750.h>
#include <Wire.h>

class BacklightManager {
private:
  BH1750 lightMeter;
  bool isSensorFound = false;

  // current smoothed pwm value
  float currentPWM = 150.0;
  float savedPWM = 150.0; // variable to remember brightness before sleep

  // limits for display brightness (0-255)
  // never use 0 for min, otherwise the screen goes completely black
  const int MIN_BRIGHTNESS = 20;
  const int MAX_BRIGHTNESS = 255;

  uint32_t lastUpdate = 0;

public:
  void begin();
  void update();
  void setBrightness(int targetPWM);
  void sleep();
  void wakeUp();
};

#endif // BACKLIGHT_MANAGER_H
