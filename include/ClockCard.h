#ifndef CLOCK_CARD_H
#define CLOCK_CARD_H

#include "UI_Card.h"
#include <time.h>

class ClockCard : public UI_Card {
private:
  uint32_t lastUpdate = 0;

public:
  void onShow(TFT_eSPI *tft) override;
  void onUpdate(TFT_eSPI *tft) override;
  void onHide() override;
};

#endif // CLOCK_CARD_H
