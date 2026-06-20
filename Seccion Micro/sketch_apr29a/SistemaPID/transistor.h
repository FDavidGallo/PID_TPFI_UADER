#ifndef TRANSISTOR_H
#define TRANSISTOR_H

#include "actuador.h"
#include <Arduino.h>

class Transistor : public actuador {
private:
    int _pin;
    int _frecuencia;

public:
    Transistor(int pin, int frecuencia = 1000, String tipo = "transistor");
    void Aplicar(int porcentaje) override;
    // Nuevo método para cambiar la frecuencia sin destruir el objeto, porque no le dá la memoria ram sino
    void setFrecuencia(int nuevaFrecuencia);
};

#endif