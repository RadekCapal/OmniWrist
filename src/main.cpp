#include "BME280Card.h"
#include "BacklightManager.h"
#include "ClockCard.h"
#include "DisplayManager.h"
#include "EncoderManager.h"
#include "HeartRateCard.h"
#include "MotionManager.h"
#include "PedometerCard.h"
#include "TimeManager.h"
#include "UI_Card.h"
#include "esp32-hal-gpio.h"
#include "esp32-hal.h"
#include "secrets.h"
#include <Arduino.h>
#include <Wire.h>

DisplayManager display;
MotionManager motion;
EncoderManager encoder;
BacklightManager backlight;

ClockCard clockCard;
BME280Card bmeCard;
HeartRateCard heartCard;
PedometerCard stepCard;

UI_Card *cards[] = {&clockCard, &bmeCard, &heartCard, &stepCard};
const int NUM_CARDS = 4;
int currentCardIndex = 0;

// variables for power management
volatile bool touchInterruptTriggered = false;
volatile bool imuInterruptTriggered = false;

uint32_t lastActivityTime = 0;
const uint32_t SLEEP_TIMEOUT =
    10000; // time in ms before screen goes to sleep (10 seconds)

// hardware interrupt routine (must be in ram for speed)
void IRAM_ATTR touchWakeISR() { touchInterruptTriggered = true; }
void IRAM_ATTR imuWakeISR() { imuInterruptTriggered = true; }

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
  delay(100);

  bmeCard.begin();
  motion.begin();
  encoder.begin();
  heartCard.begin();
  backlight.begin();

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
  pinMode(IMU_INT, INPUT_PULLUP);

  lastActivityTime = millis();
}

void loop() {
  // 1. handle hardware wake up signal
  if (touchInterruptTriggered || imuInterruptTriggered ||
      encoder.isWakeSignal()) {

    bool wasTouch = touchInterruptTriggered;
    bool wasButton = encoder.isWakeSignal();

    touchInterruptTriggered = false;
    imuInterruptTriggered = false;
    motion.clearInterrupt(); // turn off alarm in MPU6050
    encoder.clearInterrupts();

    if (!display.isScreenAwake()) {
      if (wasTouch || wasButton || motion.isTiltedUp()) {
        display.wakeUp();

        backlight.wakeUp();

        detachInterrupt(TOUCH_IRQ);
        detachInterrupt(IMU_INT);

        lastActivityTime = millis();
        Serial.println("System Woken Up by Interrupt!");

        // force full redraw of the card when waking up
        cards[currentCardIndex]->onShow(display.getTFT());
        delay(50);
        return;
      }
    }
  }

  // 2. process ui only if screen is active
  if (display.isScreenAwake()) {
    motion.update();
    backlight.update();

    cards[currentCardIndex]->onUpdate(display.getTFT());

    Gesture touchAction = display.getGesture();
    EncoderEvent encAction = encoder.getEvent();

    // if user did something, reset the sleep timer
    if (touchAction != Gesture::none || encAction != EncoderEvent::NONE ||
        display.isCurrentlyTouched()) {
      lastActivityTime = millis();

      switch (touchAction) {
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

      switch (encAction) {
      case EncoderEvent::CLICK:
        Serial.println("Encoder: BUTTON CLICK");
        switchCard(0); // to default card - time
        break;
      case EncoderEvent::RIGHT:
        Serial.println("Encoder: ROTATE RIGHT");
        switchCard((currentCardIndex + 1) %
                   NUM_CARDS); // next card - life swipe left
        break;
      case EncoderEvent::LEFT:
        Serial.println("Encoder: ROTATE LEFT");
        switchCard((currentCardIndex - 1 + NUM_CARDS) %
                   NUM_CARDS); // previous card
        break;
      }
    }

    // 3. check if it is time to sleep
    if (millis() - lastActivityTime > SLEEP_TIMEOUT) {
      if (!cards[currentCardIndex]->blocksSleep()) {
        Serial.println("Going to sleep to save power...");

        backlight.sleep();
        display.sleep();

        touchInterruptTriggered = false;
        imuInterruptTriggered = false;
        motion.clearInterrupt(); // clean old motion

        attachInterrupt(TOUCH_IRQ, touchWakeISR, FALLING);
        attachInterrupt(IMU_INT, imuWakeISR, FALLING);
      } else {
        lastActivityTime = millis();
      }
    }
  }

  // small delay to let the cpu breathe
  delay(10);
}
