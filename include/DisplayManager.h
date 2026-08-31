#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>

enum class Gesture {
  none,
  tap,
  hold,
  swipe_up,
  swipe_down,
  swipe_left,
  swipe_right
};

class DisplayManager {
private:
  TFT_eSPI tft;
  // array to store calibration data for the touch screen
  uint16_t calData[5];

  // variables for gesture detection
  bool wasTouched = false;
  uint16_t startX = 0, startY = 0;
  uint16_t lastX = 0, lastY = 0;
  uint32_t startTime = 0;

  const uint16_t SWIPE_THRESHOLD =
      30;                           // minimum pixels to be considered a swipe
  const uint32_t TAP_TIMEOUT = 300; // max miliseconds for a quick tap

public:
  DisplayManager();

  void begin();
  void drawBootScreen();

  // returns true if screen is touched, passing x and y by reference
  bool getTouch(uint16_t &x, uint16_t &y);

  // draws a tiny indicator where the finger touched
  void drawTouchPoint(uint16_t x, uint16_t y);

  Gesture getGesture();
};

#endif // DISPLAY_MANAGER_H
