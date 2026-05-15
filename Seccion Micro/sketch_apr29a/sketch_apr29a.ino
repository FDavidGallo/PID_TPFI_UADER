#include <WiFi.h>
#include <WebSocketsClient.h>
#include "Headers.h"

// ==================== DEFINICIÓN DE PINES (ESP32) ====================
#define PIN_THERMO_SO   19
#define PIN_THERMO_CS   23
#define PIN_THERMO_SCK  5
#define PIN_POTENCIOMETRO  34   // ADC1, solo entrada
#define PIN_RELE         13
#define PIN_TRANSISTOR   12
#define PIN_NIVEL        39

// Rangos del sensor de nivel (VALORES RAW DEL ADC, 0-4095)
#define NIVEL_MIN_RAW   800   // umbral inferior (por debajo → nivel bajo)
#define NIVEL_MAX_RAW   3300   // umbral superior (por encima → nivel alto)

// ==================== PARÁMETROS POR DEFECTO ====================
float SETPOINT_DEFAULT = 25.0;
float KP_DEFAULT = 10.0;
float KI_DEFAULT = 0.5;
float KD_DEFAULT = 1.0;
double tiempoMuestreo = 1.0;           // segundos (1..10)
int BASE_TIEMPO_RELE_DEFAULT = 100;     // ms
int BASE_TIEMPO_TRANSISTOR_DEFAULT = 1000; // µs

// ==================== OBJETOS GLOBALES ====================
Comunicaciones comunica;
Termocupla* termocupla = nullptr;
PotenciometroTemp* potenciometro = nullptr;
SensorDeTemperatura* sensorActivo = nullptr;
Rele* rele = nullptr;
Transistor* transistor = nullptr;
actuador* actuadorActivo = nullptr;
Control* control = nullptr;
SensorDeNivel* sensorNivel = nullptr;

bool nivelSeguro = false;          // se establecerá realmente en setup()
unsigned long tiempoUltimoControl = 0;
float setpoint = SETPOINT_DEFAULT;
float ultimaTemperatura = 0.0;
float ultimaSalida = 0.0;
String sensorActualStr = "termocupla";
String actuadorActualStr = "rele";

// ==================== PROTOTIPOS ====================
void procesarComando(String cmd);
void actualizarActuadorInactivo();
void verificarNivelYSeguridad();
void loopControl();
void enviarTelemetria();
void alRecibirMensaje(String mensaje);
void recrearControl();

// ==================== SETUP ====================
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n===== SISTEMA DE CONTROL PID CON ESP32 =====");
    Serial.println("[INFO] Tiempo de muestreo inicial: 1 segundo (use comando ts:5)");

    analogReadResolution(12);
    analogSetAttenuation(ADC_11db);

    // ---------- Sensor de nivel ----------
    sensorNivel = SensorDeNivel::crear(NIVEL_MIN_RAW, NIVEL_MAX_RAW, PIN_NIVEL);
    if (sensorNivel == nullptr) {
        Serial.println("ERROR CRÍTICO: SensorDeNivel. Reinicie.");
        while (1) delay(1000);
    }
    sensorNivel->Iniciar();

    // Lee el estado real del nivel al inicio y configura la seguridad
    int estadoInicial = sensorNivel->ChequearNivel();
    nivelSeguro = (estadoInicial == 0);
    if (!nivelSeguro) {
        Serial.println("[NIVEL] Inicialmente inseguro → salida forzada a 0");
        if (control) control->ForzarSalida(true, 0.0);
    } else {
        Serial.println("[NIVEL] Inicialmente seguro");
    }

    // ---------- Sensores de temperatura ----------
    termocupla = new Termocupla(PIN_THERMO_SO, PIN_THERMO_CS, PIN_THERMO_SCK);
    potenciometro = new PotenciometroTemp(PIN_POTENCIOMETRO, -10.0, 120.0, 3.3, 4096);
    termocupla->Inciar();
    potenciometro->Inciar();
    sensorActivo = termocupla;
    Serial.println("[INFO] Sensor activo: termocupla");

    // ---------- Actuadores ----------
    rele = new Rele(PIN_RELE, BASE_TIEMPO_RELE_DEFAULT, "rele");
    transistor = new Transistor(PIN_TRANSISTOR, BASE_TIEMPO_TRANSISTOR_DEFAULT, "transistor");
    actuadorActivo = rele;
    Serial.println("[INFO] Actuador activo: rele");

    // ---------- Controlador PID ----------
    recrearControl();
    // Si el nivel ya era inseguro, forzamos la salida después de crear el control
    if (!nivelSeguro && control) control->ForzarSalida(true, 0.0);

    // ---------- Comunicaciones WebSocket (WSS) ----------
    const char* WS_HOST = "trabajofinalintegradoruader.hostlocal.app";
    const uint16_t WS_PORT = 443;
    const char* WS_PATH = "/pepe";
    comunica.onMensajeRecibido(alRecibirMensaje);
    comunica.ConexionAutomatica(WS_HOST, WS_PORT, WS_PATH);

    // Apagar todo al inicio
    if (rele) rele->Aplicar(0);
    if (transistor) transistor->Aplicar(0);

    tiempoUltimoControl = millis();
}

// ==================== LOOP PRINCIPAL ====================
void loop() {
    comunica.loop();

    if (Serial.available()) {
        String cmd = Serial.readStringUntil('\n');
        cmd.trim();
        if (cmd.length() > 0) procesarComando(cmd);
    }

    // Verificar el nivel de líquido periódicamente
    verificarNivelYSeguridad();

    // Control por tiempo de muestreo variable
    unsigned long intervaloMs = (unsigned long)(tiempoMuestreo * 1000);
    if (millis() - tiempoUltimoControl >= intervaloMs) {
        loopControl();
        tiempoUltimoControl = millis();
        enviarTelemetria();
    }

    delay(10);
}

// ==================== RECREAR CONTROLADOR CON NUEVO T ====================
void recrearControl() {
    if (control != nullptr) delete control;
    control = new Control(KP_DEFAULT, KI_DEFAULT, KD_DEFAULT,
                          tiempoMuestreo,   // Ts actual
                          0.0, true, false);
    control->Reset();
    Serial.printf("[PID] Recreado con Ts = %.1f s, Kp=%.2f, Ki=%.2f, Kd=%.2f\n",
                  tiempoMuestreo, KP_DEFAULT, KI_DEFAULT, KD_DEFAULT);
}

// ==================== PROCESAMIENTO DE COMANDOS ====================
void procesarComando(String cmd) {
    cmd.toLowerCase();
    Serial.print("[COMANDO] ");
    Serial.println(cmd);

    if (cmd.startsWith("setpoint:")) {
        float val = cmd.substring(9).toFloat();
        if (val >= -50 && val <= 300) {
            setpoint = val;
            Serial.printf("Setpoint: %.2f °C\n", setpoint);
        } else { Serial.println("ERROR: setpoint -50..300"); }
    }
    else if (cmd.startsWith("kp:")) {
        float val = cmd.substring(3).toFloat();
        if (val >= 0 && val <= 255) {
            KP_DEFAULT = val;
            recrearControl();
        } else { Serial.println("ERROR: Kp 0..255"); }
    }
    else if (cmd.startsWith("ki:")) {
        float val = cmd.substring(3).toFloat();
        if (val >= 0 && val <= 100) {
            KI_DEFAULT = val;
            recrearControl();
        } else { Serial.println("ERROR: Ki 0..100"); }
    }
    else if (cmd.startsWith("kd:")) {
        float val = cmd.substring(3).toFloat();
        if (val >= 0 && val <= 100) {
            KD_DEFAULT = val;
            recrearControl();
        } else { Serial.println("ERROR: Kd 0..100"); }
    }
    else if (cmd.startsWith("ts:")) {
        float val = cmd.substring(3).toFloat();
        if (val >= 1.0 && val <= 10.0) {
            tiempoMuestreo = val;
            recrearControl();
            Serial.printf("Tiempo de muestreo cambiado a %.1f segundos\n", tiempoMuestreo);
        } else { Serial.println("ERROR: T debe estar entre 1 y 10 segundos"); }
    }
    else if (cmd.startsWith("sensor:")) {
        String tipo = cmd.substring(7);
        if (tipo == "termocupla") { sensorActivo = termocupla; sensorActualStr = "termocupla"; Serial.println("Sensor: termocupla"); }
        else if (tipo == "poten") { sensorActivo = potenciometro; sensorActualStr = "potenciometro"; Serial.println("Sensor: potenciómetro"); }
        else { Serial.println("Opciones: termocupla, poten"); }
    }
    else if (cmd.startsWith("actuador:")) {
        String tipo = cmd.substring(9);
        if (tipo == "rele") { actuadorActivo = rele; actuadorActualStr = "rele"; actualizarActuadorInactivo(); Serial.println("Actuador: relé"); }
        else if (tipo == "transistor") { actuadorActivo = transistor; actuadorActualStr = "transistor"; actualizarActuadorInactivo(); Serial.println("Actuador: transistor"); }
        else { Serial.println("Opciones: rele, transistor"); }
    }
    else if (cmd.startsWith("basetiemporele:")) {
        int ms = cmd.substring(15).toInt();
        if (ms >= 10 && ms <= 10000) {
            if (rele) delete rele;
            rele = new Rele(PIN_RELE, ms, "rele");
            if (actuadorActivo == rele) actuadorActivo = rele;
            Serial.printf("Base tiempo relé: %d ms\n", ms);
        } else { Serial.println("ERROR: 10..10000 ms"); }
    }
    else if (cmd.startsWith("basetiempotransistor:")) {
        int us = cmd.substring(20).toInt();
        if (us >= 100 && us <= 100000) {
            if (transistor) delete transistor;
            transistor = new Transistor(PIN_TRANSISTOR, us, "transistor");
            if (actuadorActivo == transistor) actuadorActivo = transistor;
            Serial.printf("Base tiempo transistor: %d us\n", us);
        } else { Serial.println("ERROR: 100..100000 us"); }
    }
    else {
        Serial.println("Comandos: setpoint:XX, kp:X, ki:X, kd:X, ts:1..10, sensor:termocupla|poten, actuador:rele|transistor, baseTiempoRele:ms, baseTiempoTransistor:us");
    }
}

void actualizarActuadorInactivo() {
    if (actuadorActivo == rele && transistor) transistor->Aplicar(0);
    else if (actuadorActivo == transistor && rele) rele->Aplicar(0);
}

// ==================== VERIFICACIÓN DE NIVEL (VERSIÓN SIMPLIFICADA) ====================
void verificarNivelYSeguridad() {
    int estado = sensorNivel->ChequearNivel();
    bool nivelActualSeguro = (estado == 0);   // 0 = dentro del rango, seguro

    // Transición de seguro a inseguro
    if (!nivelActualSeguro && nivelSeguro) {
        if (control) control->ForzarSalida(true, 0.0);
        nivelSeguro = false;
    }
    // Transición de inseguro a seguro
    else if (nivelActualSeguro && !nivelSeguro) {
        if (control) control->ForzarSalida(false, 0.0);
        nivelSeguro = true;
    }
}

void loopControl() {
    if (!sensorActivo || !actuadorActivo || !control) return;
    float temp = sensorActivo->LeerTemperatura();
    if (isnan(temp) || temp < -50 || temp > 300) return;
    ultimaTemperatura = temp;

    double salida = control->CalcularError(setpoint, temp);
    if (salida < 0) salida = 0;
    if (salida > 100) salida = 100;
    ultimaSalida = (float)salida;

    // Aplicar según estado de seguridad
    if (nivelSeguro)
        actuadorActivo->Aplicar((int)salida);
    else
        actuadorActivo->Aplicar(0);

    actualizarActuadorInactivo();

    Serial.printf("T=%.1fs | Temp: %.2f | Set: %.2f | Salida: %.1f%% | Nivel: %s\n",
                  tiempoMuestreo, temp, setpoint, salida, nivelSeguro ? "OK" : "INACEPTABLE");
}

void enviarTelemetria() {
    if (!comunica.webSocketConectado()) return;
    String msg = "{";
    msg += "\"temperatura\":" + String(ultimaTemperatura,2) + ",";
    msg += "\"setpoint\":" + String(setpoint,2) + ",";
    msg += "\"salida\":" + String(ultimaSalida,1) + ",";
    msg += "\"sensor\":\"" + sensorActualStr + "\",";
    msg += "\"actuador\":\"" + actuadorActualStr + "\",";
    msg += "\"nivelSeguro\":" + String(nivelSeguro ? "true" : "false") + ",";
    msg += "\"ts\":" + String(tiempoMuestreo,1);
    msg += "}";
    comunica.enviarMensaje(msg);
}

void alRecibirMensaje(String mensaje) {
    Serial.print("[WSS] ");
    Serial.println(mensaje);
    procesarComando(mensaje);
}