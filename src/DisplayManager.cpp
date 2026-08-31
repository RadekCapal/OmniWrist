#include "DisplayManager.h"
#include "esp32-hal-gpio.h"
#include "esp32-hal.h"

DisplayManager::DisplayManager() : tft(TFT_eSPI()) {}

void DisplayManager::begin() {
  Serial.println("[DISPLAY] initializing TFT...");

  tft.init();
  tft.setRotation(1);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  // interactive touch calibration routine
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 100);
  tft.setTextFont(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.println("Touch corners to calibrate");

  // built-in tft_espi function that handles calibration visually
  tft.calibrateTouch(calData, TFT_WHITE, TFT_BLACK, 15);

  Serial.println("[DISPLAY] TFT initialized and calibrated.");
}

void DisplayManager::drawBootScreen() {
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextDatum(MC_DATUM);

  tft.drawString("OmniWrist OS", 160, 100);

  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextSize(1);
  tft.drawString("Welcome to Matrix", 160, 130);
}

bool DisplayManager::getTouch(uint16_t &x, uint16_t &y) {
  // queries the touch controller for current coordinates
  return tft.getTouch(&x, &y);
}

void DisplayManager::drawTouchPoint(uint16_t x, uint16_t y) {
  // draw a tiny red circle at the touch location
  tft.fillCircle(x, y, 2, TFT_RED);
}

Gesture DisplayManager::getGesture() {
  uint16_t currentX = 0, currentY = 0;

  // read hardware state
  bool isPhysicallyTouched = tft.getTouch(&currentX, &currentY);

  if (isPhysicallyTouched) {
    // draw tracking dots only if we are not ignoring this touch
    if (!_ignoreNextGesture) {
      tft.fillCircle(currentX, currentY, 2, TFT_RED);
    }

    lastTouchTime = millis(); // keep pushing the debounce timer forward

    if (!wasTouched) {
      startX = currentX;
      startY = currentY;
      startTime = millis();
      wasTouched = true;
    }
    lastX = currentX;
    lastY = currentY;

    return Gesture::none;
  }

  // touch was lost, but we are in "wasTouched" state
  if (!isPhysicallyTouched && wasTouched) {

    // DEBOUNCE CHECK: Has the finger been off the screen long enough?
    if (millis() - lastTouchTime > TOUCH_DEBOUNCE_MS) {
      wasTouched = false; // definitively end the touch

      int16_t dx = lastX - startX;
      int16_t dy = lastY - startY;

      // use lastTouchTime instead of millis() for accurate duration without
      // debounce delay
      uint32_t duration = lastTouchTime - startTime;

      Gesture detectedGesture = Gesture::none;

      // 1. evaluate the gesture
      if (abs(dx) < SWIPE_THRESHOLD && abs(dy) < SWIPE_THRESHOLD) {
        if (duration < TAP_TIMEOUT) {
          detectedGesture = Gesture::tap;
        } else {
          detectedGesture = Gesture::hold;
        }
      } else if (abs(dx) > abs(dy)) {
        if (dx > 0)
          detectedGesture = Gesture::swipe_right;
        else
          detectedGesture = Gesture::swipe_left;
      } else {
        if (dy > 0)
          detectedGesture = Gesture::swipe_down;
        else
          detectedGesture = Gesture::swipe_up;
      }

      // 2. swallow the gesture if it was the wake-up touch
      if (_ignoreNextGesture) {
        _ignoreNextGesture = false; // reset the flag
        return Gesture::none;       // pretend nothing happened
      }

      return detectedGesture;
    }
  }

  return Gesture::none;
}

void DisplayManager::sleep() {
  // turn off backlight
  digitalWrite(TFT_BL, LOW);
  _isAwake = false;
}

void DisplayManager::wakeUp() {
  // turn on backlight
  digitalWrite(TFT_BL, HIGH);
  _isAwake = true;
  _ignoreNextGesture = true; // for touch that wake OS up
}

bool DisplayManager::isScreenAwake() { return _isAwake; }

bool DisplayManager::isCurrentlyTouched() { return wasTouched; }
