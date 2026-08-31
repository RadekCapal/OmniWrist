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
  uint16_t x = 0;
  uint16_t y = 0;

  // check if the screen is currently being pressed
  if (display.getTouch(x, y)) {
    Serial.printf("Touch detected - X: %d, Y: %d\n", x, y);
    display.drawTouchPoint(x, y);
  }

  // small delay to let the cpu breathe
  delay(10);
}
