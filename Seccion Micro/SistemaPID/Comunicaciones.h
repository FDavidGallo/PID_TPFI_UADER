#ifndef COMUNICACIONES_H
#define COMUNICACIONES_H


#include <WebSocketsClient.h>
#include <functional>

typedef std::function<void(String)> MensajeCallback;

class Comunicaciones {
public:
    Comunicaciones();

    /**
     * Inicia la conectividad WiFi (usando WiFiManager con prioridad a Contrasenha.h)
     * y luego conecta el WebSocket seguro (WSS).
     * @param wsHost   Host del servidor WebSocket (ej. "ejemplo.com")
     * @param wsPort   Puerto (generalmente 443 para WSS)
     * @param wsPath   Ruta (ej. "/websocket")
     * @param timeoutSegundos Tiempo máximo para intentar conexión WiFi (por defecto 120s)
     */
    void begin(const char* wsHost, uint16_t wsPort, const char* wsPath = "/", int timeoutSegundos = 120);

    void loop();                          // Mantener comunicación WebSocket
    void enviarMensaje(String mensaje);
    void onMensajeRecibido(MensajeCallback callback);
    bool webSocketConectado();
    bool wifiConectado();

private:
    WebSocketsClient _webSocket;
    const char* _wsHost;
    uint16_t _wsPort;
    const char* _wsPath;
    MensajeCallback _mensajeCallback;
    bool _wsConectado;

    void _webSocketEvent(WStype_t type, uint8_t* payload, size_t length);
    void _conectarWiFiConWM(int timeoutSegundos);
};

#endif
