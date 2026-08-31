#ifndef ENCODER_MANAGER_H
#define ENCODER_MANAGER_H

#include <Arduino.h>

enum class EncoderEvent { NONE, LEFT, RIGHT, CLICK };

class EncoderManager {
public:
  void begin();
  EncoderEvent getEvent();
  bool isWakeSignal();
  void clearInterrupts();
};

#endif // ENCODER_MANAGER_H
