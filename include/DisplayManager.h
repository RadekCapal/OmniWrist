#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <cstdint>

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
  bool _isAwake = true;

  // variables for gesture detection
  bool wasTouched = false;
  uint16_t startX = 0, startY = 0;
  uint16_t lastX = 0, lastY = 0;
  uint32_t startTime = 0;

  // debounce and wake up
  uint32_t lastTouchTime = 0;
  bool _ignoreNextGesture = false;
  const uint16_t TOUCH_DEBOUNCE_MS = 50;

  const uint16_t SWIPE_THRESHOLD =
      30;                           // minimum pixels to be considered a swipe
  const uint32_t TAP_TIMEOUT = 300; // max miliseconds for a quick tap

public:
  DisplayManager();

  // returns pointer to the tft object so UI cards can draw on it
  TFT_eSPI *getTFT() { return &tft; }

  void begin();
  void drawBootScreen();

  // returns true if screen is touched, passing x and y by reference
  bool getTouch(uint16_t &x, uint16_t &y);

  // draws a tiny indicator where the finger touched
  void drawTouchPoint(uint16_t x, uint16_t y);

  Gesture getGesture();

  void sleep();
  void wakeUp();
  bool isScreenAwake();
  bool isCurrentlyTouched();
};

#endif // DISPLAY_MANAGER_H
