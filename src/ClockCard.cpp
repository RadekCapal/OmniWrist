#include "ClockCard.h"

void ClockCard::onShow(TFT_eSPI *tft) {
  // draw static background once when card appears
  tft->fillScreen(TFT_BLACK);

  // top status bar line
  tft->drawFastHLine(0, 20, 320, TFT_DARKGREY);

  tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft->setTextDatum(TC_DATUM); // top center
  tft->setTextSize(1);
  tft->drawString("LOCAL TIME", 160, 5);

  // force immediate update of the dynamic time
  lastUpdate = 0;
}

void ClockCard::onUpdate(TFT_eSPI *tft) {
  // only redraw the clock once per second to prevent flickering and save cpu
  if (millis() - lastUpdate >= 1000) {
    lastUpdate = millis();

    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      // format time string as HH:MM:SS
      char timeString[10];
      strftime(timeString, sizeof(timeString), "%H:%M:%S", &timeinfo);

      // date string as DD.MM.YYYY
      char dateString[15];
      strftime(dateString, sizeof(dateString), "%d.%m.%Y", &timeinfo);

      // draw large time
      tft->setTextColor(TFT_WHITE, TFT_BLACK);
      tft->setTextDatum(MC_DATUM); // middle center
      tft->setTextSize(4);         // large font
      tft->drawString(timeString, 160, 110);

      // draw smaller date below
      tft->setTextColor(TFT_GREEN, TFT_BLACK);
      tft->setTextSize(2);
      tft->drawString(dateString, 160, 160);
    }
  }
}

void ClockCard::onHide() {
  // nothing to shut down for a simple clock
  // later, sensors will put their hardware to sleep here
}
