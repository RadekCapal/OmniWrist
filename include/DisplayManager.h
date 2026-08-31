#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>

class DisplayManager {
private:
  TFT_eSPI tft;
  // array to store calibration data for the touch screen
  uint16_t calData[5];

public:
  DisplayManager();

  void begin();
  void drawBootScreen();

  // returns true if screen is touched, passing x and y by reference
  bool getTouch(uint16_t &x, uint16_t &y);

  // draws a tiny indicator where the finger touched
  void drawTouchPoint(uint16_t x, uint16_t y);
};

#endif // DISPLAY_MANAGER_H
