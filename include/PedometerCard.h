#ifndef PEDOMETER_CARD_H
#define PEDOMETER_CARD_H

#include "UI_Card.h"
#include <Arduino.h>

class PedometerCard : public UI_Card {
private:
  uint32_t lastStepCount = 0;
  uint32_t lastDisplayUpdate = 0;

public:
  void onShow(TFT_eSPI *tft) override;
  void onUpdate(TFT_eSPI *tft) override;
  void onHide() override;
};

#endif // PEDOMETER_CARD_H
