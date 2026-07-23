#include "ir.h"

#include <IRremote.hpp>

#include "config.h"

void irBegin()
{
    IrReceiver.begin(config::IR_RECEIVER_PIN, DISABLE_LED_FEEDBACK);

    if (config::IR_DEBUG) {
        Serial.begin(config::FC_SERIAL_BAUD);
        Serial.println(F("IR learning mode: press each remote button."));
    }
}

bool irReadEvent(IrEvent &event)
{
    if (!IrReceiver.decode()) {
        return false;
    }

    event.address = IrReceiver.decodedIRData.address;
    event.command = IrReceiver.decodedIRData.command;
    event.rawCode = static_cast<IrRawCode>(IrReceiver.decodedIRData.decodedRawData);
    event.isRepeat = (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) != 0;

    if (config::IR_DEBUG) {
        Serial.print(F("address=0x"));
        Serial.print(event.address, HEX);
        Serial.print(F(" command=0x"));
        Serial.print(event.command, HEX);
        Serial.print(F(" raw=0x"));
        Serial.print(event.rawCode, HEX);
        Serial.print(F(" repeat="));
        Serial.println(event.isRepeat ? F("yes") : F("no"));
    }

    IrReceiver.resume();
    return true;
}
