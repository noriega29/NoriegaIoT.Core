#ifndef CLOCK_H
#define CLOCK_H

#include <Arduino.h>
#include <RTClib.h>
#include "Settings.h"

class Clock {

    private:
        RTC_DS3231& rtc;

        FormatoHora formatoHora;

        DateTime fechaActual;

        int segundoAnterior;
        int minutoAnterior;
        int horaAnterior;
        int diaAnterior;

        bool segundoCambiado;
        bool minutoCambiado;
        bool horaCambiada;
        bool diaCambiado;

    public:
        Clock(RTC_DS3231& rtc);

        bool begin();

        void update();

        DateTime now() const;
        void setDateTime(const DateTime& fecha);

        void setFormato(FormatoHora formato);
        FormatoHora getFormato() const;

        bool secondChanged() const;
        bool minuteChanged() const;
        bool hourChanged() const;
        bool dayChanged() const;

        int getHour12(const DateTime& fecha) const;
        int getHour24(const DateTime& fecha) const;
        bool isAM(const DateTime& fecha) const;
        bool isPM(const DateTime& fecha) const;
};

#endif
