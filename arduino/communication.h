#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <Arduino.h>

#include "config.h"

// Minimal MSP v1 implementation for an INAV flight controller. It sends
// MSP_SET_RAW_RC and reads the first field of MSP_ALTITUDE (height in cm).
void fcBegin();
void fcUpdate();
void fcRequestAltitude();
void fcSendRc(const uint16_t channels[config::RC_CHANNEL_COUNT]);
bool fcAltitudeIsFresh();
int32_t fcAltitudeCm();

#endif
