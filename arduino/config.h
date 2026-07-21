#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

/*
 * Hardware and mission settings
 * =============================
 *
 * This project uses the Arduino as a safe command source, not as a motor
 * controller. The SpeedyBee F405 Mini, running INAV, remains responsible for
 * stabilising the quad and driving the ESCs.
 *
 * Before the first real flight, set IR_DEBUG to true, disconnect the F405
 * serial wires, open the Arduino Serial Monitor at 115200 baud, and press
 * every remote button. Put the reported command values into the IR_COMMAND_*
 * settings below. Set IR_DEBUG back to false before reconnecting the F405.
 */
namespace config {

// Arduino Uno pins.
const uint8_t IR_RECEIVER_PIN = 2;       // OUT pin of the IR receiver.
const uint8_t GREEN_LED_PIN = 7;         // Through a 220 ohm resistor to LED anode.

// Set true only while the Uno is connected to a computer for IR code learning.
// Do not leave this true while D0/D1 are connected to the F405.
const bool IR_DEBUG = false;

// Common 21-button NEC remote command values. Replace these if your IR test
// prints different values. Button 0 selects 10 ft; PLAY starts; POWER lands.
const uint16_t IR_COMMAND_0 = 0x16;
const uint16_t IR_COMMAND_1 = 0x0C;
const uint16_t IR_COMMAND_2 = 0x18;
const uint16_t IR_COMMAND_3 = 0x5E;
const uint16_t IR_COMMAND_4 = 0x08;
const uint16_t IR_COMMAND_5 = 0x1C;
const uint16_t IR_COMMAND_6 = 0x5A;
const uint16_t IR_COMMAND_7 = 0x42;
const uint16_t IR_COMMAND_8 = 0x52;
const uint16_t IR_COMMAND_9 = 0x4A;
const uint16_t IR_COMMAND_START = 0x44; // PLAY/PAUSE.
const uint16_t IR_COMMAND_LAND = 0x45;  // POWER.

// MSP serial connection: Uno D1/TX -> (level shifted) F405 R3, and F405 T3
// -> Uno D0/RX. The F405 port must be set to MSP at this baud rate in INAV.
const uint32_t FC_SERIAL_BAUD = 115200UL;
const uint16_t FC_ALTITUDE_REQUEST_MS = 100; // 10 Hz altitude updates.
const uint16_t RC_SEND_INTERVAL_MS = 40;     // 25 Hz MSP RC frames.
const uint16_t ALTITUDE_STALE_MS = 350;

// MSP RC channel order is Roll, Pitch, Yaw, Throttle, AUX1, AUX2, AUX3, AUX4.
const uint8_t RC_CHANNEL_COUNT = 8;
const uint8_t RC_ROLL = 0;
const uint8_t RC_PITCH = 1;
const uint8_t RC_YAW = 2;
const uint8_t RC_THROTTLE = 3;
const uint8_t RC_AUX_ARM = 4;
const uint8_t RC_AUX_ANGLE = 5;
const uint8_t RC_AUX_ALTHOLD = 6;
const uint8_t RC_AUX_UNUSED = 7;
const uint16_t RC_LOW = 1000;
const uint16_t RC_MID = 1500;
const uint16_t RC_HIGH = 2000;

// Height command limits. The MTF-01 rangefinder can measure well beyond this
// assignment's 1-10 ft range; this code deliberately cannot command more.
const uint8_t MIN_TARGET_FEET = 1;
const uint8_t MAX_TARGET_FEET = 10;
const float CENTIMETRES_PER_FOOT = 30.48f;

// If the height reported by INAV is not zero when the drone is sitting on the
// floor, put that measured value here. Leave at 0 when INAV reports AGL height.
const int32_t HEIGHT_OFFSET_CM = 0;

// Timing and vertical-speed limits. Tune only after a tethered, props-on test
// in a clear safety area. NAV ALTHOLD must be configured with 1500 as neutral.
const uint16_t ARMING_TIME_MS = 2000;
const uint16_t TAKEOFF_TIMEOUT_MS = 25000;
const uint16_t HOVER_TIME_MS = 10000;
const uint16_t LANDING_TIMEOUT_MS = 30000;
const uint16_t TOUCHDOWN_DISARM_DELAY_MS = 1200;
const int32_t TARGET_TOLERANCE_CM = 12;
const int32_t TOUCHDOWN_HEIGHT_CM = 15;
const uint16_t CLIMB_THROTTLE_MAX = 1580;
const uint16_t DESCENT_THROTTLE_MIN = 1420;
const float HEIGHT_TO_THROTTLE_GAIN = 1.4f;
}

#endif
