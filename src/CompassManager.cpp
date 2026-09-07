#include "CompassManager.h"
#include <Wire.h>
#include <math.h>

CompassManager compassManager;

void CompassManager::begin() {
  Serial.println("==================================");
  Serial.println("[COMPASS] Booting new generation QMC (Control at 0x0A)...");

  // 1. The REAL Control Register for this variant is 0x0A
  // Writing 0x05 sets Continuous Mode and 50Hz output rate
  Wire.beginTransmission(0x2C);
  Wire.write(0x0A);
  Wire.write(0x05);
  Wire.endTransmission();
  delay(50);

  // 2. Control Register 2 (0x0B) - Set/Reset period
  Wire.beginTransmission(0x2C);
  Wire.write(0x0B);
  Wire.write(0x01);
  Wire.endTransmission();
  delay(50);

  Serial.println("[COMPASS] Start command sent to 0x0A.");
  Serial.println("==================================");
}

void CompassManager::update() {
  static uint32_t lastRead = 0;
  if (millis() - lastRead < 100)
    return; // 10Hz limit to let CPU breathe
  lastRead = millis();

  // Read 6 bytes of data starting from 0x00 (X_LSB to Z_MSB)
  Wire.beginTransmission(0x2C);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.requestFrom(0x2C, 6);

  if (Wire.available() >= 6) {
    // Combine low and high bytes into 16-bit integers
    int16_t x = Wire.read() | (Wire.read() << 8);
    int16_t y = Wire.read() | (Wire.read() << 8);
    int16_t z = Wire.read() | (Wire.read() << 8);

    // Calculate heading (azimuth)
    float heading = atan2(y, x);
    if (heading < 0) {
      heading += 2 * PI;
    }
    currentAzimuth = (int)(heading * 180.0 / PI);

    // --- DIAGNOSTIC PRINT ---
    static uint32_t lastDebugPrint = 0;
    if (millis() - lastDebugPrint > 500) {
      Serial.print("[COMPASS DEBUG] X: ");
      Serial.print(x);
      Serial.print(" | Y: ");
      Serial.print(y);
      Serial.print(" | Z: ");
      Serial.print(z);
      Serial.print(" | Azimuth: ");
      Serial.println(currentAzimuth);
      lastDebugPrint = millis();
    }
  }
}

int CompassManager::getAzimuth() { return currentAzimuth; }

String CompassManager::getDirectionString(int azimuth) {
  if (azimuth < 22 || azimuth >= 338)
    return "N";
  if (azimuth < 67)
    return "NE";
  if (azimuth < 112)
    return "E";
  if (azimuth < 157)
    return "SE";
  if (azimuth < 202)
    return "S";
  if (azimuth < 247)
    return "SW";
  if (azimuth < 292)
    return "W";
  return "NW";
}
