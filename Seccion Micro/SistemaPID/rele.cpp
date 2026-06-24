#include "rele.h"

Rele::Rele(int pin, int periodoMs, String tipo)
    : actuador(periodoMs, tipo), _pin(pin), _periodoMs(periodoMs),
      _tiempoOn(0), _ultimoCambio(0), _estadoActual(false), _porcentajeActual(0) {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, LOW);
}

void Rele::Aplicar(int porcentaje) {
    porcentaje = constrain(porcentaje, 0, 100);
    _porcentajeActual = porcentaje;
    _tiempoOn = (unsigned long)porcentaje * _periodoMs / 100;
    _ultimoCambio = millis();
    _estadoActual = false;
    digitalWrite(_pin, LOW);
}

void Rele::loop() {
    unsigned long ahora = millis();
    unsigned long tiempoEnCiclo = (ahora - _ultimoCambio) % _periodoMs;
    bool deberiaEncender = (tiempoEnCiclo < _tiempoOn);
    if (deberiaEncender != _estadoActual) {
        _estadoActual = deberiaEncender;
        digitalWrite(_pin, _estadoActual ? HIGH : LOW);
    }
}

void Rele::setPeriodo(int nuevoPeriodoMs) {
    if (nuevoPeriodoMs > 0) {
        _periodoMs = nuevoPeriodoMs;
        // Recalcular _tiempoOn en función del porcentaje actual
        _tiempoOn = (unsigned long)_porcentajeActual * _periodoMs / 100;
        // Reiniciar el ciclo para que empiece desde el principio
        _ultimoCambio = millis();
        _estadoActual = false;
        digitalWrite(_pin, LOW);
    }
}