#include "NotificationCard.h"

void NotificationCard::onShow(TFT_eSPI *tft) {
  inDetailView = false;
  selectedIndex = 0;
  needsRedraw = true;
}

void NotificationCard::onHide() {}

void NotificationCard::onUpdate(TFT_eSPI *tft) {
  if (needsRedraw) {
    tft->fillScreen(TFT_BLACK);

    if (inDetailView) {
      drawDetailView(tft);
    } else {
      drawListView(tft);
    }
    needsRedraw = false;
  }
}

void NotificationCard::handleTap() {
  // Přepínání mezi seznamem a detailem při kliknutí
  if (NotificationManager::getHistoryCount() > 0) {
    inDetailView = !inDetailView;
    needsRedraw = true;
  }
}

void NotificationCard::drawListView(TFT_eSPI *tft) {
  tft->setTextSize(2);
  tft->setTextColor(TFT_GREEN, TFT_BLACK);
  tft->setCursor(10, 10);
  tft->print("Notifications");

  tft->drawLine(10, 35, 230, 35, TFT_DARKGREY);

  int count = NotificationManager::getHistoryCount();
  if (count == 0) {
    tft->setTextColor(TFT_LIGHTGREY);
    tft->setCursor(10, 60);
    tft->print("No new messages.");
    return;
  }

  // Vykreslí max 3 nejnovější upozornění pod sebe jako seznam
  for (int i = 0; i < count && i < 3; i++) {
    NotificationData notif = NotificationManager::getHistoryItem(i);

    int yPos = 45 + (i * 60); // Odsazení každé karty

    // Zvýrazníme první položku jako vybranou
    uint16_t boxColor = (i == selectedIndex) ? TFT_DARKGREY : TFT_BLACK;
    tft->fillRoundRect(5, yPos, 230, 50, 8, boxColor);
    tft->drawRoundRect(5, yPos, 230, 50, 8, TFT_WHITE);

    tft->setTextColor(TFT_WHITE);
    tft->setCursor(15, yPos + 8);
    tft->setTextSize(2);

    // Název zkrátíme, aby se vešel
    String title = notif.title;
    if (title.length() > 14)
      title = title.substring(0, 12) + "..";
    tft->print(title);

    // Podtitulek (kousek zprávy)
    tft->setTextColor(TFT_LIGHTGREY);
    tft->setCursor(15, yPos + 30);
    tft->setTextSize(1);
    String msg = notif.message;
    if (msg.length() > 30)
      msg = msg.substring(0, 27) + "...";
    tft->print(msg);
  }

  tft->setTextColor(TFT_YELLOW);
  tft->setCursor(60, 220);
  tft->print("TAP TO OPEN");
}

void NotificationCard::drawDetailView(TFT_eSPI *tft) {
  NotificationData notif = NotificationManager::getHistoryItem(selectedIndex);

  // Nadpis
  tft->fillRoundRect(5, 5, 230, 40, 8, TFT_BLUE);
  tft->setTextColor(TFT_WHITE);
  tft->setTextSize(2);
  tft->setCursor(15, 15);
  tft->print(notif.title);

  // Zpráva
  tft->setTextColor(TFT_WHITE, TFT_BLACK);
  tft->setTextSize(1);
  tft->setCursor(10, 60);

  // Velmi jednoduché zalamování textu (TFT_eSPI ho nativně moc neumí)
  int cursorY = 60;
  String word = "";
  for (int i = 0; i < notif.message.length(); i++) {
    tft->print(notif.message[i]);
    if (tft->getCursorX() > 210) {
      cursorY += 20;
      tft->setCursor(10, cursorY);
    }
  }

  tft->setTextColor(TFT_YELLOW);
  tft->setTextSize(1);
  tft->setCursor(60, 220);
  tft->print("TAP TO GO BACK");
}
