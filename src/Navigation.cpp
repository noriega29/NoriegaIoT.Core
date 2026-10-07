#include "Navigation.h"

Navigation::Navigation():
    profundidadPila(0),
    estadoActual(MENU_PRINCIPAL),
    opcionSeleccionada(0) {
}

Estado Navigation::getEstado() const {
    return estadoActual;
}

int Navigation::getOpcionSeleccionada() const {
    return opcionSeleccionada;
}

void Navigation::setOpcionSeleccionada(int opcion) {
    opcionSeleccionada = opcion;
}

bool Navigation::entrarEstado(Estado nuevoEstado) {

    if (profundidadPila >= MAX_PROFUNDIDAD) {
        return false;
    }

    pila[profundidadPila].estado = estadoActual;
    pila[profundidadPila].opcionSeleccionada = opcionSeleccionada;

    profundidadPila++;

    estadoActual = nuevoEstado;
    opcionSeleccionada = 0;

    return true;
}

bool Navigation::regresar() {

    if (profundidadPila <= 0) {
        return false;
    }

    profundidadPila--;

    estadoActual = pila[profundidadPila].estado;
    opcionSeleccionada = pila[profundidadPila].opcionSeleccionada;

    return true;
}

void Navigation::reiniciar() {
    profundidadPila = 0;
    estadoActual = MENU_PRINCIPAL;
    opcionSeleccionada = 0;
}
