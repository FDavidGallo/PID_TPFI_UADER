//  ____  _     _                             _                               
// / ___|(_)___| |_ ___ _ __ ___   __ _    __| | ___                          
// \___ \| / __| __/ _ \ '_ ` _ \ / _` |  / _` |/ _ \                         
//  ___) | \__ \ ||  __/ | | | | | (_| | | (_| |  __/                         
// |____/|_|___/\__\___|_| |_| |_|\__,_|  \__,_|\___|                         
//   ____            _             _   ____ ___ ____                          
//  / ___|___  _ __ | |_ _ __ ___ | | |  _ \_ _|  _ \   _ __   __ _ _ __ __ _ 
// | |   / _ \| '_ \| __| '__/ _ \| | | |_) | || | | | | '_ \ / _` | '__/ _` |
// | |__| (_) | | | | |_| | | (_) | | |  __/| || |_| | | |_) | (_| | | | (_| |
//  \____\___/|_| |_|\__|/\/|\___/|_| |_|  |___|____/  | .__/ \__,_|_|  \__,_|
//   ___ _ __  ___  ___ |/\/   __ _ _ __  ______ _     |_|                    
//  / _ \ '_ \/ __|/ _ \ '_ \ / _` | '_ \|_  / _` |                           
// |  __/ | | \__ \  __/ | | | (_| | | | |/ / (_| |                           
//  \___|_| |_|___/\___|_| |_|\__,_|_| |_/___\__,_|                           

// Un trabajo de: Emanuel Alejandro Suffo y Fabricio David Gallo
// Para la cátedra: Proyecto de Sistemas Digitales (UADER FCYT)
// Profesores: Ing Guillermo Gerard e Ing Eduardo Velazquez

#include <Arduino.h> //Libreria de arduino, sabemos que no hace falta
                     // Pero la incluimos de todas formas, por si queremos
                     // Pasarnos a PlatFormio
#include <WebSocketsClient.h> // Esto es para manejar Websockets
#include "Headers.h"          // Acá  incluimos las demás clases 

// ==================== DEFINICIÓN DE PINES (ESP32) ====================
#define PIN_THERMO_SO 19
#define PIN_THERMO_CS 23
#define PIN_THERMO_SCK 5
#define PIN_POTENCIOMETRO 34
#define PIN_RELE 13
#define PIN_TRANSISTOR 12
#define PIN_NIVEL 39

#define NIVEL_MIN_RAW 400  // Marcamos un minimo de agua en el sensor, 
                           // por pruebas se determinó que cuando está con gotas marca ~315
#define NIVEL_MAX_RAW 3300 // 3,3 V

// ==================== PARÁMETROS POR DEFECTO ====================
float SETPOINT_DEFAULT = 25.0;  // 25 grados
float KP_DEFAULT = 7.5;
float KI_DEFAULT = 0.0;
float KD_DEFAULT = 0.0;
double tiempoMuestreo = 1.0;                // segundos
int BASE_TIEMPO_RELE_DEFAULT = 100;         // ms (múltiplo de 20 para SSR)
int FRECUENCIA_TRANSISTOR_DEFAULT = 1000;   // Hz
float HISTERESIS_DEFAULT = 2.0;             // grados para control ON-OFF

// ==================== OBJETOS GLOBALES ====================
Comunicaciones comunica;
// De acá en adelante  los declaramos como punteros, ya que luego se crean con new
// cuando sea conveniente...
Termocupla* termocupla = nullptr;
PotenciometroTemp* potenciometro = nullptr;
SensorDeTemperatura* sensorActivo = nullptr;
Rele* rele = nullptr;
Transistor* transistor = nullptr;
actuador* actuadorActivo = nullptr;
Control* control = nullptr;
SensorDeNivel* sensorNivel = nullptr;
//// =================== VARIABLES DE CONTROL ===================
bool nivelSeguro = false;
unsigned long tiempoUltimoControl = 0;
float setpoint = SETPOINT_DEFAULT;
float ultimaTemperatura = 0.0;
float ultimaSalida = 0.0;
String sensorActualStr = "termocupla"; // Por defecto
String actuadorActualStr = "rele";     // Por defecto

// ==================== PROTOTIPOS ====================
void procesarComando(String cmd);
void actualizarActuadorInactivo();
void verificarNivelYSeguridad();
void loopControl();
void enviarTelemetria();
void alRecibirMensaje(String mensaje);
void recrearControl(); // ya no se usa para cambiar parámetros, solo para inicial

// ==================== SETUP ====================
void setup() {
  Serial.begin(115200);
  Serial.println("\n===== SISTEMA DE CONTROL PID CON ESP32 =====");
  Serial.println("[INFO] Tiempo de muestreo inicial: 1 segundo (use comando ts:5)");

  sensorNivel = SensorDeNivel::crear(NIVEL_MIN_RAW, NIVEL_MAX_RAW, PIN_NIVEL);  // Creamos  la clase sensor de nivel
  while (sensorNivel == nullptr) {
    Serial.println("ERROR CRÍTICO: SensorDeNivel. Reinicie."); // si no existe el sensor que no arranque!; esto no debería
    delay(250);
  }
  sensorNivel->Iniciar();  // arrancamos  el sensor de nivek

  int estadoInicial = sensorNivel->ChequearNivel(); // antes del primer control chequeamos el nivel
  nivelSeguro = (estadoInicial == 0);
  if (!nivelSeguro) {
    Serial.println("[NIVEL] Inicialmente inseguro → salida forzada a 0");
    if (control) control->ForzarSalida(true, 0.0);
  } else {
    Serial.println("[NIVEL] Inicialmente seguro");
  }

  termocupla = new Termocupla(PIN_THERMO_SO, PIN_THERMO_CS, PIN_THERMO_SCK); // Creamos con  new un objeto termocupla :^)
  potenciometro = new PotenciometroTemp(PIN_POTENCIOMETRO, -10.0, 120.0, 3.3, 4096); // Creamos el pbjeto potenciometro 
                                                                                     // (los extremos de temperaturas son arbitrarios)
  termocupla->Inciar();   // Arrancamos los dos
  potenciometro->Inciar();
  sensorActivo = termocupla;
  Serial.println("[INFO] Sensor activo: termocupla");

  rele = new Rele(PIN_RELE, BASE_TIEMPO_RELE_DEFAULT, "rele");    // objeto relé instanciado
  transistor = new Transistor(PIN_TRANSISTOR, FRECUENCIA_TRANSISTOR_DEFAULT, "transistor"); // objeto transistor instanciado
  actuadorActivo = rele;
  Serial.println("[INFO] Actuador activo: rele");

  // Crear control una sola vez
  control = new Control(KP_DEFAULT, KI_DEFAULT, KD_DEFAULT, // cargamos valores por defecto
                        tiempoMuestreo,
                        HISTERESIS_DEFAULT,
                        true, false);   // arranca en PID
  control->Reset();
  Serial.printf("[PID] Creado con Ts = %.1f s, Kp=%.2f, Ki=%.2f, Kd=%.2f, Histeresis=%.1f\n",
                tiempoMuestreo, KP_DEFAULT, KI_DEFAULT, KD_DEFAULT, HISTERESIS_DEFAULT);

  if (!nivelSeguro && control) control->ForzarSalida(true, 0.0);

  comunica.begin("trabajofinalintegradoruader.hostlocal.app", 443, "/pepe", 120); // le pega a ese websockect, le pusimos pepe con fines pedagógicos y humorísticos
  comunica.onMensajeRecibido(alRecibirMensaje); // recibió un mensaje, vaya a "alRecibirMensaje"


  if (rele) rele->Aplicar(0); //que arranque en 0
  if (transistor) transistor->Aplicar(0); // (este igual)  

  tiempoUltimoControl = millis();
}

// ==================== LOOP PRINCIPAL ====================
void loop() {
  comunica.loop(); //manda y procesa datos entrantes

  if (rele) rele->loop(); //Actualizamos los datos del rele (los milis)

  if (Serial.available()) {  // Si el usuario mandó algo por serial (para pruebas)
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd.length() > 0) procesarComando(cmd); 
  }

  verificarNivelYSeguridad(); //Verificamos que el nibel sea seguro

  unsigned long intervaloMs = (unsigned long)(tiempoMuestreo * 1000); // Cada vez que se dá un tiempo de muestreo se controla
  if (millis() - tiempoUltimoControl >= intervaloMs) {
    loopControl(); // Loop de control
    tiempoUltimoControl = millis();
    enviarTelemetria(); //Enviamos los datos al servidor
  }
}

// ==================== PROCESAMIENTO DE COMANDOS ====================
void procesarComando(String cmd) {
  cmd.toLowerCase(); // Mandamos todo a minusculas, por si el usuario se equivoca
  Serial.print("[COMANDO] "); // che recibí este comando
  Serial.println(cmd);        // mirá:

  if (cmd.startsWith("setpoint:")) {
    float val = cmd.substring(9).toFloat();
    if (val >= -20 && val <= 120) {
      setpoint = val;
      Serial.printf("Setpoint: %.2f °C\n", setpoint);
    } else {
      Serial.println("ERROR: setpoint FUERA DE RANGO (0 A 120 grados C)");
    }
  } else if (cmd.startsWith("kp:")) {
    float val = cmd.substring(3).toFloat();
    if (val >= 0 && val <= 255) {
      KP_DEFAULT = val;
      if (control) control->setKp(val);
      Serial.printf("Kp = %.2f\n", val);
    } else {
      Serial.println("ERROR: Kp 0..255");
    }
  } else if (cmd.startsWith("ki:")) {
    float val = cmd.substring(3).toFloat();
    if (val >= 0 && val <= 100) {
      KI_DEFAULT = val;
      if (control) control->setKi(val);
      Serial.printf("Ki = %.2f\n", val);
    } else {
      Serial.println("ERROR: Ki 0..100");
    }
  } else if (cmd.startsWith("kd:")) {
    float val = cmd.substring(3).toFloat();
    if (val >= 0 && val <= 100) {
      KD_DEFAULT = val;
      if (control) control->setKd(val);
      Serial.printf("Kd = %.2f\n", val);
    } else {
      Serial.println("ERROR: Kd 0..100");
    }
  } else if (cmd.startsWith("ts:")) {
    float val = cmd.substring(3).toFloat();
    if (val >= 1.0 && val <= 10.0) {
      tiempoMuestreo = val;
      if (control) control->setTs(tiempoMuestreo);
      Serial.printf("Tiempo de muestreo cambiado a %.1f segundos\n", tiempoMuestreo);
    } else {
      Serial.println("ERROR: T debe estar entre 1 y 10 segundos");
    }
  } else if (cmd.startsWith("sensor:")) {
    String tipo = cmd.substring(7);
    if (tipo == "termocupla") {
      sensorActivo = termocupla;
      sensorActualStr = "termocupla";
      Serial.println("Sensor: termocupla");
    } else if (tipo == "poten") {
      sensorActivo = potenciometro;
      sensorActualStr = "potenciometro";
      Serial.println("Sensor: potenciómetro");
    } else {
      Serial.println("Opciones: termocupla, poten");
    }
  } else if (cmd.startsWith("actuador:")) {
    String tipo = cmd.substring(9);
    if (tipo == "rele") {
      actuadorActivo = rele;
      actuadorActualStr = "rele";
      actualizarActuadorInactivo();
      Serial.println("Actuador: relé");
    } else if (tipo == "transistor") {
      actuadorActivo = transistor;
      actuadorActualStr = "transistor";
      actualizarActuadorInactivo();
      Serial.println("Actuador: transistor");
    } else {
      Serial.println("Opciones: rele, transistor");
    }
  } else if (cmd.startsWith("basetiemporele:")) {
    int ms = cmd.substring(15).toInt();
    if (ms >= 10 && ms <= 100000) {
      if (ms % 20 != 0) {
        Serial.println("ADVERTENCIA: el período debe ser múltiplo de 20 ms para SSR de cruce por cero.");
      }
      if (rele) {
        rele->setPeriodo(ms);
        Serial.printf("Base tiempo relé: %d ms\n", ms);
      } else {
        Serial.println("ERROR: objeto rele no creado");
      }
    } else {
      Serial.println("ERROR: 10..100000 ms");
    }
  } else if (cmd.startsWith("frecuenciatransistor:")) {
    int hz = cmd.substring(20).toInt();
    if (hz >= 100 && hz <= 5500) {
      if (transistor) {
        transistor->setFrecuencia(hz);
        Serial.printf("Frecuencia transistor: %d Hz\n", hz);
      } else {
        Serial.println("ERROR: objeto transistor no creado");
      }
    } else {
      Serial.println("ERROR: 100..5500 Hz");
    }
  } else if (cmd.startsWith("onoff:")) {
    String estado = cmd.substring(6);
    if (estado == "on") {
      if (control) {
        control->setModo(false, true);
        control->Reset();
        Serial.println("Control ON-OFF activado (histéresis " + String(HISTERESIS_DEFAULT) + " °C)");
      }
    } else if (estado == "off") {
      if (control) {
        control->setModo(true, false);
        control->Reset();
        Serial.println("Control PID activado");
      }
    } else {
      Serial.println("ERROR: usa onoff:on o onoff:off");
    }
  } else {
    Serial.println("Comandos: setpoint:XX, kp:X, ki:X, kd:X, ts:1..10, sensor:termocupla|poten, actuador:rele|transistor, baseTiempoRele:ms, frecuenciaTransistor:Hz, onoff:on|off");
  }
}

void actualizarActuadorInactivo() { // Para que no haya dos actuadores al mismo tiempo
  if (actuadorActivo == rele && transistor) transistor->Aplicar(0);
  else if (actuadorActivo == transistor && rele) rele->Aplicar(0);
}

// ==================== VERIFICACIÓN DE NIVEL ====================
void verificarNivelYSeguridad() {
  int estado = sensorNivel->ChequearNivel();
  bool nivelActualSeguro = (estado == 0);

  if (!nivelActualSeguro && nivelSeguro) {
    if (control) control->ForzarSalida(true, 0.0);
    nivelSeguro = false;
  } else if (nivelActualSeguro && !nivelSeguro) {
    if (control) control->ForzarSalida(false, 0.0);
    nivelSeguro = true;
  }
}
// ==================== LOOP DE CONTROL ====================

// Acá pasa la magia ewe 
void loopControl() {
  if (!sensorActivo || !actuadorActivo || !control) return; // Sé que sería más fácil con and y en positivo, pero ya quedó así
  float temp = sensorActivo->LeerTemperatura(); // antes que nada leemos la temperatura
  if (isnan(temp) || temp < -50 || temp > 300) return; // errores posibles: nan y fuera de rango (se usó el rango de la termocupla)
  ultimaTemperatura = temp;

  double salida = control->CalcularError(setpoint, temp);
  if (salida < 0) salida = 0;
  if (salida > 100) salida = 100;
  ultimaSalida = (float)salida;

  if (nivelSeguro)
    actuadorActivo->Aplicar((int)salida); // Ta todo bien, mande el control
  else
    actuadorActivo->Aplicar(0);

  actualizarActuadorInactivo();

  Serial.printf("T=%.1fs | Temp: %.2f | Set: %.2f | Salida: %.1f%% | Nivel: %s\n",
                tiempoMuestreo, temp, setpoint, salida, nivelSeguro ? "OK" : "INACEPTABLE"); // Acá usamos una terna pa  asignarle sies aceptable el nivel de agua o no
}
// ==================== Envío de datos al server de node-red ====================
void enviarTelemetria() {  
  if (!comunica.webSocketConectado()) return;
// Tiene que estar en formato Json; lo armamo:
  String msg = "{";
  msg += "\"temperatura\":" + String(ultimaTemperatura, 2) + ",";
  msg += "\"setpoint\":" + String(setpoint, 2) + ",";
  msg += "\"salida\":" + String(ultimaSalida, 1) + ",";
  msg += "\"sensor\":\"" + sensorActualStr + "\",";
  msg += "\"actuador\":\"" + actuadorActualStr + "\",";
  msg += "\"nivelSeguro\":" + String(nivelSeguro ? "true" : "false") + ",";
  msg += "\"ts\":" + String(tiempoMuestreo, 1);

  // Añadir información del tipo de controlador
  if (control) {
    bool pidOn = control->getBanderaPID();
    bool onOffOn = control->getBanderaOnOff();

    if (pidOn && !onOffOn) {
      // Modo PID
      msg += ",\"modo\":\"PID\"";
      msg += ",\"kp\":" + String(KP_DEFAULT, 2);
      msg += ",\"ki\":" + String(KI_DEFAULT, 2);
      msg += ",\"kd\":" + String(KD_DEFAULT, 2);
    } else if (!pidOn && onOffOn) {
      // Modo ON-OFF
      msg += ",\"modo\":\"ONOFF\"";
      msg += ",\"histeresis\":" + String(HISTERESIS_DEFAULT, 1); // siempre 2.0
    } else {
      // Caso raro: ambos o ninguno (no debería ocurrir)
      msg += ",\"modo\":\"MIXTO\"";
    }
  } else {
    msg += ",\"modo\":\"SIN_CONTROL\"";
  }

  msg += "}";
  comunica.enviarMensaje(msg);
}
//=========Al recibir un mensaje de websocket==========
void alRecibirMensaje(String mensaje) {
  Serial.print("[WSS] ");
  Serial.println(mensaje);
  procesarComando(mensaje);
}