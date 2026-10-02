# NoriegaIoT.Core

Core reutilizable para proyectos IoT basados en ESP32.

`NoriegaIoT.Core` es una colección de componentes y utilidades diseñados para servir como base común para diferentes dispositivos del ecosistema **NoriegaIoT**.

El objetivo del proyecto es evitar que cada dispositivo tenga que implementar nuevamente funcionalidades comunes, manteniendo una arquitectura sencilla, reutilizable y fácil de mantener.

---

## 🎯 Objetivo

Este proyecto busca proporcionar una base común para futuros dispositivos IoT desarrollados bajo el ecosistema NoriegaIoT.

Entre las funcionalidades que podrán formar parte del Core se encuentran:

* Manejo de botones.
* Teclados matriciales.
* Pantallas y sistemas de visualización.
* Navegación entre menús y estados.
* Configuración persistente.
* Manejo de fecha y hora.
* Comunicación con otros componentes del sistema.
* Otras funcionalidades comunes a múltiples dispositivos.

Los componentes específicos de cada dispositivo deberán permanecer fuera del Core.

---

## 📁 Estructura del proyecto

```text
NoriegaIoT.Core/
│
├── src/
│   ├── Button.cpp
│   ├── Button.h
│   ├── MatrixKeyboard.cpp
│   └── MatrixKeyboard.h
│
├── examples/
│   └── main/
│       └── main.ino
│
├── library.properties
└── README.md
```

### `src/`

Contiene los componentes reutilizables del Core.

Actualmente incluye:

* `Button` — manejo de botones individuales.
* `MatrixKeyboard` — manejo de teclados matriciales mediante I²C.

### `examples/`

Contiene ejemplos destinados a probar y demostrar el uso de los componentes del Core.

Estos ejemplos no forman parte de la lógica específica de un producto o dispositivo.

### `library.properties`

Archivo de configuración utilizado para identificar el proyecto como una librería compatible con Arduino.

---

## 🔧 Requisitos

Actualmente, el proyecto está orientado a:

* **ESP32**
* **Arduino Framework**
* **Arduino IDE**
* **C++**

La compatibilidad puede ampliarse en el futuro cuando los componentes del Core no dependan de funcionalidades específicas del ESP32.

---

## 🚀 Uso

Una vez instalada la librería, los componentes pueden incluirse desde un proyecto Arduino mediante:

```cpp
#include <Button.h>
#include <MatrixKeyboard.h>
```

Por ejemplo:

```cpp
#include <Button.h>

Button button(4);

void setup() {
    button.begin();
}

void loop() {
    if (button.pressed()) {
        // Acción del botón
    }
}
```

Los ejemplos incluidos en `examples/` contienen demostraciones más completas.

---

## 🧩 Filosofía del proyecto

`NoriegaIoT.Core` busca mantener una arquitectura:

* **Simple**
* **Reutilizable**
* **Modular**
* **Mantenible**
* **Extensible**

Los componentes deben tener responsabilidades claras y evitar dependencias innecesarias.

El Core no debe contener lógica específica de un dispositivo.

Por ejemplo, una clase para manejar un teclado pertenece al Core:

```text
MatrixKeyboard
```

Mientras que la lógica específica de una estación meteorológica pertenece a su propio proyecto:

```text
WeatherStation
```

De esta manera, diferentes dispositivos pueden compartir el mismo Core sin estar acoplados entre sí.

---

## 🌐 Ecosistema NoriegaIoT

`NoriegaIoT.Core` forma parte de una arquitectura mayor para futuros proyectos IoT.

Conceptualmente:

```text
                    NoriegaIoT.Server
                           ▲
                           │
                    Comunicación
                           │
          ┌────────────────┼────────────────┐
          │                │                │
    WeatherStation   EnergyMonitor    PetFeeder
          │                │                │
          └────────────────┼────────────────┘
                           │
                    NoriegaIoT.Core
```

Cada dispositivo tendrá su propio proyecto y utilizará los componentes comunes proporcionados por `NoriegaIoT.Core`.

El servidor será desarrollado de manera independiente.

---

## 🛠️ Estado del proyecto

El proyecto se encuentra actualmente en desarrollo.

### Componentes disponibles

* [x] `Button`
* [x] `MatrixKeyboard`

### Componentes previstos

* [ ] `Display`
* [ ] `Navigation`
* [ ] `Settings`
* [ ] `Clock`
* [ ] Comunicación
* [ ] Otros componentes comunes

La lista irá evolucionando conforme se
### 📌 Principio de diseño

Una funcionalidad debe incorporarse al Core cuando pueda ser utilizada de forma razonable por varios proyectos diferentes.

Las funcionalidades específicas de un dispositivo deben permanecer en el repositorio correspondiente a ese dispositivo.

Esto permite que NoriegaIoT.Core permanezca pequeño, estable y reutilizable.

### 📄 Licencia

Este proyecto está distribuido bajo la licencia MIT.

Consulta el archivo LICENSE para obtener los términos completos de la licencia.

### 👤 Autor

Mateo Noriega

GitHub: @noriega29
