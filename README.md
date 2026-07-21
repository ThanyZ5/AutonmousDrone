# Autonomous Drone: IR Height Mission

This Arduino Uno code commands a SpeedyBee F405 Mini for the class mission:

1. Press **1-9** on the IR remote to choose that height in feet; press **0**
   to choose 10 ft.
2. Press **PLAY** to start the mission.
3. The F405 arms, climbs to the selected height, hovers for 10 seconds, then
   lands and disarms.
4. The green LED is on during takeoff and hover. It turns off at the beginning
   of landing. Press **POWER** at any time after arming for a controlled landing.

The code will not arm unless the F405 is returning fresh altitude data. This
is intentional: a missing sensor, wrong serial wiring, or wrong INAV setup
must not turn into a blind takeoff.

## Required flight-controller setup

Use **INAV**, not Betaflight, for this assignment. The MTF-01 publishes its
rangefinder/optical-flow data as MSP and officially supports INAV. Configure
the F405 Mini with the `SPEEDYBEEF405MINI` target and a standard quadcopter
mixer, calibrate its accelerometer, and confirm all four motors spin in the
correct order **with propellers removed**.

In INAV Configurator:

- On **Ports**, configure UART3 as **MSP**, 115200 baud (Arduino link).
- On **Ports**, configure UART6 as **Sensor Input: MSP**, 115200 baud
  (MTF-01 link).
- On **Configuration**, set the receiver type to **MSP RX**. Do not enable
  Serial RX on UART3.
- On **Configuration**, enable the MSP rangefinder and optical-flow inputs.
  Confirm a live rangefinder/altitude reading before continuing.
- On **Modes**, assign these Arduino RC channels: AUX1 = ARM, AUX2 = ANGLE,
  and AUX3 = NAV ALTHOLD. Configure each active range as 1700-2100.
- Set `nav_use_midthr_for_althold` on, so throttle **1500** means hold height.
- Configure Failsafe to **land/disarm** if MSP RC data stops. The code sends
  frames at 25 Hz; INAV needs updates at least 5 Hz.

Keep the MTF-01 protocol set to **MSP / INAV** using the vendor's MicoAssistant
program before wiring it to the flight controller.

## Wiring

All grounds must be connected. Never put the Arduino Uno's 5 V serial output
directly into a 3.3 V F405 input.

| Part | Connect to | Notes |
| --- | --- | --- |
| IR receiver VCC | Uno 5V | Verify your receiver supports 5 V. |
| IR receiver GND | Uno GND | |
| IR receiver OUT | Uno D2 | |
| Green LED anode | Uno D7 through 220 ohm resistor | LED cathode to Uno GND. |
| Arduino D1 / TX | F405 R3 through a 5 V -> 3.3 V divider | 1 k ohm from D1 to R3, 2 k ohm from R3 to GND works. |
| Arduino D0 / RX | F405 T3 | 3.3 V from the F405 is safe as an Uno logic high. |
| Arduino GND | F405 GND | Required for serial communication. |
| MTF-01 TX | F405 R6 | |
| MTF-01 RX | F405 T6 | |
| MTF-01 5V/GND | F405 5V/GND | Point the sensor straight down. |

For flight power, use the F405's regulated 5 V output to the Uno **5V** pin
(not VIN) and share ground. Disconnect that 5 V connection while the Uno is
powered by USB for programming. Disconnect the D0/D1 serial wires while
uploading, otherwise the F405 can interfere with the upload.

## Install and customize

1. In Arduino IDE Library Manager, install **IRremote** by Armin Joachimsmeyer.
2. Open `arduino/AutonomousDrone.ino` and select **Arduino Uno**.
3. Learn your remote codes: set `IR_DEBUG = true` in `arduino/config.h`,
   disconnect F405 T3/R3, upload, and open Serial Monitor at 115200. Press the
   number, PLAY, and POWER buttons. Copy each printed `command=0x...` value to
   its matching `IR_COMMAND_*` setting. Set `IR_DEBUG` back to `false`.
4. Reconnect the F405 serial wires, upload again, and verify that the INAV
   Receiver tab shows the expected channel positions with props removed.
5. Confirm MTF-01 height updates in INAV. With the drone on the floor, adjust
   `HEIGHT_OFFSET_CM` only if INAV reports a non-zero height.
6. Test selected height, LED, arming, hover, and landing while tethered, in a
   clear area, with an experienced spotter. Start at 1 ft.

Mission constants, including the 10-second hover time and conservative ascent
and descent limits, are all together in `arduino/config.h`.
