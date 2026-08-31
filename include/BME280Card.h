#ifndef BME280_CARD_H
#define BME280_CARD_H

#include "UI_Card.h"
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>

class BME280Card : public UI_Card {
private:
  Adafruit_BME280 bme;
  bool isSensorFound = false;
  uint32_t lastUpdate = 0;

public:
  void begin();
  void onShow(TFT_eSPI *tft) override;
  void onUpdate(TFT_eSPI *tft) override;
  void onHide() override;
};

#endif // BME280_CARD_H
