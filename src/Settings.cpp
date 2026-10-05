#include "Settings.h"

Settings::Settings()
    : formatoHora(FORMATO_24_HORAS),
      unidadTemperatura(CELSIUS),
      intervaloMedicion(1) {
}

bool Settings::begin() {

    if (!preferences.begin("config", false)) {
        return false; // Error al inicializar Preferences
    }
    
    // Leer el formato de hora guardado en Preferences, si no existe, se usa el valor por defecto
    formatoHora = static_cast<FormatoHora>(
        preferences.getUChar(
            "formatoHora",
            FORMATO_24_HORAS
        )
    );
    
    // Leer la unidad de temperatura guardada en Preferences, si no existe, se usa el valor por defecto
    unidadTemperatura = static_cast<UnidadTemperatura>(
        preferences.getUChar(
            "unidadTemp",
            CELSIUS
        )
    );

    // Leer el intervalo de medición guardado en Preferences, si no existe, se usa el valor por defecto
    intervaloMedicion = preferences.getInt(
        "intervalo",
        1 // Valor por defecto
    );

    return true; // Inicialización exitosa
}

// Getter y setter para FormatoHora
FormatoHora Settings::getFormatoHora() const {
    return formatoHora;
}

void Settings::setFormatoHora(FormatoHora formato) {
    formatoHora = formato;
    preferences.putUChar(
        "formatoHora",
        static_cast<uint8_t>(formatoHora)
    );
}

// Getter y setter para UnidadTemperatura
UnidadTemperatura Settings::getUnidadTemperatura() const {
    return unidadTemperatura;
}

void Settings::setUnidadTemperatura(UnidadTemperatura unidad) {
    unidadTemperatura = unidad;
    preferences.putUChar(
        "unidadTemp",
        static_cast<uint8_t>(unidadTemperatura)
    );
}

// Getter y setter para IntervaloMedicion
int Settings::getIntervaloMedicion() const {
    return intervaloMedicion;
}

void Settings::setIntervaloMedicion(int intervalo) {
    intervaloMedicion = intervalo;
    preferences.putInt(
        "intervalo",
        intervaloMedicion
    );
}