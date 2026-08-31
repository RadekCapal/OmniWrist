#include "DisplayManager.h"
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
  bool isTouched = tft.getTouch(&currentX, &currentY);

  // case 1: finger is currently on the screen
  if (isTouched) {
    if (!wasTouched) {
      startX = currentX;
      startY = currentY;
      startTime = millis();
      wasTouched = true;
    }
    // continuously update the last known position
    lastX = currentX;
    lastY = currentY;

    return Gesture::none; // gesture is not finished yet
  }

  // case 2: finger was just released from the screen
  if (!isTouched && wasTouched) {
    wasTouched = false; // reset state
    int16_t dx = lastX - startX;
    int16_t dy = lastY - startY;
    uint32_t duration = millis() - startTime;

    // check if movement was too small to be a swipe
    if (abs(dx) < SWIPE_THRESHOLD && abs(dy) < SWIPE_THRESHOLD) {
      if (duration < TAP_TIMEOUT) {
        return Gesture::tap;
      }
      return Gesture::hold; // touch was too long for a tap, but too short for a
                            // swipe => hold
    }
    // if movement was significat, determine the primary axis
    if (abs(dx) > abs(dy)) {
      // horizontal movement is dominant
      if (dx > 0)
        return Gesture::swipe_right;
      else
        return Gesture::swipe_left;
    } else {
      // vertical movement is dominant
      if (dy > 0)
        return Gesture::swipe_down;
      else
        return Gesture::swipe_up;
    }
  }
  // case 3: no touch detected, nothing happening
  return Gesture::none;
}
