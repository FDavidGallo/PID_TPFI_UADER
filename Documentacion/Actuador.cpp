#include "Comunicaciones.h"

// Intento de incluir el archivo opcional Contrasenha.h
#if __has_include("Contrasenha.h")
  #include "Contrasenha.h"
  #define TIENE_CONTRASENHA 1
#else
  #define TIENE_CONTRASENHA 0
  #define RED nullptr
  #define contrasenha nullptr
#endif

Comunicaciones::Comunicaciones() {
    _wsConectado = false;
    _mensajeCallback = nullptr;
    _ssid = nullptr;
    _password = nullptr;
}

void Comunicaciones::configurar(const char* ssid, const char* password,
                                const char* wsHost, uint16_t wsPort, const char* wsPath) {
    _ssid = ssid;
    _password = password;
    _wsHost = wsHost;
    _wsPort = wsPort;
    _wsPath = wsPath;
}

void Comunicaciones::ConexionAutomatica(const char* wsHost, uint16_t wsPort, const char* wsPath) {
    _wsHost = wsHost;
    _wsPort = wsPort;
    _wsPath = wsPath;
    
    bool credencialesExternas = false;
    
#if TIENE_CONTRASENHA
    if (RED != nullptr && contrasenha != nullptr) {
        Serial.println("[Auto] Usando credenciales desde Contrasenha.h");
        _ssid = RED;
        _password = contrasenha;
        credencialesExternas = true;
    }
#endif
    
    if (!credencialesExternas) {
        Serial.println("[Auto] No se encontró Contrasenha.h o sus macros son inválidas. Modo manual.");
        IngresarRedWifiManualmente();
        _ssid = _manualSSID.c_str();
        _password = _manualPassword.c_str();
    }
    
    _conectarWiFi();
    
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Auto] Falló la conexión con las credenciales actuales.");
        Serial.println("Solicitando nuevas credenciales manualmente...");
        IngresarRedWifiManualmente();
        _ssid = _manualSSID.c_str();
        _password = _manualPassword.c_str();
        _conectarWiFi();
    }
    
    // --- CAMBIO A WSS: Se usa beginSSL en lugar de begin ---
    _webSocket.beginSSL(_wsHost, _wsPort, _wsPath);
    _webSocket.onEvent([this](WStype_t type, uint8_t* payload, size_t length) {
        this->_webSocketEvent(type, payload, length);
    });
    _webSocket.setReconnectInterval(5000);
}

void Comunicaciones::IngresarRedWifiManualmente() {
    Serial.println("\n=== Modo de ingreso manual de WiFi ===");
    Serial.println("Ingrese el nombre de la red (SSID):");
    
    _manualSSID = "";
    while (_manualSSID.length() == 0) {
        if (Serial.available()) {
            _manualSSID = Serial.readStringUntil('\n');
            _manualSSID.trim();
        }
        delay(100);
    }
    
    Serial.println("Ingrese la contraseña:");
    _manualPassword = "";
    while (_manualPassword.length() == 0) {
        if (Serial.available()) {
            _manualPassword = Serial.readStringUntil('\n');
            _manualPassword.trim();
        }
        delay(100);
    }
}

void Comunicaciones::conectar() {
    _conectarWiFi();
    // --- CAMBIO A WSS: Se usa beginSSL en lugar de begin ---
    _webSocket.beginSSL(_wsHost, _wsPort, _wsPath);
    _webSocket.onEvent([this](WStype_t type, uint8_t* payload, size_t length) {
        this->_webSocketEvent(type, payload, length);
    });
    _webSocket.setReconnectInterval(5000);
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

bool Comunicaciones::wifiConectado() {
    return WiFi.status() == WL_CONNECTED;
}

bool Comunicaciones::webSocketConectado() {
    return _wsConectado;
}

void Comunicaciones::_conectarWiFi() {
    if (_ssid == nullptr || _password == nullptr) {
        Serial.println("[WiFi] Error: credenciales no configuradas");
        return;
    }
    
    Serial.print("[WiFi] Conectando a ");
    Serial.println(_ssid);
    WiFi.begin(_ssid, _password);
    
    int intentos = 0;
    while (WiFi.status() != WL_CONNECTED && intentos < 20) {
        delay(500);
        Serial.print(".");
        intentos++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n[WiFi] Conectado!");
        Serial.print("[WiFi] IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n[WiFi] Falló la conexión");
    }
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