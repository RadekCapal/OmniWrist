#ifndef COMPASS_MANAGER_H
#define COMPASS_MANAGER_H

#include <Arduino.h>

class CompassManager {
public:
  void begin();
  void update();
  int getAzimuth();
  String getDirectionString(int azimuth);

private:
  int currentAzimuth = 0;
};

extern CompassManager compassManager;

#endif
