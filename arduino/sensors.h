#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

// The MTF-01 is wired to the F405, not to the Uno. INAV receives the sensor's
// MSP rangefinder/optical-flow data and the Uno asks INAV for its altitude.
void sensorsBegin();
void sensorsUpdate();
bool sensorsHeightIsFresh();
int32_t sensorsHeightCm();

#endif
