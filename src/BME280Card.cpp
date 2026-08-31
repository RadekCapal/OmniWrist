#include "BME280Card.h"
#include "TFT_eSPI.h"

void BME280Card::begin() {
  // initialize sensor on i2c address 0x76 (most common for chinese modules)
  // if it fails, try 0x77
  isSensorFound = bme.begin(0x76, &Wire);
}

void BME280Card::onShow(TFT_eSPI *tft) {
  tft->fillScreen(TFT_BLACK);
  tft->drawFastHLine(0, 20, 320, TFT_DARKGREY);

  tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft->setTextDatum(TC_DATUM);
  tft->setTextSize(1);
  tft->drawString("ENVIRONMENT", 160, 5);

  if (!isSensorFound) {
    tft->setTextColor(TFT_RED, TFT_BLACK);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("BME280 NOT FOUND!", 160, 120);
  }

  lastUpdate = 0; // force immediate draw
}

void BME280Card::onUpdate(TFT_eSPI *tft) {
  if (!isSensorFound)
    return;

  // update screen every 2 seconds
  if (millis() - lastUpdate >= 2000) {
    lastUpdate = millis();

    float temp = bme.readTemperature();
    float hum = bme.readHumidity();
    float bar = bme.readPressure() / 100.0F; // convert to hPa
    float alt = bme.readAltitude(1013.25);   // (TODO: update with GPS)

    char tempStr[10];
    char humStr[10];
    char barStr[15];
    char altStr[15];
    // format to 1 decimal place
    snprintf(tempStr, sizeof(tempStr), "%.1f C", temp);
    snprintf(humStr, sizeof(humStr), "%.1f %%", hum);
    snprintf(barStr, sizeof(barStr), "%.1f hPa", bar);
    snprintf(altStr, sizeof(altStr), "%.1f m", alt);

    tft->setTextColor(TFT_ORANGE, TFT_BLACK);
    tft->setTextDatum(MC_DATUM);
    tft->setTextSize(4);
    tft->drawString(tempStr, 160, 80);

    tft->setTextColor(TFT_CYAN, TFT_BLACK);
    tft->setTextSize(3);
    tft->drawString(humStr, 160, 130);

    tft->setTextColor(TFT_LIGHTGREY, TFT_BLACK);
    tft->setTextSize(2);
    tft->drawString(barStr, 160, 180);

    tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
    tft->setTextSize(2);
    tft->drawString(altStr, 160, 210);
  }
}

void BME280Card::onHide() {
  // bme280 puts itself to sleep automatically in normal mode,
  // so we don't need manual shutdown here for now.
}
