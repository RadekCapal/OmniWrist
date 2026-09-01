#include "MotionManager.h"
#include <math.h> // needed for sqrt() function

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

// core logic for step detection
void MotionManager::update() {
  if (!isReady)
    return;

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // calculate 3d vector magnitude
  float rawMag = sqrt((a.acceleration.x * a.acceleration.x) +
                      (a.acceleration.y * a.acceleration.y) +
                      (a.acceleration.z * a.acceleration.z));

  // 1. low-pass filter: smooths out sharp taps and vibrations (allows only slow
  // human movement) 85% of old value + 15% of new value
  lpfMagnitude = (lpfMagnitude * 0.85) + (rawMag * 0.15);

  // 2. hysteresis: requires a solid rise and fall to count as one step
  float UPPER_THRESHOLD = 11.5; // push required to start a step
  float LOWER_THRESHOLD = 10.2; // gravity baseline to finish a step

  // step starts (foot hits the ground)
  if (!isStepActive && lpfMagnitude > UPPER_THRESHOLD) {
    isStepActive = true;

    uint32_t now = millis();
    // human limit max ~3 steps per sec
    if (now - lastStepTime > 300) {
      stepCount++;
      lastStepTime = now;
    }
  }
  // step ends (hand returns to normal resting gravity)
  else if (isStepActive && lpfMagnitude < LOWER_THRESHOLD) {
    isStepActive = false; // step finished, ready for the next one
  }
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

  // z-axis points perpendicularly out of the display
  // normal earth gravity is ~9.8 m/s^2
  // if z-axis is bigger than 5.0, that means displej is poiting mostly up
  if (a.acceleration.z > 5.0) {
    return true;
  }

  return false;
}

uint32_t MotionManager::getSteps() { return stepCount; }
