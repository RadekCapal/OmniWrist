#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>

class TimeManager {
public:
  // connects to wifi, syncs time via ntp and disconnects
  static void syncTime(const char *ssid, const char *password);
};

#endif // TIME_MANAGER_H
