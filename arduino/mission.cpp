#include "mission.h"

#include <Arduino.h>

#include "communication.h"
#include "config.h"
#include "ir.h"
#include "led.h"
#include "sensors.h"

namespace {
enum MissionState {
    IDLE,
    ARMING,
    TAKEOFF,
    HOVER,
    LANDING,
    TOUCHDOWN
};

MissionState state = IDLE;
uint16_t rc[config::RC_CHANNEL_COUNT];
uint8_t selectedHeightFeet = 0;
int32_t targetHeightCm = 0;
uint32_t stateStartedAtMs = 0;
uint32_t inTargetBandSinceMs = 0;
uint32_t lastRcSentAtMs = 0;

int32_t absoluteValue(int32_t value)
{
    return value < 0 ? -value : value;
}

void setRcDefaults()
{
    rc[config::RC_ROLL] = config::RC_MID;
    rc[config::RC_PITCH] = config::RC_MID;
    rc[config::RC_YAW] = config::RC_MID;
    rc[config::RC_THROTTLE] = config::RC_LOW;
    rc[config::RC_AUX_ARM] = config::RC_LOW;
    rc[config::RC_AUX_ANGLE] = config::RC_HIGH;
    rc[config::RC_AUX_ALTHOLD] = config::RC_LOW;
    rc[config::RC_AUX_UNUSED] = config::RC_LOW;
}

void enterState(MissionState nextState)
{
    state = nextState;
    stateStartedAtMs = millis();
    inTargetBandSinceMs = 0;

    // Green means the aircraft is taking off or holding its requested height.
    ledSetGreen(nextState == TAKEOFF || nextState == HOVER);
}

int8_t commandToHeight(uint32_t code)
{
    if (code == config::IR_COMMAND_1) return 1;
    if (code == config::IR_COMMAND_2) return 2;
    if (code == config::IR_COMMAND_3) return 3;
    if (code == config::IR_COMMAND_4) return 4;
    if (code == config::IR_COMMAND_5) return 5;
    if (code == config::IR_COMMAND_6) return 6;
    if (code == config::IR_COMMAND_7) return 7;
    if (code == config::IR_COMMAND_8) return 8;
    if (code == config::IR_COMMAND_9) return 9;
    if (code == config::IR_COMMAND_0) return 10;
    return 0;
}

void beginMission()
{
    // Do not arm if the F405 is not returning a current height. This catches a
    // missing/unconfigured MTF-01 and a disconnected Arduino-to-FC serial link.
    if (selectedHeightFeet == 0 || !sensorsHeightIsFresh()) {
        return;
    }

    targetHeightCm = static_cast<int32_t>(
        static_cast<float>(selectedHeightFeet) * config::CENTIMETRES_PER_FOOT);
    setRcDefaults();
    enterState(ARMING);
}

uint16_t altitudeControlThrottle(int32_t heightErrorCm)
{
    if (absoluteValue(heightErrorCm) <= config::TARGET_TOLERANCE_CM) {
        return config::RC_MID;
    }

    const float requested = config::RC_MID +
                            (static_cast<float>(heightErrorCm) * config::HEIGHT_TO_THROTTLE_GAIN);

    if (requested > config::CLIMB_THROTTLE_MAX) return config::CLIMB_THROTTLE_MAX;
    if (requested < config::DESCENT_THROTTLE_MIN) return config::DESCENT_THROTTLE_MIN;
    return static_cast<uint16_t>(requested);
}

void land()
{
    if (state != IDLE && state != LANDING && state != TOUCHDOWN) {
        enterState(LANDING);
    }
}

void processIr()
{
    IrEvent event;
    if (!irReadEvent(event) || event.isRepeat) {
        return;
    }

    const uint32_t code = event.rawCode;
    const int8_t chosenHeight = commandToHeight(code);
    if (chosenHeight != 0 && state == IDLE) {
        selectedHeightFeet = static_cast<uint8_t>(chosenHeight);
        return;
    }

    if (code == config::IR_COMMAND_START && state == IDLE) {
        beginMission();
    } else if (code == config::IR_COMMAND_LAND) {
        land();
    }
}

void updateMissionState()
{
    const uint32_t elapsed = millis() - stateStartedAtMs;

    switch (state) {
    case IDLE:
        setRcDefaults();
        ledSetGreen(false);
        break;

    case ARMING:
        setRcDefaults();
        rc[config::RC_AUX_ARM] = config::RC_HIGH;
        if (elapsed >= config::ARMING_TIME_MS) {
            enterState(TAKEOFF);
        }
        break;

    case TAKEOFF: {
        rc[config::RC_AUX_ARM] = config::RC_HIGH;
        rc[config::RC_AUX_ALTHOLD] = config::RC_HIGH;

        if (!sensorsHeightIsFresh() || elapsed >= config::TAKEOFF_TIMEOUT_MS) {
            land();
            break;
        }

        const int32_t errorCm = targetHeightCm - sensorsHeightCm();
        rc[config::RC_THROTTLE] = altitudeControlThrottle(errorCm);

        if (absoluteValue(errorCm) <= config::TARGET_TOLERANCE_CM) {
            if (inTargetBandSinceMs == 0) inTargetBandSinceMs = millis();
            if (millis() - inTargetBandSinceMs >= 1000) {
                enterState(HOVER);
            }
        } else {
            inTargetBandSinceMs = 0;
        }
        break;
    }

    case HOVER:
        rc[config::RC_AUX_ARM] = config::RC_HIGH;
        rc[config::RC_AUX_ALTHOLD] = config::RC_HIGH;
        rc[config::RC_THROTTLE] = config::RC_MID;
        if (!sensorsHeightIsFresh() || elapsed >= config::HOVER_TIME_MS) {
            land();
        }
        break;

    case LANDING:
        rc[config::RC_AUX_ARM] = config::RC_HIGH;
        rc[config::RC_AUX_ALTHOLD] = config::RC_HIGH;
        rc[config::RC_THROTTLE] = config::DESCENT_THROTTLE_MIN;

        if ((sensorsHeightIsFresh() && sensorsHeightCm() <= config::TOUCHDOWN_HEIGHT_CM) ||
            elapsed >= config::LANDING_TIMEOUT_MS) {
            enterState(TOUCHDOWN);
        }
        break;

    case TOUCHDOWN:
        // Keep throttle low briefly, then turn the motor command and LED off.
        setRcDefaults();
        if (elapsed >= config::TOUCHDOWN_DISARM_DELAY_MS) {
            enterState(IDLE);
        }
        break;
    }
}

void sendRcIfDue()
{
    const uint32_t now = millis();
    if (now - lastRcSentAtMs >= config::RC_SEND_INTERVAL_MS) {
        fcSendRc(rc);
        lastRcSentAtMs = now;
    }
}
}

// missionSetup() runs only once when the Arduino powers on.
void missionSetup()
{
    ledBegin();
    irBegin();
    fcBegin();
    sensorsBegin();
    setRcDefaults();
    enterState(IDLE);
}

// missionUpdate() runs continuously while the Arduino is powered.
void missionUpdate()
{
    // Learning mode is deliberately isolated from the flight controller. It
    // only prints the remote's values and never emits motor/RC commands.
    if (config::IR_DEBUG) {
        processIr();
        return;
    }

    fcUpdate();
    sensorsUpdate();
    processIr();
    updateMissionState();
    sendRcIfDue();
}
