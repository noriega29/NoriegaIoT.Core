#include "Button.h"

Button::Button(int pin) {
    this -> pin = pin;

    lastState = LOW;
    currentState = LOW;

    lastDebounceTime = 0;
}

void Button::begin() {
    pinMode(pin, INPUT);
}

bool Button::pressed() {
    bool reading = digitalRead(pin);

    if (reading != lastState) {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading != currentState) {
            currentState = reading;

            if (currentState == HIGH) {
                lastState = reading;
                return true;
            }
        }
    }

    lastState = reading;
    return false;
}
