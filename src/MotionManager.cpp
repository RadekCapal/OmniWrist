#include "MotionManager.h"

void MotionManager::begin() {
  if (!mpu.begin()) {
    Serial.println("[IMU] MPU6050 NOT FOUND!");
    return;
  }

  // sensitivity setting for motion detection
  mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
  mpu.setMotionDetectionThreshold(4);  // motion sensitivity (1-5)
  mpu.setMotionDetectionDuration(100); // how long does the motion have to be

  // INT pin setup to work like touch screen (active in LOW)
  mpu.setInterruptPinPolarity(true); // true = Active LOW
  mpu.setInterruptPinLatch(true);    // hold signal until we read it
  mpu.setMotionInterrupt(true);      // turn on interrupt when motion

  isReady = true;
  Serial.println("[IMU] MPU6050 Initialized with Motion Interrupt");
}

void MotionManager::clearInterrupt() {
  if (isReady) {
    // by reading sensors register INT signal will reset
    mpu.getMotionInterruptStatus();
  }
}

bool MotionManager::isTiltedUp() {
  if (!isReady)
    return false;

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // Z-axis points perpendicularly out of the display
  // normal Earth gravity is ~9.8 m/s^2
  // if z-axis is bigger than 5.0, that means displej is poiting mostly up
  if (a.acceleration.z > 5.0) {
    return true;
  }

  return false;
}
