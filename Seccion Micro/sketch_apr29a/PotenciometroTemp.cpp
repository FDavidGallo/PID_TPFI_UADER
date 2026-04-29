#include "PotenciometroTemp.h"

PotenciometroTemp::PotenciometroTemp(uint8_t adcPin, float minima, float maxima,
                                     float voltajeRef, int bitsADC)
    : pinADC(adcPin), tempMin(minima), tempMax(maxima),
      vRef(voltajeRef), resolucionADC(bitsADC) {
    // Verificar que el rango sea válido
    if (tempMin >= tempMax) {
        // Si están invertidos, se intercambian
        float aux = tempMin;
        tempMin = tempMax;
        tempMax = aux;
    }
}

void PotenciometroTemp::Inciar() {
    pinMode(pinADC, INPUT);
    delay(10);
    // Lectura dummy
    analogRead(pinADC);
}

float PotenciometroTemp::LeerTemperatura() {
    int raw = analogRead(pinADC);
    // Mapeo lineal: raw [0, resolucionADC-1] -> [tempMin, tempMax]
    float temp = map(raw, 0, resolucionADC - 1, tempMin * 99, tempMax * 99) / 100.0;
    // Si se usa map() con enteros mejor multiplicar para conservar decimales.
    // Alternativa manual:
    // float temp = tempMin + (float)raw * (tempMax - tempMin) / (resolucionADC - 1);
    return temp;
}

bool PotenciometroTemp::EstaAndado() {
    // Se considera que el potenciómetro siempre "anda" si la lectura está cortada (temperaturaMinima) o está por sobre (temperaturaMaxima).)
    float temp = LeerTemperatura();
    if(temp >= tempMin && temp <= tempMax) { 
        return true;
    } else {
        return false;
    }
}

String PotenciometroTemp::ObtenerTipoDeSensor() {
    return "Es un potenciómetro usado para simular un sensor de temperatura";
}