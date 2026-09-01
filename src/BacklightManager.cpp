#include "BacklightManager.h"
#include "esp32-hal.h"

// we use the pin defined in your PINOUT.md
#ifndef TFT_BL
#define TFT_BL 21
#endif

void BacklightManager::begin() {
  // initialize pwm for the backlight pin
  // using standard arduino analogwrite configuration for esp32
  pinMode(TFT_BL, OUTPUT);
  analogWrite(TFT_BL, (int)currentPWM);

  // initialize the bh1750 sensor
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    isSensorFound = true;
    Serial.println("[LUX] BH1750 initialized successfully");
  } else {
    Serial.println("[LUX] BH1750 not found!");
  }
}

void BacklightManager::update() {
  if (!isSensorFound)
    return;

  uint32_t now = millis();

  // read sensor only every 100ms to save i2c bandwidth
  if (now - lastUpdate > 100) {
    lastUpdate = now;

    // read light level in lux
    float lux = lightMeter.readLightLevel();

    // map lux to target pwm
    // typical room is ~100 lux, direct sunlight > 1000 lux, dark room < 10 lux
    int targetPWM;
    if (lux > 800) {
      targetPWM = MAX_BRIGHTNESS;
    } else if (lux < 10) {
      targetPWM = MIN_BRIGHTNESS;
    } else {
      // linear map for values in between
      targetPWM = map((long)lux, 10, 800, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
    }

    // low-pass filter for smooth fading (90% old, 10% new)
    currentPWM = (currentPWM * 0.90) + ((float)targetPWM * 0.10);

    // apply the smoothed value to the display backlight
    analogWrite(TFT_BL, (int)currentPWM);
  }
}

void BacklightManager::setBrightness(int targetPWM) {
  // manual override if you need it (e.g., when screen wakes up)
  currentPWM = targetPWM;
  analogWrite(TFT_BL, (int)currentPWM);
}

void BacklightManager::sleep() {
  savedPWM = currentPWM;
  analogWrite(TFT_BL, 0);
}

void BacklightManager::wakeUp() {
  currentPWM = savedPWM;
  analogWrite(TFT_BL, (int)currentPWM);
}
