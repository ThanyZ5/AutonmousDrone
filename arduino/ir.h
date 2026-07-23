#ifndef IR_H
#define IR_H

#include <Arduino.h>

// The Uno and the IRremote library use 32-bit raw IR values.
using IrRawCode = uint32_t;

struct IrEvent {
    uint16_t address;
    uint16_t command;
    IrRawCode rawCode;
    bool isRepeat;
};

void irBegin();
bool irReadEvent(IrEvent &event);

#endif
