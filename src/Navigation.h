#ifndef NAVIGATION_H
#define NAVIGATION_H

enum Estado {
    MENU_PRINCIPAL,

    PANTALLA_HORA_FECHA,
    PANTALLA_TEMPERATURA,
    PANTALLA_HUMEDAD,
    PANTALLA_PRESION,

    MENU_CONFIGURACION,

    MENU_UNIDADES,
    MENU_INTERVALO,
    MENU_HORA_FECHA,
    MENU_FORMATO_HORA,
    MENU_CALIBRACION,

    PANTALLA_EDITAR_HORA,
    PANTALLA_EDITAR_FECHA
};

class Navigation {

private:

    struct Entrada {
        Estado estado;
        int opcionSeleccionada;
    };

    static const int MAX_PROFUNDIDAD = 10;

    Entrada pila[MAX_PROFUNDIDAD];
    int profundidadPila;

    Estado estadoActual;
    int opcionSeleccionada;

public:

    Navigation();

    Estado getEstado() const;
    int getOpcionSeleccionada() const;

    void setOpcionSeleccionada(int opcion);

    bool entrarEstado(Estado nuevoEstado);
    bool regresar();
    void reiniciar();
};

#endif
