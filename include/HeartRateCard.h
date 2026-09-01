#ifndef HEART_RATE_CARD_H
#define HEART_RATE_CARD_H

#include "UI_Card.h"
#include <Wire.h>

// fix for the i2c_buffer_length redefinition warning
#ifdef I2C_BUFFER_LENGTH
#undef I2C_BUFFER_LENGTH
#endif

#include "MAX30105.h"

class HeartRateCard : public UI_Card {
private:
  MAX30105 particleSensor;
  bool isSensorFound = false;
  bool isMeasuring = false;

  static const byte RATE_SIZE = 8;
  byte rates[RATE_SIZE];
  byte rateSpot = 0;
  long lastBeat = 0;

  float beatsPerMinute = 0;
  int beatAvg = 0;

  byte validBeats = 0;
  byte rejectedBeats = 0;

  uint32_t lastDisplayUpdate = 0;

  // variables for advanced dsp algorithm
  float dcFilterW = 0;
  float previousLpf = 0;
  float dynamicThreshold = 10.0;
  bool peakRising = false;

public:
  void begin();
  void onShow(TFT_eSPI *tft) override;
  void onUpdate(TFT_eSPI *tft) override;
  void onHide() override;
  bool blocksSleep() override;
};

#endif // HEART_RATE_CARD_H
