#include "CompassCard.h"
#include "CompassManager.h"
#include <math.h>

void CompassCard::onShow(TFT_eSPI *tft) {
  tft->fillScreen(TFT_BLACK);
  drawCompassRose(tft);
  lastAzimuth = -1; // Vynutí první vykreslení střelky
}

void CompassCard::onHide() {
  // Není potřeba nic speciálního při opuštění karty
}

void CompassCard::onUpdate(TFT_eSPI *tft) {
  // Omezíme překreslování displeje na max 10x za vteřinu, aby se procesor
  // nezpotil
  if (millis() - lastUpdate > 100) {
    lastUpdate = millis();

    int currentAzimuth = compassManager.getAzimuth();

    // Překreslíme střelku, jen pokud se azimut změnil o více než 1 stupeň
    // (zabrání "třepání")
    if (abs(currentAzimuth - lastAzimuth) > 1 || lastAzimuth == -1) {

      // 1. Smazat starou střelku (černou barvou)
      if (lastAzimuth != -1) {
        drawNeedle(tft, lastAzimuth, TFT_BLACK);
      }

      // 2. Nakreslit novou střelku (červeně)
      drawNeedle(tft, currentAzimuth, TFT_RED);

      // 3. Obnovit středový bod (aby ho střelka nesmazala)
      tft->fillCircle(tft->width() / 2, tft->height() / 2 - 15, 5, TFT_WHITE);

      // 4. Vypsat text dole na obrazovce
      tft->fillRect(0, tft->height() - 40, tft->width(), 40,
                    TFT_BLACK); // Vyčistit starý text
      tft->setTextColor(TFT_GREEN, TFT_BLACK);

      String text = compassManager.getDirectionString(currentAzimuth) + "  " +
                    String(currentAzimuth) + " deg";
      tft->drawCentreString(text, tft->width() / 2, tft->height() - 30,
                            4); // Font 4 je krásně čitelný

      lastAzimuth = currentAzimuth;
    }
  }
}

void CompassCard::drawCompassRose(TFT_eSPI *tft) {
  int cx = tft->width() / 2;
  int cy = tft->height() / 2 - 15;
  int r = 80; // Poloměr kompasu

  // Kruhy pro vzhled kompasu
  tft->drawCircle(cx, cy, r, TFT_DARKGREY);
  tft->drawCircle(cx, cy, r + 1, TFT_DARKGREY);

  // Světové strany
  tft->setTextColor(TFT_WHITE);
  tft->drawCentreString("N", cx, cy - r - 20, 2);
  tft->drawCentreString("S", cx, cy + r + 5, 2);
  tft->drawCentreString("E", cx + r + 10, cy - 8, 2);
  tft->drawCentreString("W", cx - r - 20, cy - 8, 2);
}

void CompassCard::drawNeedle(TFT_eSPI *tft, int azimuth, uint16_t color) {
  int cx = tft->width() / 2;
  int cy = tft->height() / 2 - 15;
  int r = 70; // Délka střelky

  // Výpočet X a Y na kružnici pomocí goniometrických funkcí.
  // Odebíráme -90 stupňů, protože nula (Sever) je na displeji "nahoře", zatímco
  // v matematice je nula "vpravo".
  float rad = (azimuth - 90) * PI / 180.0;

  int x = cx + r * cos(rad);
  int y = cy + r * sin(rad);

  // Vykreslení hlavní linky
  tft->drawLine(cx, cy, x, y, color);

  // Aby byla střelka lépe vidět, vykreslíme i druhou linku těsně vedle ní
  tft->drawLine(cx + 1, cy, x + 1, y, color);
  tft->drawLine(cx, cy + 1, x, y + 1, color);
}
