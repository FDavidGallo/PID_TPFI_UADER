#include "Comunicaciones.h"

#include <WiFiManager.h>          // Librería WiFiManager

// Intento de incluir archivo opcional de credenciales
#if __has_include("Contrasenha.h")
  #include "Contrasenha.h"
  #define TIENE_CONTRASENHA 1
#else
  #define TIENE_CONTRASENHA 0
#endif

Comunicaciones::Comunicaciones() {
    _wsConectado = false;
    _mensajeCallback = nullptr;
}

void Comunicaciones::begin(const char* wsHost, uint16_t wsPort, const char* wsPath, int timeoutSegundos) {
    _wsHost = wsHost;
    _wsPort = wsPort;
    _wsPath = wsPath;

    // 1. Conectar WiFi usando WiFiManager con prioridad a Contrasenha.h
    _conectarWiFiConWM(timeoutSegundos);

    // 2. Inicializar WebSocket seguro (WSS)
    _webSocket.beginSSL(_wsHost, _wsPort, _wsPath);
    _webSocket.onEvent([this](WStype_t type, uint8_t* payload, size_t length) {
        this->_webSocketEvent(type, payload, length);
    });
    _webSocket.setReconnectInterval(4000);
    Serial.println("[Comunicaciones] WebSocket inicializado (SSL)");
}

void Comunicaciones::_conectarWiFiConWM(int timeoutSegundos) {
    WiFi.mode(WIFI_STA);
    bool wifiOk = false;

    // Prioridad: si existe Contrasenha.h, intentar con esas credenciales primero
#if TIENE_CONTRASENHA
    Serial.println("[WiFi] Intentando con credenciales de Contrasenha.h");
    WiFi.begin(RED, contrasenha);
    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 20) { // 10 segundos máximo
        delay(500);
        Serial.print(".");
        intentos++;
    }
    if (WiFi.status() == WL_CONNECTED) {
        wifiOk = true;
        Serial.println("\n[WiFi] Conectado con credenciales predefinidas");
    } else {
        Serial.println("\n[WiFi] Falló conexión con credenciales predefinidas");
    }
#endif

    // Si no se pudo conectar, usar WiFiManager con portal cautivo
    if (!wifiOk) {
        Serial.println("[WiFi] Lanzando portal de configuración WiFiManager...");
        WiFiManager wm;
        wm.setConfigPortalTimeout(timeoutSegundos);
        wm.setConnectTimeout(50); // segundos intentando conectar
        // Nombre del AP y contraseña (puedes personalizarlos)
        bool res = wm.autoConnect("ControlESP", "control123");
        if (!res) {
            Serial.println("[WiFi] Error crítico: no se pudo conectar. Reiniciando...");
            delay(3000);
            ESP.restart();
        }
        Serial.println("[WiFi] Conectado mediante WiFiManager");
    }

    Serial.print("[WiFi] IP: ");
    Serial.println(WiFi.localIP());
}

void Comunicaciones::loop() {
    _webSocket.loop();
}

void Comunicaciones::enviarMensaje(String mensaje) {
    if (_wsConectado) {
        _webSocket.sendTXT(mensaje);
        Serial.println("[Comunicaciones] Mensaje enviado: " + mensaje);
    } else {
        Serial.println("[Comunicaciones] No se pudo enviar: WebSocket no conectado");
    }
}

void Comunicaciones::onMensajeRecibido(MensajeCallback callback) {
    _mensajeCallback = callback;
}

bool Comunicaciones::webSocketConectado() {
    return _wsConectado;
}

bool Comunicaciones::wifiConectado() {
    return WiFi.status() == WL_CONNECTED;
}

void Comunicaciones::_webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.println("[WebSocket] Desconectado");
            _wsConectado = false;
            break;
        case WStype_CONNECTED:
            Serial.println("[WebSocket] Conectado al servidor (Secure)");
            _wsConectado = true;
            break;
        case WStype_TEXT:
            {
                String mensaje = String((char*)payload);
                Serial.printf("[WebSocket] Mensaje recibido: %s\n", payload);
                if (_mensajeCallback != nullptr) {
                    _mensajeCallback(mensaje);
                }
            }
            break;
        case WStype_ERROR:
            Serial.println("[WebSocket] Error de conexión SSL");
            _wsConectado = false;
            break;
        default:
            break;
    }
}