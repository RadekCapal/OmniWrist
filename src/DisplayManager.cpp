#include "DisplayManager.h"

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
