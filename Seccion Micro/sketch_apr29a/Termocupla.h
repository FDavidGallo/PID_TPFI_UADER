#ifndef TERMOCUPLA_H
#define TERMOCUPLA_H

#include "SensorDeTemperatura.h"
#include <max6675.h>

class Termocupla : public SensorDeTemperatura {
private:
    int pinSO;
    int pinCS;
    int pinSCK;
    MAX6675 thermocouple;   // Objeto del sensor

public:
    /**
     * Constructor.
     * @param soPin  Pin SO (MISO) del MAX6675
     * @param csPin  Pin CS (Chip Select)
     * @param sckPin Pin SCK (Clock)
     */
    Termocupla(int soPin, int csPin, int sckPin);

    // Implementación de los métodos virtuales puros
    void Inciar() override;
    float LeerTemperatura() override;
    bool EstaAndado() override;
    String ObtenerTipoDeSensor() override;
};

#endif // TERMOCUPLA_H