#include "HeartRateCard.h"

#include "heartRate.h" // We go back to the proven SparkFun algorithm

void HeartRateCard::begin() {
  if (particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    isSensorFound = true;

    byte ledBrightness = 60; // 60 is strong enough to penetrate skin, but
                             // doesn't blind the sensor
    byte sampleAverage = 4;  // Less smoothing so peaks are actually visible
    byte ledMode = 2;        // Red and IR
    int sampleRate = 400;    // 400Hz / 4 (average) = 100Hz output (perfectly
                             // matches our 10ms loop delay)
    int pulseWidth = 411;    // Maximum resolution
    int adcRange = 4096;

    particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate,
                         pulseWidth, adcRange);
    particleSensor.shutDown();

    for (byte i = 0; i < RATE_SIZE; i++)
      rates[i] = 0;
  }
}

void HeartRateCard::onShow(TFT_eSPI *tft) {
  tft->fillScreen(TFT_BLACK);
  tft->drawFastHLine(0, 20, 320, TFT_DARKGREY);

  tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
  tft->setTextDatum(TC_DATUM);
  tft->setTextSize(1);
  tft->drawString("HEART RATE", 160, 5);

  if (!isSensorFound) {
    tft->setTextColor(TFT_RED, TFT_BLACK);
    tft->setTextDatum(MC_DATUM);
    tft->drawString("MAX30102 NOT FOUND", 160, 120);
  } else {
    particleSensor.wakeUp();

    tft->setTextColor(TFT_WHITE, TFT_BLACK);
    tft->setTextDatum(MC_DATUM);
    tft->setTextSize(2);
    tft->drawString("Place sensor", 160, 60);
    tft->drawString("on skin...", 160, 90);

    beatAvg = 0;
    beatsPerMinute = 0;
    rateSpot = 0;
    validBeats = 0;
    rejectedBeats = 0;
    lastBeat = millis();
  }

  lastDisplayUpdate = 0;
}

void HeartRateCard::onUpdate(TFT_eSPI *tft) {
  if (!isSensorFound)
    return;

  long irValue = particleSensor.getIR();

  // If IR is above 50k, skin is detected
  if (irValue > 50000) {
    isMeasuring = true;

    // TEMPORARY DEBUG: Send raw values to PC
    // Serial.println(irValue);

    if (checkForBeat(irValue) == true) {
      long delta = millis() - lastBeat;
      lastBeat = millis();

      beatsPerMinute = 60 / (delta / 1000.0);

      // Human limits (40 to 200 BPM)
      if (beatsPerMinute < 200 && beatsPerMinute > 40) {

        // Self-healing filter: accept if it's the first beats, OR if it's
        // within 30 BPM of current average
        if (validBeats < 2 || abs(beatsPerMinute - beatAvg) < 30) {
          rates[rateSpot++] = (byte)beatsPerMinute;
          rateSpot %= RATE_SIZE;

          if (validBeats < RATE_SIZE)
            validBeats++;
          rejectedBeats = 0; // Reset error counter

          // Calculate new average
          beatAvg = 0;
          for (byte x = 0; x < RATE_SIZE; x++) {
            if (rates[x] > 0)
              beatAvg += rates[x];
          }
          beatAvg /= validBeats;

        } else {
          // Reject anomaly
          rejectedBeats++;

          // Reset filter if 3 anomalies occur in a row
          if (rejectedBeats >= 3) {
            beatAvg = 0;
            validBeats = 0;
            rateSpot = 0;
            rejectedBeats = 0;
            for (byte i = 0; i < RATE_SIZE; i++)
              rates[i] = 0;
          }
        }
      }
    }
  } else {
    isMeasuring = false;
    beatAvg = 0;
    validBeats = 0;
    rejectedBeats = 0;
    rateSpot = 0;
    for (byte i = 0; i < RATE_SIZE; i++)
      rates[i] = 0;
  }

  // UI rendering
  if (millis() - lastDisplayUpdate > 500) {
    lastDisplayUpdate = millis();
    tft->setTextDatum(MC_DATUM);

    tft->fillRect(0, 130, 320, 100, TFT_BLACK);

    if (irValue < 50000) {
      tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
      tft->setTextSize(2);
      tft->drawString("Waiting...", 160, 160);
    } else {
      tft->setTextColor(TFT_RED, TFT_BLACK);
      tft->setTextSize(6);

      char bpmStr[10];
      if (beatAvg == 0) {
        snprintf(bpmStr, sizeof(bpmStr), "---");
      } else {
        snprintf(bpmStr, sizeof(bpmStr), "%d", beatAvg);
      }

      tft->drawString(bpmStr, 160, 160);

      tft->setTextColor(TFT_DARKGREY, TFT_BLACK);
      tft->setTextSize(2);
      tft->drawString("BPM", 160, 210);
    }
  }
}

void HeartRateCard::onHide() {
  if (isSensorFound) {
    particleSensor.shutDown();
  }
}

bool HeartRateCard::blocksSleep() { return isMeasuring; }
