#include "led.h"

#include <Arduino.h>

#include "config.h"

void ledBegin()
{
    pinMode(config::GREEN_LED_PIN, OUTPUT);
    ledSetGreen(false);
}

void ledSetGreen(bool on)
{
    digitalWrite(config::GREEN_LED_PIN, on ? HIGH : LOW);
}
