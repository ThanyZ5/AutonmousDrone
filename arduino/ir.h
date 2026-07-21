#ifndef IR_H
#define IR_H

#include <Arduino.h>

struct IrEvent {
    uint16_t address;
    uint16_t command;
    bool isRepeat;
};

void irBegin();
bool irReadEvent(IrEvent &event);

#endif
