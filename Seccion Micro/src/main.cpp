#include <Arduino.h>
#include "Comunicaciones.h"

// Datos del servidor WebSocket
const char* WS_HOST = "192.168.1.155";   // Cambia por la IP de tu servidor
const uint16_t WS_PORT = 1880;
const char* WS_PATH = "/pepe";

Comunicaciones comunica;

// Variables para controlar la secuencia de números
bool secuenciaIniciada = false;
int numeroActual = 1;
unsigned long tiempoUltimoEnvio = 0;
const unsigned long INTERVALO_ENVIO = 1000; // 1 segundo

void alRecibirMensaje(String mensaje) {
    Serial.print("[main] Mensaje recibido del servidor: ");
    Serial.println(mensaje);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\nIniciando aplicación...");
    
    // Conexión automática (EEPROM -> archivo -> manual)
    comunica.ConexionAutomatica(WS_HOST, WS_PORT, WS_PATH);
    
    // Callback para mensajes entrantes (opcional)
    comunica.onMensajeRecibido(alRecibirMensaje);
    
    // No iniciamos la secuencia todavía; esperamos a que el WebSocket esté conectado
}

void loop() {
    // Mantener viva la conexión WebSocket (obligatorio)
    comunica.loop();
    
    // Comprobar si el WebSocket ya está conectado y la secuencia no ha comenzado
    if (!secuenciaIniciada && comunica.webSocketConectado()) {
        Serial.println("[main] WebSocket conectado. Iniciando secuencia 1..14 en 1 segundo...");
        secuenciaIniciada = true;
        tiempoUltimoEnvio = millis();   // Para que empiece ya
        numeroActual = 1;
    }
    
    // Si la secuencia está activa y ha pasado el intervalo, enviar el siguiente número
    if (secuenciaIniciada && (millis() - tiempoUltimoEnvio >= INTERVALO_ENVIO)) {
        if (numeroActual <= 14) {
            // Enviar el número como mensaje de texto
            String mensaje = String(numeroActual);
            comunica.enviarMensaje(mensaje);
            Serial.print("[main] Enviado: ");
            Serial.println(mensaje);
            
            numeroActual++;
            tiempoUltimoEnvio = millis();
        } else {
            // Secuencia terminada
            Serial.println("[main] Secuencia completada.");
            secuenciaIniciada = false; // o podrías dejarla en true para que no se repita
        }
    }
    
    // Pequeña pausa para no saturar el CPU (opcional)
    delay(40);
}