#include "communication.h"

namespace {
const uint8_t MSP_ALTITUDE = 109;
const uint8_t MSP_SET_RAW_RC = 200;
const uint8_t MSP_MAX_PAYLOAD = 32;

enum ParserState {
    WAIT_DOLLAR,
    WAIT_M,
    WAIT_DIRECTION,
    WAIT_SIZE,
    WAIT_COMMAND,
    WAIT_PAYLOAD,
    WAIT_CHECKSUM
};

ParserState parserState = WAIT_DOLLAR;
uint8_t payloadSize = 0;
uint8_t payloadIndex = 0;
uint8_t command = 0;
uint8_t checksum = 0;
uint8_t payload[MSP_MAX_PAYLOAD];
int32_t latestAltitudeCm = 0;
uint32_t altitudeUpdatedAtMs = 0;

void sendMsp(uint8_t messageCommand, const uint8_t *messagePayload, uint8_t messageSize)
{
    uint8_t packetChecksum = messageSize ^ messageCommand;

    Serial.write('$');
    Serial.write('M');
    Serial.write('<');
    Serial.write(messageSize);
    Serial.write(messageCommand);

    for (uint8_t i = 0; i < messageSize; ++i) {
        Serial.write(messagePayload[i]);
        packetChecksum ^= messagePayload[i];
    }

    Serial.write(packetChecksum);
}

int32_t readSigned32(const uint8_t *bytes)
{
    return static_cast<int32_t>(
        static_cast<uint32_t>(bytes[0]) |
        (static_cast<uint32_t>(bytes[1]) << 8) |
        (static_cast<uint32_t>(bytes[2]) << 16) |
        (static_cast<uint32_t>(bytes[3]) << 24));
}

void handleFrame()
{
    // INAV MSP_ALTITUDE begins with a signed, centimetre-based altitude.
    if (command == MSP_ALTITUDE && payloadSize >= 4) {
        latestAltitudeCm = readSigned32(payload);
        altitudeUpdatedAtMs = millis();
    }
}

void resetParser()
{
    parserState = WAIT_DOLLAR;
    payloadSize = 0;
    payloadIndex = 0;
    checksum = 0;
}
}

void fcBegin()
{
    if (!config::IR_DEBUG) {
        Serial.begin(config::FC_SERIAL_BAUD);
    }
    resetParser();
}

void fcUpdate()
{
    while (Serial.available() > 0) {
        const uint8_t value = static_cast<uint8_t>(Serial.read());

        switch (parserState) {
        case WAIT_DOLLAR:
            if (value == '$') parserState = WAIT_M;
            break;

        case WAIT_M:
            parserState = (value == 'M') ? WAIT_DIRECTION : WAIT_DOLLAR;
            break;

        case WAIT_DIRECTION:
            // '<' is a command received by the FC; '>' is an FC response.
            parserState = (value == '>' || value == '!') ? WAIT_SIZE : WAIT_DOLLAR;
            break;

        case WAIT_SIZE:
            if (value > MSP_MAX_PAYLOAD) {
                resetParser();
            } else {
                payloadSize = value;
                payloadIndex = 0;
                checksum = value;
                parserState = WAIT_COMMAND;
            }
            break;

        case WAIT_COMMAND:
            command = value;
            checksum ^= value;
            parserState = payloadSize == 0 ? WAIT_CHECKSUM : WAIT_PAYLOAD;
            break;

        case WAIT_PAYLOAD:
            payload[payloadIndex++] = value;
            checksum ^= value;
            if (payloadIndex == payloadSize) parserState = WAIT_CHECKSUM;
            break;

        case WAIT_CHECKSUM:
            if (checksum == value) handleFrame();
            resetParser();
            break;
        }
    }
}

void fcRequestAltitude()
{
    sendMsp(MSP_ALTITUDE, 0, 0);
}

void fcSendRc(const uint16_t channels[config::RC_CHANNEL_COUNT])
{
    uint8_t message[config::RC_CHANNEL_COUNT * 2];

    for (uint8_t channelIndex = 0; channelIndex < config::RC_CHANNEL_COUNT; ++channelIndex) {
        const uint16_t value = channels[channelIndex];
        message[channelIndex * 2] = value & 0xFF;
        message[channelIndex * 2 + 1] = value >> 8;
    }

    sendMsp(MSP_SET_RAW_RC, message, sizeof(message));
}

bool fcAltitudeIsFresh()
{
    return altitudeUpdatedAtMs != 0 &&
           (millis() - altitudeUpdatedAtMs) <= config::ALTITUDE_STALE_MS;
}

int32_t fcAltitudeCm()
{
    return latestAltitudeCm - config::HEIGHT_OFFSET_CM;
}
