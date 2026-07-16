#ifndef MISSION_H      // Prevents this file from being included more than once
#define MISSION_H

// Called once when the Arduino starts.
// Used to initialize sensors, LEDs, communication, etc.
void missionSetup();

// Called repeatedly from loop().
// This controls the mission sequence:
// Wait for IR → Arm → Takeoff → Hover → Land
void missionUpdate();

#endif