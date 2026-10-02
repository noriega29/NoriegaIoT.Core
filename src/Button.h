#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button {
private:
    int pin;

    bool lastState;
    bool currentState;

    unsigned long lastDebounceTime;

    const unsigned long debounceDelay = 50;

public:
    Button(int pin);

    void begin();

    bool pressed();
};

#endif
