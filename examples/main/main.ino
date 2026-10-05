#include <LCDI2C_Multilingual.h>
#include <Wire.h>
#include <RTClib.h>

#include "MatrixKeyboard.h"
#include "Clock.h"
#include "Settings.h"

// ─────────────────────────────
// OBJETOS
// ─────────────────────────────
Settings settings;
LCDI2C_Latin_Symbols lcd(0x27, 16, 2);
RTC_DS3231 rtc;
Clock reloj(rtc);

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
  MENU_CALIBRACION,

  PANTALLA_EDITAR_HORA,
  PANTALLA_EDITAR_FECHA
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

  {"Hora", PANTALLA_EDITAR_HORA},
  {"Fecha", PANTALLA_EDITAR_FECHA},
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

enum CampoHora {
  HORA,
  MINUTO,
  SEGUNDO,
  AM_PM
};

enum CampoFecha {
  DIA,
  MES,
  ANIO
};

int horaTemporal = 0; // Hora temporal para la configuración
int minutoTemporal = 0; // Minuto temporal para la configuración
int segundoTemporal = 0; // Segundo temporal para la configuración
int diaTemporal = 1; // Día temporal para la configuración
int mesTemporal = 1; // Mes temporal para la configuración
int anioTemporal = 2000; // Año temporal para la configuración

CampoHora campoHoraSeleccionado = HORA;
CampoFecha campoFechaSeleccionado = DIA;

// Variable para alternar la visualización del campo seleccionado
bool mostrarCampoSeleccionado = true; 

unsigned long ultimoParpadeo = 0;
const unsigned long INTERVALO_PARPADEO = 850; // Intervalo de parpadeo en milisegundos

// Intervalo de medición en minutos
const int intervalos[] = {
  1, 5, 10, 15, 30, 60
};

const int NUM_INTERVALOS = sizeof(intervalos) / sizeof(intervalos[0]);

// Variables temporales para la configuración
int intervaloMedicionTemporal = 1; // Intervalo de medición temporal (en minutos)
FormatoHora formatoHoraTemporal = FORMATO_24_HORAS; // Formato de hora temporal para la selección en el menú
UnidadTemperatura unidadTemperaturaTemporal = CELSIUS; // Unidad de temperatura temporal para la selección en el menú


unsigned long ultimaActualizacionHora = 0;
const unsigned long INTERVALO_ACTUALIZACION_HORA = 1000;

// ─────────────────────────────
// SETUP
// ─────────────────────────────
void setup() {
  // Inicialización de la comunicación serial para depuración
  Serial.begin(115200);

  // Inicialización de preferencias para guardar configuraciones
  if (!settings.begin()) {
    Serial.println("Error al inicializar Settings");
    while (1);
  }

  formatoHoraTemporal = settings.getFormatoHora();
  reloj.setFormato(formatoHoraTemporal);

  unidadTemperaturaTemporal = settings.getUnidadTemperatura();

  // Pines SDA y SCL para tu ESP32 / Arduino
  Wire.begin();

  // LCD
  lcd.init();
  lcd.backlight();

  // RTC
  if (!reloj.begin()) {

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

    case PANTALLA_EDITAR_HORA:
      manejarHora();
      actualizarParpadeoHora();
      break;

    case PANTALLA_EDITAR_FECHA:
      manejarFecha();
      actualizarParpadeoFecha();
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

// Función para mostrar la hora y la fecha en el LCD
void mostrarHoraFecha() {

  lcd.clear();

  mostrarHora();
  mostrarFecha();
}

// Función para mostrar la hora en el LCD
void mostrarHora() {

  DateTime ahora = reloj.now();

  lcd.setCursor(8, 0);

  if (reloj.getFormato() == FORMATO_24_HORAS) {

    if (ahora.hour() < 10) lcd.print("0");
    lcd.print(ahora.hour());

    lcd.print(":");

    if (ahora.minute() < 10) lcd.print("0");
    lcd.print(ahora.minute());

    lcd.print(":");

    if (ahora.second() < 10) lcd.print("0");
    lcd.print(ahora.second());

  } else {

    int hora12 = reloj.getHour12(ahora);

    if (hora12 < 10) lcd.print("0");
    lcd.print(hora12);

    lcd.print(":");

    if (ahora.minute() < 10) lcd.print("0");
    lcd.print(ahora.minute());

    if (reloj.isAM(ahora)) {
      lcd.print(" am");
    } else {
      lcd.print(" pm");
    }
  }
}

// Función para mostrar la fecha en el LCD
void mostrarFecha() {

  DateTime ahora = reloj.now();

  lcd.setCursor(6, 1);

  if (ahora.day() < 10) lcd.print("0");
  lcd.print(ahora.day());

  lcd.print("/");

  if (ahora.month() < 10) lcd.print("0");
  lcd.print(ahora.month());

  lcd.print("/");

  lcd.print(ahora.year());
}

void actualizarHoraFecha() {

  reloj.update();

  DateTime ahora = reloj.now();

  if (reloj.hourChanged() || reloj.minuteChanged()) {
    mostrarHora();
  }

  if (reloj.getFormato() == FORMATO_24_HORAS &&
      reloj.secondChanged()) {

    lcd.setCursor(14, 0);

    if (ahora.second() < 10) lcd.print("0");

    lcd.print(ahora.second());
  }

  if (reloj.dayChanged()) {
    mostrarFecha();
  }
}

void mostrarHoraEdicion() {

  lcd.clear();

  lcd.setCursor(0, 0);

  // Hora
  if (reloj.getFormato() == FORMATO_24_HORAS) {

    lcd.print("HH:MM:SS");

    lcd.setCursor(8, 1);

    if (campoHoraSeleccionado == HORA && !mostrarCampoSeleccionado) {
      
      lcd.print("  ");

    } else {

      if (horaTemporal < 10) lcd.print("0");
      lcd.print(horaTemporal);
    }

  } else {

    lcd.print("HH:MM:SS am/pm");

    lcd.setCursor(5, 1);

    int hora12 = horaTemporal % 12;

    if (hora12 == 0) {
      hora12 = 12;
    }

    if (campoHoraSeleccionado == HORA && !mostrarCampoSeleccionado) {
      
      lcd.print("  ");

    } else {

      if (hora12 < 10) lcd.print("0");
      lcd.print(hora12);
    }
  }

  lcd.print(":");

  // Minutos
  if (campoHoraSeleccionado == MINUTO && !mostrarCampoSeleccionado) {
    lcd.print("  ");
  } else {
    if (minutoTemporal < 10) lcd.print("0");
    lcd.print(minutoTemporal);
  }

  lcd.print(":");

  // Segundos
  if (campoHoraSeleccionado == SEGUNDO && !mostrarCampoSeleccionado) {
    lcd.print("  ");
  } else {
    if (segundoTemporal < 10) {
      lcd.print("0");
    }

    lcd.print(segundoTemporal);
  }

  // AM / PM
  if (reloj.getFormato() == FORMATO_12_HORAS) {

    if (campoHoraSeleccionado == AM_PM && !mostrarCampoSeleccionado) {
      lcd.print("   ");
    } else {

      if (horaTemporal < 12) {
        lcd.print(" am");
      } else {
        lcd.print(" pm");
      }
    }
  }
}

void mostrarCampoHoraSeleccionado() {

  if (reloj.getFormato() == FORMATO_24_HORAS) {

    switch (campoHoraSeleccionado) {

      case HORA:
        lcd.setCursor(8, 1);

        if (!mostrarCampoSeleccionado) {
          lcd.print("  ");
        } else {
          if (horaTemporal < 10) lcd.print("0");
          lcd.print(horaTemporal);
        }
        break;

      case MINUTO:
        lcd.setCursor(11, 1);

        if (!mostrarCampoSeleccionado) {
          lcd.print("  ");
        } else {
          if (minutoTemporal < 10) lcd.print("0");
          lcd.print(minutoTemporal);
        }
        break;

      case SEGUNDO:
        lcd.setCursor(14, 1);

        if (!mostrarCampoSeleccionado) {
          lcd.print("  ");
        } else {
          if (segundoTemporal < 10) lcd.print("0");
          lcd.print(segundoTemporal);
        }
        break;

      default:
        break;
    }

  } else {

    switch (campoHoraSeleccionado) {

      case HORA: {
        lcd.setCursor(5, 1);

        if (!mostrarCampoSeleccionado) {
          lcd.print("  ");
        } else {

          int hora12 = horaTemporal % 12;

          if (hora12 == 0) {
            hora12 = 12;
          }

          if (hora12 < 10) lcd.print("0");
          lcd.print(hora12);
        }
        break;
      }

      case MINUTO:
        lcd.setCursor(8, 1);

        if (!mostrarCampoSeleccionado) {
          lcd.print("  ");
        } else {
          if (minutoTemporal < 10) lcd.print("0");
          lcd.print(minutoTemporal);
        }
        break;

      case SEGUNDO:
        lcd.setCursor(11, 1);

        if (!mostrarCampoSeleccionado) {
          lcd.print("  ");
        } else {
          if (segundoTemporal < 10) lcd.print("0");
          lcd.print(segundoTemporal);
        }
        break;

      case AM_PM:
        lcd.setCursor(13, 1);

        if (!mostrarCampoSeleccionado) {
          lcd.print("   ");
        } else {

          if (horaTemporal < 12) {
            lcd.print(" am");
          } else {
            lcd.print(" pm");
          }
        }
        break;
    }
  }
}

void actualizarParpadeoHora() {

  if (millis() - ultimoParpadeo >= INTERVALO_PARPADEO) {

    ultimoParpadeo = millis();
    mostrarCampoSeleccionado = !mostrarCampoSeleccionado;

    mostrarCampoHoraSeleccionado();
  }
}

// Función para mostrar la selección de los formatos de hora
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

void mostrarFechaEdicion() {

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Fecha           ");

  lcd.setCursor(6, 1);

  if (diaTemporal < 10) lcd.print("0");
  lcd.print(diaTemporal);

  lcd.print("/");

  if (mesTemporal < 10) lcd.print("0");
  lcd.print(mesTemporal);

  lcd.print("/");

  lcd.print(anioTemporal);
}

void mostrarCampoFechaSeleccionado() {

  if (!mostrarCampoSeleccionado) {
    return;
  }

  switch (campoFechaSeleccionado) {

    case DIA:
      lcd.setCursor(6, 1);

      if (diaTemporal < 10) {
        lcd.print("0");
      }

      lcd.print(diaTemporal);
      break;

    case MES:
      lcd.setCursor(9, 1);

      if (mesTemporal < 10) {
        lcd.print("0");
      }

      lcd.print(mesTemporal);
      break;

    case ANIO:
      lcd.setCursor(12, 1);
      lcd.print(anioTemporal);
      break;
  }
}

void ocultarCampoFechaSeleccionado() {

  switch (campoFechaSeleccionado) {

    case DIA:
      lcd.setCursor(6, 1);
      lcd.print("  ");
      break;

    case MES:
      lcd.setCursor(9, 1);
      lcd.print("  ");
      break;

    case ANIO:
      lcd.setCursor(12, 1);
      lcd.print("    ");
      break;
  }
}

void actualizarParpadeoFecha() {

  if (millis() - ultimoParpadeo >= INTERVALO_PARPADEO) {

    ultimoParpadeo = millis();

    mostrarCampoSeleccionado = !mostrarCampoSeleccionado;

    if (mostrarCampoSeleccionado) {
      mostrarCampoFechaSeleccionado();
    } else {
      ocultarCampoFechaSeleccionado();
    }
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
    actualizarHoraFecha();
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
    
    settings.setUnidadTemperatura(unidadTemperaturaTemporal);

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
    
    settings.setIntervaloMedicion(intervaloMedicionTemporal);
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
   
    // Guardar el formato de hora seleccionado en las preferencias
    settings.setFormatoHora(formatoHoraTemporal);

    // Actualizar el formato de hora del reloj
    reloj.setFormato(formatoHoraTemporal);

    regresar();
  }

  // Cancelar la selección y regresar al estado anterior
  if (keyboard.getReleasedKey("Back")) {
    
    regresar();
  }
}

void manejarHora() {

  if (keyboard.getReleasedKey("Left")) {

    if (reloj.getFormato() == FORMATO_24_HORAS) {

      if (campoHoraSeleccionado == HORA) {
        campoHoraSeleccionado = SEGUNDO;
      }
      else if (campoHoraSeleccionado == MINUTO) {
        campoHoraSeleccionado = HORA;
      }
      else if (campoHoraSeleccionado == SEGUNDO) {
        campoHoraSeleccionado = MINUTO;
      }
    }
    else {

      if (campoHoraSeleccionado == HORA) {
        campoHoraSeleccionado = AM_PM;
      }
      else if (campoHoraSeleccionado == MINUTO) {
        campoHoraSeleccionado = HORA;
      }
      else if (campoHoraSeleccionado == SEGUNDO) {
        campoHoraSeleccionado = MINUTO;
      }
      else if (campoHoraSeleccionado == AM_PM) {
        campoHoraSeleccionado = SEGUNDO;
      }
    }

    mostrarCampoSeleccionado = true;
    ultimoParpadeo = millis();

    mostrarHoraEdicion();
  }

  if (keyboard.getReleasedKey("Right")) {

    if (reloj.getFormato() == FORMATO_24_HORAS) {

      if (campoHoraSeleccionado == HORA) {
        campoHoraSeleccionado = MINUTO;
      }
      else if (campoHoraSeleccionado == MINUTO) {
        campoHoraSeleccionado = SEGUNDO;
      }
      else if (campoHoraSeleccionado == SEGUNDO) {
        campoHoraSeleccionado = HORA;
      }
    }
    else {

      if (campoHoraSeleccionado == HORA) {
        campoHoraSeleccionado = MINUTO;
      }
      else if (campoHoraSeleccionado == MINUTO) {
        campoHoraSeleccionado = SEGUNDO;
      }
      else if (campoHoraSeleccionado == SEGUNDO) {
        campoHoraSeleccionado = AM_PM;
      }
      else if (campoHoraSeleccionado == AM_PM) {
        campoHoraSeleccionado = HORA;
      }
    }

    mostrarCampoSeleccionado = true;
    ultimoParpadeo = millis();

    mostrarHoraEdicion();
  }

  if (keyboard.isRepeated("Up")) {

    switch (campoHoraSeleccionado) {

      case HORA:
        aumentarHora();
        actualizarHoraYAMPM();
        break;

      case MINUTO:
        aumentarMinuto();
        break;

      case SEGUNDO:
        aumentarSegundo();
        break;

      case AM_PM:
        alternarAMPM();
        break;
    }

    mostrarCampoSeleccionado = true;
    ultimoParpadeo = millis();
    mostrarCampoHoraSeleccionado();
  }

  if (keyboard.isRepeated("Down")) {

    switch (campoHoraSeleccionado) {

      case HORA:
        disminuirHora();
        actualizarHoraYAMPM();
        break;

      case MINUTO:
        disminuirMinuto();
        break;

      case SEGUNDO:
        disminuirSegundo();
        break;

      case AM_PM:
        alternarAMPM();
        break;
    }

    mostrarCampoSeleccionado = true;
    ultimoParpadeo = millis();
    mostrarCampoHoraSeleccionado();
  }

  if (keyboard.getReleasedKey("Back")) {
    regresar();
    return;
  }

  if (keyboard.getReleasedKey("Ok")) {

    reloj.update();

    DateTime ahora = reloj.now();

    // Guardar la hora configurada en el RTC
    DateTime nuevaHora(
      ahora.year(),
      ahora.month(),
      ahora.day(),
      horaTemporal,
      minutoTemporal,
      segundoTemporal
    );

    reloj.setDateTime(nuevaHora);

    regresar();
    return;
  }
}

void aumentarHora() {
  horaTemporal++;

  if (horaTemporal > 23) {
    horaTemporal = 0;
  }
}

void disminuirHora() {
  horaTemporal--;

  if (horaTemporal < 0) {
    horaTemporal = 23;
  }
}

void aumentarMinuto() {

  minutoTemporal++;

  if (minutoTemporal > 59) {
    minutoTemporal = 0;
  }
}

void disminuirMinuto() {

  minutoTemporal--;

  if (minutoTemporal < 0) {
    minutoTemporal = 59;
  }
}

void aumentarSegundo() {

  segundoTemporal++;

  if (segundoTemporal > 59) {
    segundoTemporal = 0;
  }
}

void disminuirSegundo() {

  segundoTemporal--;

  if (segundoTemporal < 0) {
    segundoTemporal = 59;
  }
}

void alternarAMPM() {

  if (horaTemporal < 12) {
    horaTemporal += 12;
  } else {
    horaTemporal -= 12;
  }
}

void actualizarHoraYAMPM() {

  mostrarCampoHoraSeleccionado();

  if (reloj.getFormato() == FORMATO_12_HORAS) {

    lcd.setCursor(13, 1);

    if (horaTemporal < 12) {
      lcd.print(" am");
    } else {
      lcd.print(" pm");
    }
  }
}

void aumentarDia() {

  diaTemporal++;

  int maximoDias = diasEnMes(mesTemporal, anioTemporal);

  if (diaTemporal > maximoDias) {
    diaTemporal = 1;
  }
}

void disminuirDia() {

  diaTemporal--;

  if (diaTemporal < 1) {
    diaTemporal = diasEnMes(mesTemporal, anioTemporal);
  }
}

bool aumentarMes() {
  mesTemporal++;

  if (mesTemporal > 12) {
    mesTemporal = 1;
  }

  int maximoDias = diasEnMes(mesTemporal, anioTemporal);

  if (diaTemporal > maximoDias) {
    diaTemporal = maximoDias;
    return true; // El día también cambió
  }

  return false; // Solo cambió el mes
}

bool disminuirMes() {

  mesTemporal--;

  if (mesTemporal < 1) {
    mesTemporal = 12;
  }

  int maximoDias = diasEnMes(mesTemporal, anioTemporal);

  if (diaTemporal > maximoDias) {
    diaTemporal = maximoDias;
    return true; // El día también cambió
  }

  return false; // Solo cambió el mes
}

bool aumentarAnio() {

  anioTemporal++;

  if (anioTemporal > 2099) {
    anioTemporal = 2000;
  }

  int maximoDias = diasEnMes(mesTemporal, anioTemporal);

  if (diaTemporal > maximoDias) {
    diaTemporal = maximoDias;
    return true; // El día también cambió
  }

  return false; // Solo cambió el año
}

bool disminuirAnio() {

  anioTemporal--;

  if (anioTemporal < 2000) {
    anioTemporal = 2099;
  }

  int maximoDias = diasEnMes(mesTemporal, anioTemporal);

  if (diaTemporal > maximoDias) {
    diaTemporal = maximoDias;
    return true; // El día también cambió
  }

  return false; // Solo cambió el año
}

int diasEnMes(int mes, int anio) {

  switch (mes) {

    case 2:
      if ((anio % 400 == 0) || (anio % 4 == 0 && anio % 100 != 0)) {
        return 29;
      }

      return 28;

    case 4:
    case 6:
    case 9:
    case 11:
      return 30;

    default:
      return 31;
  }
}

void manejarFecha() {

  if (keyboard.getReleasedKey("Right")) {

    switch (campoFechaSeleccionado) {
      case DIA:
        campoFechaSeleccionado = MES;
        break;

      case MES:
        campoFechaSeleccionado = ANIO;
        break;

      case ANIO:
        campoFechaSeleccionado = DIA;
        break;
    }

    mostrarCampoSeleccionado = true;
    ultimoParpadeo = millis();

    mostrarFechaEdicion();
    mostrarCampoFechaSeleccionado();
  }


  if (keyboard.getReleasedKey("Left")) {

    switch (campoFechaSeleccionado) {
      case DIA:
        campoFechaSeleccionado = ANIO;
        break;

      case MES:
        campoFechaSeleccionado = DIA;
        break;

      case ANIO:
        campoFechaSeleccionado = MES;
        break;
    }

    mostrarCampoSeleccionado = true;
    ultimoParpadeo = millis();

    mostrarFechaEdicion();
    mostrarCampoFechaSeleccionado();
  }

  if (keyboard.isRepeated("Up")) {

    bool diaCambio = false;

    switch (campoFechaSeleccionado) {
      case DIA:
        aumentarDia();
        break;

      case MES:
        diaCambio = aumentarMes();
        break;

      case ANIO:
        diaCambio = aumentarAnio();
        break;
    }

    mostrarCampoSeleccionado = true;
    ultimoParpadeo = millis();

    if (diaCambio) {
      mostrarFechaEdicion();
      mostrarCampoFechaSeleccionado();
    } else {
      mostrarCampoFechaSeleccionado();
    }
  }

  if (keyboard.isRepeated("Down")) {

    bool diaCambio = false;

    switch (campoFechaSeleccionado) {
      case DIA:
        disminuirDia();
        break;

      case MES:
        diaCambio = disminuirMes();
        break;

      case ANIO:
        diaCambio = disminuirAnio();
        break;
    }

    mostrarCampoSeleccionado = true;
    ultimoParpadeo = millis();

    if (diaCambio) {
      mostrarFechaEdicion();
      mostrarCampoFechaSeleccionado();
    } else {
      mostrarCampoFechaSeleccionado();
    }
  }

  if (keyboard.getReleasedKey("Ok")) {

    reloj.update();

    DateTime ahora = reloj.now();

    DateTime nuevaFecha(
      anioTemporal,
      mesTemporal,
      diaTemporal,
      ahora.hour(),
      ahora.minute(),
      ahora.second()
    );

    reloj.setDateTime(nuevaFecha);

    regresar();
    return;
  }

  if (keyboard.getReleasedKey("Back")) {

    regresar();
    return;
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
    unidadTemperaturaTemporal = settings.getUnidadTemperatura();
  }

  // Guardar el intervalo de medición actual antes de cambiar
  if (nuevoEstado == MENU_INTERVALO) {
    intervaloMedicionTemporal = settings.getIntervaloMedicion();
  }

  // Guardar el formato de hora actual antes de cambiar
  if (nuevoEstado == MENU_FORMATO_HORA) {
    formatoHoraTemporal = settings.getFormatoHora();
  }

  if (nuevoEstado == PANTALLA_EDITAR_HORA) {
    
    reloj.update();
    DateTime ahora = reloj.now();

    horaTemporal = ahora.hour();
    minutoTemporal = ahora.minute();
    segundoTemporal = ahora.second();

    campoHoraSeleccionado = HORA;

    mostrarCampoSeleccionado = true;
    ultimoParpadeo = millis();
  }

  if (nuevoEstado == PANTALLA_EDITAR_FECHA) {

    reloj.update();

    DateTime ahora = reloj.now();

    diaTemporal = ahora.day();
    mesTemporal = ahora.month();
    anioTemporal = ahora.year();

    campoFechaSeleccionado = DIA;

    mostrarCampoSeleccionado = true;
    ultimoParpadeo = millis();
  }

  // Mostrar el nuevo estado en la pantalla
  mostrarEstado();
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

    case PANTALLA_EDITAR_HORA:
      mostrarHoraEdicion();
      break;

    case PANTALLA_EDITAR_FECHA:
      mostrarFechaEdicion();
      mostrarCampoFechaSeleccionado();
      break;
  }
}
