#include "EncoderManager.h"

volatile int encoderCount = 0;
volatile bool buttonPressed = false;

// for debouncing
uint32_t lastEncoderTime = 0;
uint32_t lastButtonTime = 0;

void IRAM_ATTR encoderISR() {
  uint32_t now = millis();
  // filtration of noise
  if (now - lastEncoderTime > 50) {
    int dtValue = digitalRead(ENC_DT);
    if (dtValue == HIGH) {
      encoderCount++; // by direction (right)
    } else {
      encoderCount--; // against the direction (left)
    }
    lastEncoderTime = now;
  }
}

void IRAM_ATTR encoderButtonISR() {
  uint32_t now = millis();
  if (now - lastButtonTime > 250) { // double click protection
    buttonPressed = true;
    lastButtonTime = now;
  }
}

void EncoderManager::begin() {
  pinMode(ENC_CLK, INPUT_PULLUP);
  pinMode(ENC_DT, INPUT_PULLUP);
  pinMode(ENC_SW, INPUT_PULLUP);

  // connect interrupts
  attachInterrupt(ENC_CLK, encoderISR, FALLING);
  attachInterrupt(ENC_SW, encoderButtonISR, FALLING);
}

EncoderEvent EncoderManager::getEvent() {
  if (buttonPressed) {
    buttonPressed = false;
    return EncoderEvent::CLICK;
  }

  if (encoderCount > 0) {
    encoderCount--; //
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
