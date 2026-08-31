#ifndef MOTION_MANAGER_H
#define MOTION_MANAGER_H

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>

class MotionManager {
private:
  Adafruit_MPU6050 mpu;
  bool isReady = false;

public:
  void begin();
  void clearInterrupt();
  bool isTiltedUp();
};

#endif // MOTION_MANAGER_H
