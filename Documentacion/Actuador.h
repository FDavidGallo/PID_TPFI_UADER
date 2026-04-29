#ifndef COMUNICACIONES_H
#define COMUNICACIONES_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <functional>

typedef std::function<void(String)> MensajeCallback;

class Comunicaciones {
public:
    Comunicaciones();
    
    void configurar(const char* ssid, const char* password,
                    const char* wsHost, uint16_t wsPort, const char* wsPath = "/");
    void ConexionAutomatica(const char* wsHost, uint16_t wsPort, const char* wsPath = "/");
    void conectar();
    void loop();
    void enviarMensaje(String mensaje);
    void onMensajeRecibido(MensajeCallback callback);
    bool wifiConectado();
    bool webSocketConectado();

private:
    WebSocketsClient _webSocket;
    const char* _ssid;
    const char* _password;
    const char* _wsHost;
    uint16_t _wsPort;
    const char* _wsPath;
    MensajeCallback _mensajeCallback;
    bool _wsConectado;
    
    void _conectarWiFi();
    void _webSocketEvent(WStype_t type, uint8_t* payload, size_t length);
    void IngresarRedWifiManualmente();
    
    String _manualSSID;
    String _manualPassword;
};

#endif