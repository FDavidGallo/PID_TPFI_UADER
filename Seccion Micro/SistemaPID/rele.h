#ifndef RELE_H
#define RELE_H

#include "actuador.h"
#include <Arduino.h>

class Rele : public actuador {
private:
    int _pin;
    unsigned long _periodoMs;
    unsigned long _tiempoOn;
    unsigned long _ultimoCambio;
    bool _estadoActual;
    int _porcentajeActual;   // último porcentaje aplicado

public:
    Rele(int pin, int periodoMs, String tipo = "rele");
    void Aplicar(int porcentaje) override;
    void loop();
    // Nuevo método para cambiar el período sin destruir el objeto
    void setPeriodo(int nuevoPeriodoMs);
};

#endif