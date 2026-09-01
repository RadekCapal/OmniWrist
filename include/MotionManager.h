#ifndef MOTION_MANAGER_H
#define MOTION_MANAGER_H

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>

class MotionManager {
private:
  Adafruit_MPU6050 mpu;
  bool isReady = false;

  uint32_t stepCount = 0;
  uint32_t lastStepTime = 0;

  // variables for digital signal processing
  float lpfMagnitude = 9.81; // starts at earth gravity
  bool isStepActive = false; // tracks if foot is currently in the air

public:
  void begin();
  void update();
  void clearInterrupt();
  bool isTiltedUp();
  uint32_t getSteps();
};

#endif // MOTION_MANAGER_H
