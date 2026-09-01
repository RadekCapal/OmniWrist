#include "PedometerCard.h"
#include "MotionManager.h"

// link to the global instance existing in main.cpp
extern MotionManager motion;

void PedometerCard::onShow(TFT_eSPI *tft) {
  tft->fillScreen(TFT_BLACK);
  tft->drawFastHLine(0, 20, 320, TFT_DARKGREY);

  tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft->setTextDatum(TC_DATUM);
  tft->setTextSize(1);
  tft->drawString("ACTIVITY", 160, 5);

  tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft->setTextDatum(MC_DATUM);
  tft->setTextSize(2);
  tft->drawString("Steps today:", 160, 70);

  // set to impossible value to force full redraw on first update
  lastStepCount = 0xFFFFFFFF;
  lastDisplayUpdate = 0;
}

void PedometerCard::onUpdate(TFT_eSPI *tft) {
  // update ui every 200ms to stay responsive but prevent flickering
  if (millis() - lastDisplayUpdate > 200) {
    lastDisplayUpdate = millis();

    uint32_t currentSteps = motion.getSteps();

    // only redraw if the user actually took a step
    if (currentSteps != lastStepCount) {
      lastStepCount = currentSteps;

      // clear just the area where the numbers are
      tft->fillRect(0, 100, 320, 90, TFT_BLACK);

      tft->setTextColor(TFT_GREEN, TFT_BLACK);
      tft->setTextDatum(MC_DATUM);
      tft->setTextSize(7);

      char stepStr[15];
      snprintf(stepStr, sizeof(stepStr), "%lu", currentSteps);
      tft->drawString(stepStr, 160, 140);

      // optional: add small motivational text if moving
      if (currentSteps > 0) {
        tft->setTextColor(TFT_ORANGE, TFT_BLACK);
        tft->setTextSize(2);
        tft->drawString("Keep going!", 160, 210);
      }
    }
  }
}

void PedometerCard::onHide() {
  // pedometer does not need to shut down hardware, motion manager handles it
}
