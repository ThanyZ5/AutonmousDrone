#include "mission.h"   // Gives access to the mission functions

// setup() runs ONE time when the Arduino powers on or resets.
void setup()
{
    // Initialize everything needed for the mission
    missionSetup();
}

// loop() runs over and over for as long as the Arduino has power.
void loop()
{
    // Continuously update the mission.
    // This function will decide what the drone should do next.
    missionUpdate();
}