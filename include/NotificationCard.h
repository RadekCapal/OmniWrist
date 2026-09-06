#ifndef NOTIFICATION_CARD_H
#define NOTIFICATION_CARD_H

#include "NotificationManager.h"
#include "UI_Card.h"
#include <TFT_eSPI.h>

class NotificationCard : public UI_Card {
public:
  void onShow(TFT_eSPI *tft) override;
  void onHide() override;
  void onUpdate(TFT_eSPI *tft) override;

  // Custom function to pass TAP events from main.cpp
  void handleTap();

private:
  bool inDetailView = false;
  int selectedIndex = 0; // Která zpráva je aktuálně vybraná (pro scrollování)
  bool needsRedraw = false;

  void drawListView(TFT_eSPI *tft);
  void drawDetailView(TFT_eSPI *tft);
};

#endif
