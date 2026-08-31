#include "DisplayManager.h"
#include <Arduino.h>

DisplayManager display;

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("==================================");
  Serial.println("Starting OmniWrist OS...");
  Serial.println("==================================");

  display.begin();
  display.drawBootScreen();
}

void loop() {
  // get user input
  Gesture userAction = display.getGesture();
  uint16_t x = 0;
  uint16_t y = 0;

  // handle the action
  switch (userAction) {
  case Gesture::tap:
    Serial.println("Action : TAP - opening card details");
    break;
  case Gesture::hold:
    Serial.println("Action : HOLD - something");
    break;
  case Gesture::swipe_left:
    Serial.println("Action : SWIPE LEFT - moving to next sensor card");
    break;
  case Gesture::swipe_right:
    Serial.println("Action : SWIPE RIGHT - moving to previous sensor card");
    break;
  case Gesture::swipe_up:
    Serial.println("Action : SWIPE UP - scrolling content up");
    break;
  case Gesture::swipe_down:
    Serial.println("Action : SWIPE DOWN - scrolling content down");
    break;
  case Gesture::none:
    // do nothing
    break;
  }

  // check if the screen is currently being pressed
  // if (display.getTouch(x, y)) {
  // Serial.printf("Touch detected - X: %d, Y: %d\n", x, y);
  // display.drawTouchPoint(x, y);
  //}

  // small delay to let the cpu breathe
  delay(10);
}
