#ifndef POTENCIOMETRO_TEMP_H
#define POTENCIOMETRO_TEMP_H

#include "SensorDeTemperatura.h"
#include <Arduino.h>

class PotenciometroTemp : public SensorDeTemperatura {
private:
    uint8_t pinADC;           // Pin analógico donde está conectado el potenciómetro
    float tempMin;            // Temperatura mínima que representará el valor 0 del ADC
    float tempMax;            // Temperatura máxima que representará el valor máximo del ADC
    float vRef;               // Voltaje de referencia del ADC (ej. 3.3V o 5V)
    int resolucionADC;        // Resolución del ADC (4096 para ESP32, 1024 para Arduino Uno)

public:
    /**
     * Constructor.
     * @param adcPin       Pin analógico (ej. A0, 34 en ESP32)
     * @param minima       Temperatura mínima del rango (cuando el potenciómetro da el valor mínimo)
     * @param maxima       Temperatura máxima del rango (cuando el potenciómetro da el valor máximo)
     * @param voltajeRef   Voltaje de referencia del ADC (por defecto 3.3V)
     * @param bitsADC      Resolución del ADC (por defecto 4096 para ESP32)
     */
    PotenciometroTemp(uint8_t adcPin, float minima, float maxima,
                      float voltajeRef = 3.3, int bitsADC = 4096);

    // Implementación de métodos virtuales puros
    void Inciar() override;
    float LeerTemperatura() override;
    bool EstaAndado() override;
    String ObtenerTipoDeSensor() override;
};

#endif // POTENCIOMETRO_TEMP_H