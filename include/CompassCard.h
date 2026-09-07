#ifndef COMPASS_CARD_H
#define COMPASS_CARD_H

#include "UI_Card.h"
#include <TFT_eSPI.h>

class CompassCard : public UI_Card {
public:
  void onShow(TFT_eSPI *tft) override;
  void onHide() override;
  void onUpdate(TFT_eSPI *tft) override;

private:
  int lastAzimuth = -1;
  uint32_t lastUpdate = 0;

  void drawCompassRose(TFT_eSPI *tft);
  void drawNeedle(TFT_eSPI *tft, int azimuth, uint16_t color);
};

#endif
