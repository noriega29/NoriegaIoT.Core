#include <Button.h>
#include <MatrixKeyboard.h>

#include <Preferences.h>
#include <LCDI2C_Multilingual.h>
#include <Wire.h>
#include <RTClib.h>
#include "Button.h"
#include "MatrixKeyboard.h"

// ─────────────────────────────
// OBJETOS
// ─────────────────────────────
Preferences preferences;
LCDI2C_Latin_Symbols lcd(0x27, 16, 2);
RTC_DS3231 rtc;

// ─────────────────────────────
// TECLADO MATRIZ
// ─────────────────────────────

// Definimos el mapa de caracteres según la disposición de tu circuito:
// Fila 1 tiene físicamente los botones: S1, S3, S5
// Fila 2 tiene físicamente los botones: S2, S4, S6
const char* keys[2][3] = {
    {"Up", "Left", "Ok"}, 
    {"Down", "Right", "Back"}  
};

// Inicializamos el objeto pasándole la dirección y el mapa
MatrixKeyboard keyboard(0x20, keys);

// ─────────────────────────────
// ESTADOS DEl PROGRAMA
// ─────────────────────────────
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
  MENU_CALIBRACION
};

// Estado actual del programa
Estado estadoActual = MENU_PRINCIPAL;

// ─────────────────────────────
// ESTRUCTURA DE NAVEGACIÓN
// ─────────────────────────────
struct Navegacion {
  Estado estado;
  int opcionSeleccionada;
};

const int MAX_PROFUNDIDAD = 10; // Tamaño máximo del historial de navegación
Navegacion pila[MAX_PROFUNDIDAD];

int posicionPila = 0; // Posición actual en la pila de navegación

// ─────────────────────────────
// ESTRUCTURA DE OPCIONES DEL MENÚ
// ─────────────────────────────
struct OpcionMenu {

  const char * nombre;
  Estado destino;
};

// ─────────────────────────────
// OPCIONES DEL MENÚ PRINCIPAL
// ─────────────────────────────
OpcionMenu menuPrincipal[] = {
  
  {"Hora/Fecha", PANTALLA_HORA_FECHA},
  {"Temperatura", PANTALLA_TEMPERATURA},
  {"Humedad", PANTALLA_HUMEDAD},
  {"Presion", PANTALLA_PRESION},
  {"Configuracion", MENU_CONFIGURACION}
};

const int NUM_OPCIONES_PRINCIPAL = sizeof(menuPrincipal) / sizeof(menuPrincipal[0]);

// ─────────────────────────────
// MENÚ DE CONFIGURACIÓN
// ─────────────────────────────
OpcionMenu menuConfiguracion[] = {

  {"Unidades", MENU_UNIDADES},
  {"Intervalo", MENU_INTERVALO},
  {"Hora/Fecha", MENU_HORA_FECHA},
  {"Calibracion", MENU_CALIBRACION}
};

const int NUM_OPCIONES_CONFIGURACION = sizeof(menuConfiguracion) / sizeof(menuConfiguracion[0]);

OpcionMenu menuHoraFecha[] = {

  {"Hora", PANTALLA_HORA_FECHA},
  {"Fecha", PANTALLA_HORA_FECHA},
  {"Formato", MENU_FORMATO_HORA}
};

const int NUM_OPCIONES_HORA_FECHA = sizeof(menuHoraFecha) / sizeof(menuHoraFecha[0]);

// ─────────────────────────────
// OPCIÓN SELECCIONADA
// ─────────────────────────────
// Índice de la opción actualmente seleccionada en el menú
int opcionSeleccionada = 0;

// ─────────────────────────────
// CONFIGURACIÓN DEL DISPOSITIVO
// ─────────────────────────────

// Formato de hora y fecha
enum FormatoHora {
  FORMATO_12_HORAS,
  FORMATO_24_HORAS
};

FormatoHora formatoHora = FORMATO_24_HORAS; // Formato de hora actual
FormatoHora formatoHoraTemporal = FORMATO_24_HORAS; // Formato de hora temporal para la selección en el menú

// Intervalo de medición en minutos
enum UnidadTemperatura {
  CELSIUS,
  FAHRENHEIT
};

UnidadTemperatura unidadTemperatura = CELSIUS; // Unidad de temperatura actual
UnidadTemperatura unidadTemperaturaTemporal = CELSIUS; // Unidad de temperatura temporal para la selección en el menú

// Intervalo de medición en minutos
const int intervalos[] = {
  1, 5, 10, 15, 30, 60
};

const int NUM_INTERVALOS = sizeof(intervalos) / sizeof(intervalos[0]);

int intervaloMedicion = 1; // Intervalo de medición actual (en minutos)
int intervaloMedicionTemporal = 1; // Intervalo de medición temporal (en minutos)

unsigned long ultimaActualizacionHora = 0;
const unsigned long INTERVALO_ACTUALIZACION_HORA = 1000;

// ─────────────────────────────
// SETUP
// ─────────────────────────────
void setup() {
  // Inicialización de la comunicación serial para depuración
  Serial.begin(115200);

  // Inicialización de preferencias para guardar configuraciones
  preferences.begin("config", false);

  formatoHora = static_cast<FormatoHora>(
    preferences.getUChar("formatoHora", FORMATO_24_HORAS)
  );

  formatoHoraTemporal = formatoHora;

  // Pines SDA y SCL para tu ESP32 / Arduino
  Wire.begin();

  // LCD
  lcd.init();
  lcd.backlight();

  // RTC
  if (!rtc.begin()) {

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Error RTC");

    while (1);
  }

  // Botones
  keyboard.begin();

  // Mostrar pantalla inicial
  mostrarEstado();
}

// ─────────────────────────────
// LOOP
// ─────────────────────────────
void loop() {
  
  // 1. Escanear teclado y actualizar estados
  keyboard.update();

  // 2. Ejecutar lógica del estado actual
  switch (estadoActual) {

    case MENU_PRINCIPAL:
      manejarMenu(menuPrincipal, NUM_OPCIONES_PRINCIPAL);
      break;

    case MENU_CONFIGURACION:
      manejarMenu(menuConfiguracion, NUM_OPCIONES_CONFIGURACION);
      break;

    case MENU_HORA_FECHA:
      manejarMenuFechaHora();
      break;

    case MENU_FORMATO_HORA:
      manejarFormatoHora();
      break;

    case MENU_UNIDADES:
      manejarPantallaUnidades();
      break;

    case MENU_INTERVALO:
      manejarPantallaIntervalo();
      break;
    
    case MENU_CALIBRACION:
      manejarPantallaCalibracion();
      break;

    case PANTALLA_TEMPERATURA:
      manejarPantallaTemperatura();
      break;

    case PANTALLA_HUMEDAD:
      manejarPantallaHumedad();
      break;

    case PANTALLA_PRESION:
      manejarPantallaPresion();
      break;
    
    case PANTALLA_HORA_FECHA:
      manejarPantallaHoraFecha();
      break;
  }
}

// ─────────────────────────────
// NAVEGACIÓN EN CUALQUIER MENÚ
// ─────────────────────────────

void manejarMenu(OpcionMenu menu[], int numOpciones) {

  if (keyboard.getReleasedKey("Up")) {

        Serial.println("EVENTO: Up");

        subir(numOpciones);
    }

    if (keyboard.getReleasedKey("Down")) {

        Serial.println("EVENTO: Down");

        bajar(numOpciones);
    }

    if (keyboard.getReleasedKey("Left")) {

        Serial.println("EVENTO: Left");

        // Acción para la tecla "Left" si es necesario
    }

    if (keyboard.getReleasedKey("Right")) {

        Serial.println("EVENTO: Right");

        // Acción para la tecla "Right" si es necesario
    }

    if (keyboard.getReleasedKey("Ok")) {

        Serial.println("EVENTO: Ok");

        seleccionar(menu);
    }

    if (keyboard.getReleasedKey("Back")) {

        Serial.println("EVENTO: Back");

        regresar();
    }
}

// ─────────────────────────────
// NAVEGACIÓN DEL MENÚ
// SUBIR
// ─────────────────────────────
void subir(int numOpciones) {

  if (opcionSeleccionada > 0) {

    opcionSeleccionada--;
    mostrarMenuActual();
  }
}

// ─────────────────────────────
// NAVEGACIÓN DEL MENÚ
// BAJAR
// ─────────────────────────────
void bajar(int numOpciones) {

  if (opcionSeleccionada < numOpciones - 1) {

    opcionSeleccionada++;
    mostrarMenuActual();
  }
}

// ─────────────────────────────
// NAVEGACIÓN DEL MENÚ
// SELECCIONAR
// ─────────────────────────────
void seleccionar(OpcionMenu menu[]) {

  // Obtener el estado al que conduce la opción seleccionada
  Estado destino = menu[opcionSeleccionada].destino;

  // Entrar al nuevo estado
  entrarEstado(destino);
}

// ─────────────────────────────
// NAVEGACIÓN DEL MENÚ
// REGRESAR
// ─────────────────────────────
void regresar() {

  // Regresar al estado anterior utilizando la pila de navegación
  if (posicionPila > 0) {

    // Retroceder en la pila de navegación
    posicionPila--;

    // Restaurar el estado y la opción seleccionada desde la pila
    estadoActual = pila[posicionPila].estado;
    opcionSeleccionada = pila[posicionPila].opcionSeleccionada;

    mostrarEstado();
  }
}

// ─────────────────────────────
// MOSTRAR ESTADO ACTUAL
// ─────────────────────────────
void mostrarEstado() {

  switch (estadoActual) {

    // Menu principal
    case MENU_PRINCIPAL:

      mostrarMenuActual();
      break;

    // Pantalla de hora y fecha
    case PANTALLA_HORA_FECHA:

      mostrarHoraFecha();
      break;

    // Temperatura
    case PANTALLA_TEMPERATURA:

      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print("Temperatura");

      lcd.setCursor(0, 1);
      lcd.print("25.4 C"); // Aquí deberías mostrar la temperatura real
      
      break;

    // Humedad
    case PANTALLA_HUMEDAD:

      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print("Humedad");

      lcd.setCursor(0, 1);
      lcd.print("65 %"); // Aquí deberías mostrar la humedad real
      
      break;

    // Presión
    case PANTALLA_PRESION:

      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print("Presion");

      lcd.setCursor(0, 1);
      lcd.print("1013 hPa"); // Aquí deberías mostrar la presión real
      
      break;

    case MENU_CONFIGURACION:

      mostrarMenuActual();
      break;

    case MENU_HORA_FECHA:

      mostrarMenuActual();
      break;

    // Menú de unidades
    case MENU_UNIDADES:

      mostrarUnidadTemperatura();
      break;

    // Intervalo
    case MENU_INTERVALO:

      mostrarIntervaloMedicion();
      break;

    // Calibración
    case MENU_CALIBRACION:
      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print("Calibracion");

      lcd.setCursor(0, 1);
      lcd.print("Proximamente..."); // Aquí deberías mostrar la calibración actual
      
      break;

    // Formato de hora
    case MENU_FORMATO_HORA:
      mostrarFormatoHora();
      break;
  }
}

// ─────────────────────────────
// MOSTRAR UNIDAD DE TEMPERATURA
// ─────────────────────────────
void mostrarUnidadTemperatura() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Unidad Temp:");

  lcd.setCursor(0, 1);

  if (unidadTemperaturaTemporal == CELSIUS) {
    lcd.print("> Celsius");
  }
  else {
    lcd.print("> Fahrenheit");
  }
}

// ─────────────────────────────
// MOSTRAR INTERVALO DE MEDICIÓN
// ─────────────────────────────
void mostrarIntervaloMedicion() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Intervalo:");

  lcd.setCursor(0, 1);
  lcd.print("> ");
  lcd.print(intervaloMedicionTemporal);
  lcd.print(" min");
}

// ─────────────────────────────
// MOSTRAR HORA Y FECHA
// ─────────────────────────────
void mostrarHoraFecha() {
  DateTime ahora = rtc.now();

  lcd.clear();

  // -------------------------
  // HORA
  // -------------------------

  lcd.setCursor(8, 0);

  if (formatoHora == FORMATO_24_HORAS) {

    // 24 horas → HH:MM:SS

    if (ahora.hour() < 10) lcd.print("0");
    lcd.print(ahora.hour());

    lcd.print(":");

    if (ahora.minute() < 10) lcd.print("0");
    lcd.print(ahora.minute());

    lcd.print(":");

    if (ahora.second() < 10) lcd.print("0");
    lcd.print(ahora.second());

  } else {

    // 12 horas → HH:MM AM/PM

    int hora12 = ahora.hour() % 12;

    if (hora12 == 0) {
      hora12 = 12;
    }

    if (hora12 < 10) lcd.print("0");
    lcd.print(hora12);

    lcd.print(":");

    if (ahora.minute() < 10) lcd.print("0");
    lcd.print(ahora.minute());

    if (ahora.hour() < 12) {
      lcd.print(" am");
    } else {
      lcd.print(" pm");
    }
  }

  // -------------------------
  // FECHA
  // -------------------------

  lcd.setCursor(6, 1);

  if (ahora.day() < 10) lcd.print("0");
  lcd.print(ahora.day());

  lcd.print("/");

  if (ahora.month() < 10) lcd.print("0");
  lcd.print(ahora.month());

  lcd.print("/");

  lcd.print(ahora.year());
}

void mostrarFormatoHora() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Formato Hora:");

  lcd.setCursor(0, 1);

  if (formatoHoraTemporal == FORMATO_24_HORAS) {
    lcd.print("> 24 Horas");
  }
  else {
    lcd.print("> 12 Horas");
  }
}

// ─────────────────────────────
// MOSTRAR MENÚ ACTUAL
// ─────────────────────────────

void mostrarMenuActual() {

  OpcionMenu * menuActual;
  int cantidadOpciones;

  // Determinar qué menú mostrar según el estado actual
  if (estadoActual == MENU_PRINCIPAL) {

    menuActual = menuPrincipal;
    cantidadOpciones = NUM_OPCIONES_PRINCIPAL;
  }
  else if (estadoActual == MENU_CONFIGURACION) {

    menuActual = menuConfiguracion;
    cantidadOpciones = NUM_OPCIONES_CONFIGURACION;
  }
  else if (estadoActual == MENU_HORA_FECHA) {

    menuActual = menuHoraFecha;
    cantidadOpciones = NUM_OPCIONES_HORA_FECHA;
  }

  // Limpiar la pantalla antes de mostrar el menú
  lcd.clear();

  // Determinar la primera opción a mostrar en la pantalla
  int primeraOpcion;

  if (opcionSeleccionada < 2) {

    primeraOpcion = 0;

  } else {

    primeraOpcion = opcionSeleccionada - 1;
  }

  // Dibujar las opciones en la pantalla
  for (int fila = 0; fila < 2; fila++) {

    int opcionActual = primeraOpcion + fila;

    if (opcionActual < cantidadOpciones) {

      lcd.setCursor(0, fila);

      // Mostrar un indicador ">" si la opción actual es la seleccionada
      if (opcionActual == opcionSeleccionada) {

        lcd.print(">");
      } else {

        lcd.print(" ");
      }

      // Mostrar el nombre de la opción
      lcd.print(menuActual[opcionActual].nombre);
    }
  }
}

// ─────────────────────────────
// PANTALLAS DEL MENÚ
// ─────────────────────────────

void manejarPantallaHoraFecha() {

  // Actualiza la hora cada segundo
  if (millis() - ultimaActualizacionHora >= INTERVALO_ACTUALIZACION_HORA) {

    ultimaActualizacionHora = millis();
    mostrarHoraFecha();
  }

  // Regresar al menú anterior
  if (keyboard.getReleasedKey("Back")) {
    regresar();
  }
}

void manejarPantallaTemperatura() {
  
  if (keyboard.getReleasedKey("Back")) {
    
    regresar();
  }
}

void manejarPantallaHumedad() {
  
  if (keyboard.getReleasedKey("Back")) {
    
    regresar();
  }
}

void manejarPantallaPresion() {
  
  if (keyboard.getReleasedKey("Back")) {
    
    regresar();
  }
}

void manejarPantallaUnidades() {
  
  if (keyboard.getReleasedKey("Up") || keyboard.getReleasedKey("Down")) {
    
    if (unidadTemperaturaTemporal == CELSIUS) {
      unidadTemperaturaTemporal = FAHRENHEIT;
    } else {
      unidadTemperaturaTemporal = CELSIUS;
    }

    mostrarUnidadTemperatura();
  }

  // Confirmar la selección de unidad de temperatura
  if (keyboard.getReleasedKey("Ok")) {
    
    unidadTemperatura = unidadTemperaturaTemporal;
    regresar();
  }

  // Cancelar la selección y regresar al estado anterior
  if (keyboard.getReleasedKey("Back")) {
    
    regresar();
  }
}

void manejarPantallaIntervalo() {
  
  // Cambiar al intervalo de medición anterior
  if (keyboard.getReleasedKey("Up")) {
    
    for (int i = 0; i < NUM_INTERVALOS; i++) {

      if (intervalos[i] == intervaloMedicionTemporal) {

        if (i > 0) {
          intervaloMedicionTemporal = intervalos[i - 1];
        }

        // Salir del bucle una vez que se ha encontrado y actualizado el intervalo
        break;
      }
    }

    mostrarIntervaloMedicion();
  }

  // Cambiar al siguiente intervalo de medición
  if (keyboard.getReleasedKey("Down")) {
    
    for (int i = 0; i < NUM_INTERVALOS; i++) {

      if (intervalos[i] == intervaloMedicionTemporal) {

        if (i < NUM_INTERVALOS - 1) {
          intervaloMedicionTemporal = intervalos[i + 1];
        }

        // Salir del bucle una vez que se ha encontrado y actualizado el intervalo
        break;
      }
    }

    mostrarIntervaloMedicion();
  }

  // Confirmar la selección de intervalo de medición
  if (keyboard.getReleasedKey("Ok")) {
    
    intervaloMedicion = intervaloMedicionTemporal;
    regresar();
  }

  // Cancelar la selección y regresar al estado anterior
  if (keyboard.getReleasedKey("Back")) {
    
    regresar();
  }
}

void manejarMenuFechaHora() {

  if (keyboard.getReleasedKey("Up")) {
    subir(NUM_OPCIONES_HORA_FECHA);
  }

  if (keyboard.getReleasedKey("Down")) {
    bajar(NUM_OPCIONES_HORA_FECHA);
  }

  if (keyboard.getReleasedKey("Ok")) {
    seleccionar(menuHoraFecha);
  }

  if (keyboard.getReleasedKey("Back")) {
    regresar();
  }
}

void manejarFormatoHora() {
  
  if (keyboard.getReleasedKey("Left") || keyboard.getReleasedKey("Right")) {
    
    if (formatoHoraTemporal == FORMATO_24_HORAS) {
      formatoHoraTemporal = FORMATO_12_HORAS;
    } else {
      formatoHoraTemporal = FORMATO_24_HORAS;
    }

    mostrarFormatoHora();
  }

  // Confirmar la selección de formato de hora
  if (keyboard.getReleasedKey("Ok")) {
    
    formatoHora = formatoHoraTemporal;

    preferences.putUChar(
      "formatoHora",
      static_cast<uint8_t>(formatoHora)
    );

    regresar();
  }

  // Cancelar la selección y regresar al estado anterior
  if (keyboard.getReleasedKey("Back")) {
    
    regresar();
  }
}

void manejarPantallaCalibracion() {
  
  if (keyboard.getReleasedKey("Back")) {
    
    regresar();
  }
}

void entrarEstado(Estado nuevoEstado) {
  
  // Guardar el estado actual y la opción seleccionada en la pila
  if (posicionPila < MAX_PROFUNDIDAD) {
    
    pila[posicionPila].estado = estadoActual;
    pila[posicionPila].opcionSeleccionada = opcionSeleccionada;

    posicionPila++;
  }

  // Cambiar al nuevo estado
  estadoActual = nuevoEstado;

  // Reiniciar la opción seleccionada al entrar a un nuevo estado
  opcionSeleccionada = 0;

  // Guardar la unidad actual antes de cambiar
  if (nuevoEstado == MENU_UNIDADES) {
    unidadTemperaturaTemporal = unidadTemperatura;
  }

  // Guardar el intervalo de medición actual antes de cambiar
  if (nuevoEstado == MENU_INTERVALO) {
    intervaloMedicionTemporal = intervaloMedicion;
  }

  // Guardar el formato de hora actual antes de cambiar
  if (nuevoEstado == MENU_FORMATO_HORA) {
    formatoHoraTemporal = formatoHora;
  }

  // Mostrar el nuevo estado en la pantalla
  mostrarEstado();
}
