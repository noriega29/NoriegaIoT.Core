#include "MatrixKeyboard.h"
#include <Wire.h>

// ─────────────────────────────────────────────
// CONSTRUCTOR
// ─────────────────────────────────────────────

MatrixKeyboard::MatrixKeyboard(
    uint8_t i2cAddr,
    const char* userKeymap[2][3]
) {
    address = i2cAddr;

    lastPressedKey = "";
    lastReleasedKey = "";

    for (int r = 0; r < 2; r++) {

        for (int c = 0; c < 3; c++) {

            keymap[r][c] = userKeymap[r][c];

            lastReading[r][c] = false;
            stableState[r][c] = false;

            lastDebounceTime[r][c] = 0;
        }
    }
}


// ─────────────────────────────────────────────
// COMUNICACIÓN I2C
// ─────────────────────────────────────────────

void MatrixKeyboard::writePCF(uint8_t value) {

    Wire.beginTransmission(address);

    Wire.write(value);

    Wire.endTransmission();
}


uint8_t MatrixKeyboard::readPCF() {

    Wire.requestFrom(address, (uint8_t)1);

    if (Wire.available()) {
        return Wire.read();
    }

    return 0xFF;
}


// ─────────────────────────────────────────────
// INICIALIZACIÓN
// ─────────────────────────────────────────────

void MatrixKeyboard::begin() {

    writePCF(0xFF);
}


// ─────────────────────────────────────────────
// BARRIDO + DEBOUNCE
// ─────────────────────────────────────────────

void MatrixKeyboard::update() {

    for (int c = 0; c < 3; c++) {

        // Activar columna
        writePCF(0xFF & ~COL_MASKS[c]);

        delayMicroseconds(20);

        uint8_t estado = readPCF();

        for (int r = 0; r < 2; r++) {

            // LOW = tecla presionada
            bool currentReading =
                !(estado & ROW_MASKS[r]);


            // ─────────────────────────────────
            // DEBOUNCE
            // ─────────────────────────────────

            if (currentReading != lastReading[r][c]) {

                lastDebounceTime[r][c] = millis();

                lastReading[r][c] = currentReading;
            }


            // ─────────────────────────────────
            // ESTADO ESTABLE
            // ─────────────────────────────────

            if ((millis() - lastDebounceTime[r][c])
                >= debounceDelay) {

                if (currentReading != stableState[r][c]) {

                    stableState[r][c] = currentReading;


                    // ─────────────────────────
                    // TECLA PRESIONADA
                    // ─────────────────────────

                    if (stableState[r][c]) {

                        lastPressedKey =
                            keymap[r][c];
                    }


                    // ─────────────────────────
                    // TECLA LIBERADA
                    // ─────────────────────────

                    else {

                        lastReleasedKey =
                            keymap[r][c];
                    }
                }
            }
        }
    }

    // Liberar todas las líneas
    writePCF(0xFF);
}


// ─────────────────────────────────────────────
// EVENTO: TECLA PRESIONADA
// ─────────────────────────────────────────────

bool MatrixKeyboard::getKey(const char* name) {

    // No hay evento pendiente
    if (lastPressedKey == nullptr ||
        lastPressedKey[0] == '\0') {

        return false;
    }


    // ¿Es la tecla que estamos buscando?
    if (strcmp(lastPressedKey, name) == 0) {

        // Consumir evento
        lastPressedKey = "";

        return true;
    }


    return false;
}


// ─────────────────────────────────────────────
// EVENTO: TECLA LIBERADA
// ─────────────────────────────────────────────

bool MatrixKeyboard::getReleasedKey(const char* name) {

    // No hay evento pendiente
    if (lastReleasedKey == nullptr ||
        lastReleasedKey[0] == '\0') {

        return false;
    }

    // ¿Es la tecla que estamos buscando?
    if (strcmp(lastReleasedKey, name) == 0) {

        // Consumir evento
        lastReleasedKey = "";

        return true;
    }

    return false;
}

// ─────────────────────────────────────────────
// ESTADO ACTUAL DE LA TECLA
// ─────────────────────────────────────────────

bool MatrixKeyboard::isPressed(const char* key) {

    for (int r = 0; r < 2; r++) {

        for (int c = 0; c < 3; c++) {

            if (strcmp(keymap[r][c], key) == 0) {

                return stableState[r][c];
            }
        }
    }

    return false;
}
