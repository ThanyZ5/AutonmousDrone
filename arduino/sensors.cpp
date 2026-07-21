#include "sensors.h"

#include "communication.h"

namespace {
uint32_t lastAltitudeRequestAtMs = 0;
}

void sensorsBegin()
{
    lastAltitudeRequestAtMs = 0;
}

void sensorsUpdate()
{
    const uint32_t now = millis();
    if (now - lastAltitudeRequestAtMs >= config::FC_ALTITUDE_REQUEST_MS) {
        fcRequestAltitude();
        lastAltitudeRequestAtMs = now;
    }
}

bool sensorsHeightIsFresh()
{
    return fcAltitudeIsFresh();
}

int32_t sensorsHeightCm()
{
    return fcAltitudeCm();
}
