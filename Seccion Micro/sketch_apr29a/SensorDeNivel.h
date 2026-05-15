#ifndef SENSORDENIVEL_H
#define SENSORDENIVEL_H

#include <Arduino.h>

class SensorDeNivel {
private:
    int _minRaw, _maxRaw;   // umbrales en unidades del ADC (0-4095)
    uint8_t _pin;
    bool _inicializado;

public:
    SensorDeNivel(int minRaw, int maxRaw, uint8_t pin);
    static SensorDeNivel* crear(int minRaw, int maxRaw, uint8_t pin);
    void Iniciar();
    int ChequearNivel();   // retorna: 0=ok, 1=alto, -1=bajo, -2=error
};

#endif