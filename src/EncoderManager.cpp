#include "EncoderManager.h"

volatile int encoderCount = 0;
volatile bool buttonPressed = false;

// Only needed for the button now
uint32_t lastButtonTime = 0;

void IRAM_ATTR encoderISR() {
  static uint32_t lastInterruptTime = 0;
  uint32_t now = millis();

  if (now - lastInterruptTime > 100) {
    if (digitalRead(ENC_CLK) == LOW) {
      if (digitalRead(ENC_DT) == HIGH) {
        encoderCount++; // right
      } else {
        encoderCount--; // left
      }
    }
    lastInterruptTime = now;
  }
}

void IRAM_ATTR encoderButtonISR() {
  uint32_t now = millis();
  // double click
  if (now - lastButtonTime > 200) {
    buttonPressed = true;
    lastButtonTime = now;
  }
}

void EncoderManager::begin() {
  // External 10k resistors provide a much stronger pull-up
  pinMode(ENC_CLK, INPUT);
  pinMode(ENC_DT, INPUT);
  pinMode(ENC_SW, INPUT);

  attachInterrupt(ENC_CLK, encoderISR, FALLING);
  attachInterrupt(ENC_SW, encoderButtonISR, FALLING);
}

EncoderEvent EncoderManager::getEvent() {
  if (buttonPressed) {
    buttonPressed = false;
    return EncoderEvent::CLICK;
  }

  // Process the accumulated counts smoothly
  if (encoderCount > 0) {
    encoderCount--;
    return EncoderEvent::RIGHT;
  }

  if (encoderCount < 0) {
    encoderCount++;
    return EncoderEvent::LEFT;
  }

  return EncoderEvent::NONE;
}

bool EncoderManager::isWakeSignal() {
  if (buttonPressed) {
    buttonPressed = false;
    return true;
  }
  return false;
}

void EncoderManager::clearInterrupts() {
  encoderCount = 0;
  buttonPressed = false;
}
