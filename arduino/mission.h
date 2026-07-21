#ifndef MISSION_H      // Prevents this file from being included more than once
#define MISSION_H

// Called once when the Arduino starts.
// Used to initialize sensors, LEDs, communication, etc.
void missionSetup();

// Called repeatedly from loop(). Controls the mission sequence:
// select height by IR -> start -> arm -> take off -> hover -> land -> disarm.
void missionUpdate();

#endif
