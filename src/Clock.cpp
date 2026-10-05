#include "Clock.h"

// Constructor de la clase Clock
Clock::Clock(RTC_DS3231& rtc) : rtc(rtc),
    formatoHora(FORMATO_24_HORAS),
    segundoAnterior(-1),
    minutoAnterior(-1),
    horaAnterior(-1),
    diaAnterior(-1),
    segundoCambiado(false),
    minutoCambiado(false),
    horaCambiada(false),
    diaCambiado(false) {
}

bool Clock::begin() {
    if (!rtc.begin()) {
        return false; // Error al inicializar el RTC
    }

    fechaActual = rtc.now();

    return true; // RTC inicializado correctamente
}

// Actualiza el estado del reloj y verifica si ha habido cambios en la hora, minuto, segundo o día
void Clock::update() {

    fechaActual = rtc.now();

    segundoCambiado = (fechaActual.second() != segundoAnterior);
    minutoCambiado = (fechaActual.minute() != minutoAnterior);
    horaCambiada = (fechaActual.hour() != horaAnterior);
    diaCambiado = (fechaActual.day() != diaAnterior);

    segundoAnterior = fechaActual.second();
    minutoAnterior = fechaActual.minute();
    horaAnterior = fechaActual.hour();
    diaAnterior = fechaActual.day();
}

DateTime Clock::now() const {
    return fechaActual;
}

void Clock::setDateTime(const DateTime& fecha) {
    rtc.adjust(fecha);
    fechaActual = fecha;
}

void Clock::setFormato(FormatoHora formato) {
    formatoHora = formato;
}

FormatoHora Clock::getFormato() const {
    return formatoHora;
}

bool Clock::secondChanged() const {
    return segundoCambiado;
}

bool Clock::minuteChanged() const {
    return minutoCambiado;
}

bool Clock::hourChanged() const {
    return horaCambiada;
}

bool Clock::dayChanged() const {
    return diaCambiado;
}

int Clock::getHour12(const DateTime& fecha) const {
    
    int hour = fecha.hour() % 12; // Convert to 12-hour format

    if (hour == 0) {
        return 12; // Midnight
    }

    return hour; // Return the hour as is for 1-12
}

int Clock::getHour24(const DateTime& fecha) const {
    return fecha.hour(); // Return the hour in 24-hour format
}

bool Clock::isAM(const DateTime& fecha) const {
    return fecha.hour() < 12; // Returns true if the hour is less than 12 (AM)
}

bool Clock::isPM(const DateTime& fecha) const {
    return fecha.hour() >= 12; // Returns true if the hour is 12 or greater (PM)
}
