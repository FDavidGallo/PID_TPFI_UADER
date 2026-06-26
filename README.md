# 🌡️ Control PID para  Enseñanza 

> **Trabajo Práctico Final · Proyecto de Sistemas Digitales · UADER FCyT 2026*

> *Licenciatura en Automatización y Control de Procesos Industriales*

**Autores:** Suffo Emanuel · Gallo Fabricio David
**Profesores:** Ing. Eduardo Velázquez · Ing Guillermo Gerard

> <img width="135" height="45" alt="logo (5)" src="https://github.com/user-attachments/assets/dddc2f87-ea36-4421-aa79-d720b334455c" />
[![C++](https://img.shields.io/badge/C++-ESP32%20Arduino-00599C?style=flat&logo=cplusplus)](https://www.arduino.cc/)
[![Node-RED](https://img.shields.io/badge/Node--RED-v4.1.10-8F0000?style=flat&logo=nodered)](https://nodered.org/)
[![WebSocket](https://img.shields.io/badge/Comunicación-WebSocket%20Seguro-4CAF50?style=flat)](https://websockets.spec.whatwg.org/)
[![Pangolin](https://img.shields.io/badge/Túnel-Pangolin%20%2B%20WireGuard-FF6B00?style=flat)](https://docs.pangolin.net/)

---

## 🎯 ¿Qué es esto?

Un sistema **Un calentador de agua con control PID**, diseñado con fines pedagógicos.
Permite que docentes y alumnos controlen y visualicen en **tiempo real** —desde el teléfono o la PC— las variables de un lazo de control PID.


## 🏗️ Arquitectura general

<img width="2048" height="512" alt="Gemini_Generated_Image_lrgoqlrgoqlrgoql" src="https://github.com/user-attachments/assets/e4781a48-a8eb-423a-9227-6ffd657164d0" />

```

> La comunicación es completamente bidireccional. El servidor puede estar en otro país y funciona igual.

---

## 📁 Estructura del repositorio

```
PID_TPFI_UADER/
│
├── 📂 Seccion Micro/          # Firmware ESP32 (Arduino IDE / C++)
│   ├── Sistema.ino            # Programa principal (Setup + Loop)
│   ├── Headers.h              # Incluye todas las clases
│   ├── Contrasenha.h          # ⚠️ NO incluido (.gitignore) — ver abajo
│   ├── Comunicaciones.h/.cpp  # WiFi + WebSocket seguro
│   ├── Control.h/.cpp         # Orquesta PID y ON-OFF
│   ├── PIDDiscreto.h/.cpp     # Controlador PID discreto
│   ├── ControlBinario.h/.cpp  # Control ON-OFF con histéresis
│   ├── Actuador.h             # Clase base abstracta (actuadores)
│   ├── Rele.h/.cpp            # Relé SSR con PWM por tiempo
│   ├── Transistor.h/.cpp      # Transistor con PWM (ledc ESP32)
│   ├── SensorDeTemperatura.h  # Clase abstracta (sensores)
│   ├── Termocupla.h/.cpp      # MAX6675 vía SPI
│   ├── PotenciometroTemp.h/.cpp # Potenciómetro como sensor simulado
│   └── SensorDeNivel.h/.cpp   # Sensor de nivel (ADC)
│
├── 📂 Seccion Web/            # Flujo Node-RED (Backend + Dashboard)
│   ├── flow.json              # Flujo exportado de Node-RED
│   └── Arrancar.bat           # Script para lanzar el servidor fácil
│
├── 📂 Documentacion/          # Informe técnico y diagramas
│
├── .gitignore
└── README.md
```

### ⚠️ `Contrasenha.h` — archivo que debés crear vos

Este archivo **no está en el repo** (está en `.gitignore`) porque contiene credenciales WiFi que algún vecino podría querer.
Crealo en `Seccion Micro/` con esta estructura:

```cpp
// Contrasenha.h
#ifndef CONTRASENHA_H
#define CONTRASENHA_H
#define RED        "NombreDeTuRed"
#define contrasenha "TuContraseña"
#endif
```

> Si la red no está disponible, el ESP32 crea automáticamente un hotspot llamado **`controlESP`** (contraseña: `control123`) para configurar otra red.

---


> El programa opera en **modo Polling** con tiempo de muestreo configurable.  
> Esto es intencional: refuerza el carácter **digital y discreto** del control (fines pedagógicos).

---

## 🔌 Hardware

| Componente | Rol | Detalles |
|---|---|---|
| **ESPDuino (D1 R32)** | Microcontrolador | ESP32, WiFi, Bluetooth 4.2, compatible con shields Arduino |
| **MAX6675** | Sensor temperatura | Termocupla tipo K, 12-bit ADC, SPI, compensación de unión fría |
| **Potenciómetro** | Sensor simulado | Permite simular errores y explicar sintonía a lazo abierto |
| **Sensor de nivel** | Seguridad | ADC con umbrales; corta el sistema si no hay agua |
| **Relé SSR (40A)** | Actuador 1 | Resistencia herradura 66Ω a 220V, controlada con PWM temporal |
| **Transistor 2N3055** | Actuador 2 | Resistencia a 19V, controlada con PWM (ledc ESP32) |

---

## 🚀 Cómo levantar el servidor

### Prerrequisitos
- Node.js + npm instalados
- Node-RED instalado (`npm install -g node-red`)
- Dashboard instalado (`npm install node-red-dashboard`)
- Pangolin configurado (ver documentación)

### Pasos

**Opción 1 — Windows (fácil):**
```
Doble clic en Seccion Web/Arrancar.bat
```

**Opción 2 — Cualquier SO:**
```bash
cd "Seccion Web"
node-red flow.json
```

Luego abrí el dashboard en `http://localhost:1880/ui`

> Para exposición pública sin abrir puertos, usamos **Pangolin + WireGuard**.  
> El dominio del proyecto es: `https://trabajofinalintegradoruader.hostlocal.app/ui/`

---

## 🧩 Conceptos de POO aplicados

| Concepto | Dónde se aplica |
|---|---|
| **Abstracción** | `SensorDeTemperatura` y `Actuador` definen contratos sin implementación |
| **Herencia** | `Termocupla`, `PotenciometroTemp`, `Rele`, `Transistor` heredan de sus bases |
| **Polimorfismo** | `sensorActivo->LeerTemperatura()` ejecuta la versión correcta en runtime |
| **Encapsulamiento** | Atributos `private`, acceso solo por métodos públicos |
| **Composición** | `Control` contiene un `PIDDiscreto` y un `ControlBinario` |
| **Fábrica estática** | `SensorDeNivel::crear()` retorna una nueva instancia |

---

## 📄 Más información

Este README es una versión resumida.
Para la documentación completa del proyecto —esquemáticos, diagramas de secuencia, descripción detallada de cada clase y del servidor Node-RED— podés consultar el **informe técnico** en la carpeta [`Documentacion/`](./Documentacion/informe.pdf).

---

*FCyT — UADER · Proyecto de Sistemas Digitales · 2026*
