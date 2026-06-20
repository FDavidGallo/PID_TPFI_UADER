#include "transistor.h"

Transistor::Transistor(int pin, int frecuencia, String tipo)
    : actuador(0, tipo), _pin(pin), _frecuencia(frecuencia) {
    ledcAttach(_pin, _frecuencia, 8); // resolución 8 bits (0-255)
    ledcWrite(_pin, 0);
}

void Transistor::Aplicar(int porcentaje) {
    porcentaje = constrain(porcentaje, 0, 100);
    int duty = map(porcentaje, 0, 100, 0, 255);
    ledcWrite(_pin, duty);
}

void Transistor::setFrecuencia(int nuevaFrecuencia) {
    if (nuevaFrecuencia > 0) {
        _frecuencia = nuevaFrecuencia;
        // Reconfigurar el canal PWM
        ledcDetach(_pin);
        ledcAttach(_pin, _frecuencia, 8);
        // Mantener el duty según el último porcentaje aplicado (se pierde, pero se puede guardar)
        // Para mantener el estado, necesitaríamos guardar el último duty.
        // Por simplicidad, ponemos a 0 y el siguiente Aplicar() lo pondrá bien.
        ledcWrite(_pin, 0);
    }
}