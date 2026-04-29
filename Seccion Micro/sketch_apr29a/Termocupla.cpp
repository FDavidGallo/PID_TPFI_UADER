#include "Termocupla.h"

Termocupla::Termocupla(int soPin, int csPin, int sckPin)
    : pinSO(soPin), pinCS(csPin), pinSCK(sckPin),
      thermocouple(sckPin, csPin, soPin)   // orden esperado por la librería: SCK, CS, SO
{
    // El constructor solo inicializa los pines y el objeto MAX6675.
}

void Termocupla::Inciar() {
    // El MAX6675 no requiere inicialización adicional.
    // Se añade una pequeña pausa y unas lecturas dummy para estabilizar.
    delay(50);
    thermocouple.readCelsius();
     delay(125);
    thermocouple.readCelsius();
}

float Termocupla::LeerTemperatura() {
    float temp = thermocouple.readCelsius();
    // Si la lectura no es un número válido, se retorna -999.0 (error)
    if (isnan(temp)) {
        return -999.0;
    }
    return temp;
}

bool Termocupla::EstaAndado() {
     float temp = thermocouple.readCelsius();
    // Si la lectura no es un número válido, se retorna -999.0 (error)
    if (isnan(temp)) {
        return -999.0;
    }
    if(temp>= -20.0 && temp <= 1200.0) {
        return true;
    } else {
        return false;
    }   
}

String Termocupla::ObtenerTipoDeSensor() {
    return "Es una termocupla tipo k con un MAX6675";
}