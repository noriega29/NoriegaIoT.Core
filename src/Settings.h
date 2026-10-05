#ifndef SETTINGS_H
#define SETTINGS_H

#include <Preferences.h>

enum FormatoHora {
    FORMATO_12_HORAS,
    FORMATO_24_HORAS
};

enum UnidadTemperatura {
    CELSIUS,
    FAHRENHEIT
};

class Settings {

    private:
        Preferences preferences;

        FormatoHora formatoHora;
        UnidadTemperatura unidadTemperatura;
        int intervaloMedicion; // Intervalo de medición en minutos

    public:
        Settings();

        bool begin();

        FormatoHora getFormatoHora() const;
        void setFormatoHora(FormatoHora formato);

        UnidadTemperatura getUnidadTemperatura() const;
        void setUnidadTemperatura(UnidadTemperatura unidad);

        int getIntervaloMedicion() const;
        void setIntervaloMedicion(int intervalo);
};

#endif
