#include "Comunicaciones.h"
#include "PotenciometroTemp.h"
#include "Termocupla.h"
#include <WiFi.h>
#include <WebSocketsClient.h>
#include "SensorDeTemperatura.h"

// Datos del servidor WebSocket SEGURO (WSS)
// 1. SOLO el dominio (sin https:// y sin rutas)
const char* WS_HOST = "trabajofinalintegradoruader.hostlocal.app"; 

// 2. El puerto donde corre tu servicio WSS (Si es Node-RED con SSL suele ser 1880 o 443)
const uint16_t WS_PORT = 443; 

// 3. La ruta específica del endpoint (Ajusta según tu servidor: /pepe o /wss)
const char* WS_PATH = "/pepe";
Comunicaciones comunica;

bool secuenciaIniciada = false;
int numeroActual = 1;
unsigned long tiempoUltimoEnvio = 0;
const unsigned long INTERVALO_ENVIO = 1000;
/////////////////////////////////////////////////////////////////////////
#include <max6675.h>


// Definición de pines (Ajustar según tu cableado)
const int thermoSO = 19;
const int thermoCS = 23;
const int thermoSCK = 5;

// Instancia del sensor
MAX6675 thermocouple(thermoSCK, thermoCS, thermoSO);

/**
 * Lee la temperatura del sensor MAX6675.
 * @return float con la temperatura en grados Celsius.
 */
float leerTemperatura() {
    float temp = thermocouple.readCelsius();
    
    // Validación básica de lectura
    if (isnan(temp)) {
        return -1.0; // Error de lectura o sensor desconectado
    }
    
    return temp;
}
/////////////////////////////////////////////////////////////////////////
void alRecibirMensaje(String mensaje) {
    Serial.print("[main] Mensaje recibido: ");
    Serial.println(mensaje);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\nIniciando aplicación WSS...");
    
    comunica.ConexionAutomatica(WS_HOST, WS_PORT, WS_PATH);
    comunica.onMensajeRecibido(alRecibirMensaje);
    ///////////////////////////////////////////

    ///////////////////////////////////////////
}

void loop() {
    comunica.loop();
    
    if (!secuenciaIniciada && comunica.webSocketConectado()) {
        Serial.println("[main] WSS Conectado. Iniciando secuencia...");
        secuenciaIniciada = true;
        tiempoUltimoEnvio = millis();
        numeroActual = 1;
    }
    
    if (secuenciaIniciada && (millis() - tiempoUltimoEnvio >= INTERVALO_ENVIO)) {
        if (numeroActual <= 14) {
            String mensaje = String(numeroActual);
            comunica.enviarMensaje(mensaje);
            numeroActual++;
            tiempoUltimoEnvio = millis();
        } else {
            Serial.println("[main] Secuencia completada.");
            secuenciaIniciada = false;
        }
    }
    float t = leerTemperatura();
    Serial.print("Temperatura: ");
    Serial.print(t);
    Serial.println(" C");

    // El MAX6675 necesita al menos 250ms entre lecturas
    delay(1000);
    delay(150);
}