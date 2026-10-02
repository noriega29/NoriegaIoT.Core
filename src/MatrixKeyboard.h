#ifndef MATRIX_KEYBOARD_H
#define MATRIX_KEYBOARD_H

#include <Arduino.h>

class MatrixKeyboard {

private:

    uint8_t address;

    // Mapa de teclas
    const char* keymap[2][3];

    // -----------------------------------------
    // ESTADOS INTERNOS PARA DEBOUNCE
    // -----------------------------------------

    bool lastReading[2][3];
    bool stableState[2][3];

    unsigned long lastDebounceTime[2][3];

    const unsigned long debounceDelay = 50;

    // -----------------------------------------
    // ÚLTIMO EVENTO DETECTADO
    // -----------------------------------------

    const char* lastPressedKey;
    const char* lastReleasedKey;

    // -----------------------------------------
    // MÁSCARAS DEL PCF8574
    // -----------------------------------------

    const uint8_t COL_MASKS[3] = {
        (1 << 0),
        (1 << 1),
        (1 << 2)
    };

    const uint8_t ROW_MASKS[2] = {
        (1 << 3),
        (1 << 4)
    };

    // -----------------------------------------
    // COMUNICACIÓN CON PCF8574
    // -----------------------------------------

    void writePCF(uint8_t value);
    uint8_t readPCF();

public:

    MatrixKeyboard(
        uint8_t i2cAddr,
        const char* userKeymap[2][3]
    );

    void begin();
    void update();

    // -----------------------------------------
    // EVENTOS
    // -----------------------------------------

    bool getKey(const char* name);
    bool getReleasedKey(const char* name);

    // -----------------------------------------
    // ESTADO ACTUAL
    // -----------------------------------------

    bool isPressed(const char* key);
};

#endif
