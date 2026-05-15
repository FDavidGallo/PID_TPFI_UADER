#include "SensorDeNivel.h"

SensorDeNivel::SensorDeNivel(int minRaw, int maxRaw, uint8_t pin)
    : _minRaw(minRaw), _maxRaw(maxRaw), _pin(pin), _inicializado(false) {}

SensorDeNivel* SensorDeNivel::crear(int minRaw, int maxRaw, uint8_t pin) {
    return new SensorDeNivel(minRaw, maxRaw, pin);
}

void SensorDeNivel::Iniciar() {
    pinMode(_pin, INPUT);
    analogSetAttenuation(ADC_11db);
    delay(100);
    int test = analogRead(_pin);
    if (test < 0 || test > 4095) {
        _inicializado = false;
        Serial.println("Sensor de nivel: error de inicialización");
    } else {
        _inicializado = true;
        Serial.println("Sensor de nivel listo");
    }
}

int SensorDeNivel::ChequearNivel() {
    if (!_inicializado) return -2;
    int raw = analogRead(_pin);
    // Serial.println(raw);
    if (raw < _minRaw) return -1;
    if (raw > _maxRaw) return 1;
    return 0;
}