#include "BME280Card.h"
#include "ClockCard.h"
#include "DisplayManager.h"
#include "TimeManager.h"
#include "UI_Card.h"
#include "esp32-hal-gpio.h"
#include "secrets.h"
#include <Arduino.h>
#include <Wire.h>

DisplayManager display;

ClockCard clockCard;
BME280Card bmeCard;

UI_Card *cards[] = {&clockCard, &bmeCard};
const int NUM_CARDS = 2;
int currentCardIndex = 0;

// variables for power management
volatile bool touchInterruptTriggered =
    false; // volatile tells compiler this changes outside main thread
uint32_t lastActivityTime = 0;
const uint32_t SLEEP_TIMEOUT =
    10000; // time in ms before screen goes to sleep (10 seconds)

// hardware interrupt routine (must be in ram for speed)
void IRAM_ATTR touchWakeISR() { touchInterruptTriggered = true; }

void switchCard(int newIndex) {
  cards[currentCardIndex]->onHide();
  currentCardIndex = newIndex;
  cards[currentCardIndex]->onShow(display.getTFT());
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  // initialize i2c bus
  Wire.begin(I2C_SDA, I2C_SCL);
  bmeCard.begin();

  Serial.println("==================================");
  Serial.println("Starting OmniWrist OS...");
  Serial.println("==================================");

  display.begin();
  display.drawBootScreen();

  // sync time
  TimeManager::syncTime(WIFI_SSID, WIFI_PASS);

  // show the first card
  cards[currentCardIndex]->onShow(display.getTFT());

  // setup the interrupt  pin
  pinMode(TOUCH_IRQ, INPUT_PULLUP);

  lastActivityTime = millis();
}

void loop() {
  // 1. handle hardware wake up signal
  if (touchInterruptTriggered) {
    touchInterruptTriggered = false; // lower the flag

    if (!display.isScreenAwake()) {
      display.wakeUp();
      detachInterrupt(TOUCH_IRQ);

      lastActivityTime = millis();
      Serial.println("System Woken Up by Touch Interrupt!");

      // force full redraw of the card when waking up
      cards[currentCardIndex]->onShow(display.getTFT());
      delay(50);
      return;
    }
  }

  // 2. process ui only if screen is active
  if (display.isScreenAwake()) {
    cards[currentCardIndex]->onUpdate(display.getTFT());

    Gesture userAction = display.getGesture();

    // if user did something, reset the sleep timer
    if (userAction != Gesture::none || display.isCurrentlyTouched()) {
      lastActivityTime = millis();

      switch (userAction) {
      case Gesture::tap:
        Serial.println("Action: TAP");
        break;
      case Gesture::hold:
        Serial.println("Action: HOLD");
        break;
      case Gesture::swipe_left:
        Serial.println("Action: SWIPE LEFT");
        switchCard((currentCardIndex + 1) % NUM_CARDS);
        break;
      case Gesture::swipe_right:
        Serial.println("Action: SWIPE RIGHT");
        switchCard((currentCardIndex - 1 + NUM_CARDS) % NUM_CARDS);
        break;
      case Gesture::swipe_up:
        Serial.println("Action: SWIPE UP");
        break;
      case Gesture::swipe_down:
        Serial.println("Action: SWIPE DOWN");
        break;
      case Gesture::none:
        break;
      }
    }

    // 3. check if it is time to sleep
    if (millis() - lastActivityTime > SLEEP_TIMEOUT) {
      Serial.println("Going to sleep to save power...");
      display.sleep();

      touchInterruptTriggered = false;
      attachInterrupt(TOUCH_IRQ, touchWakeISR, FALLING);
    }
  }

  // small delay to let the cpu breathe
  delay(10);
}
