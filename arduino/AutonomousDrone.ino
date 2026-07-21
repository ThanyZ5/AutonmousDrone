#include "mission.h"

// Arduino sketch entry point. All mission logic lives in mission.cpp so it is
// easier to keep the hardware-specific pieces separate.
void setup()
{
    missionSetup();
}

void loop()
{
    missionUpdate();
}
