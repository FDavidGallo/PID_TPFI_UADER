#include "transistor.h"

// Constructor: almacena el período y configura el pin
Transistor::Transistor(int pin, int periodoMicros, String tipo)
  : actuador(periodoMicros, tipo), // la base de tiempo se guarda en la clase base (como int)
    _pin(pin),
    _periodo((unsigned long)periodoMicros)
{
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);   // estado inicial: apagado
}

// Método que genera un pulso con ciclo de trabajo = porcentaje
void Transistor::Aplicar(int porcentaje) {
  // Validar porcentaje
  if (porcentaje < 1) porcentaje = 1;
  if (porcentaje > 100) porcentaje = 100;

  unsigned long tiempoOn = (_periodo * porcentaje) / 100;
  unsigned long tiempoOff = _periodo - tiempoOn;

  // Encender transistor
  digitalWrite(_pin, HIGH);
  // Espera el tiempo de encendido (microsegundos o milisegundos según magnitud)
  if (tiempoOn > 0) {
    if (tiempoOn < 1000) {
      delayMicroseconds(tiempoOn);
    } else {
      delay(tiempoOn / 1000);
      delayMicroseconds(tiempoOn % 1000);
    }
  }

  // Apagar transistor
  digitalWrite(_pin, LOW);
  // Espera el tiempo de apagado
  if (tiempoOff > 0) {
    if (tiempoOff < 1000) {
      delayMicroseconds(tiempoOff);
    } else {
      delay(tiempoOff / 1000);
      delayMicroseconds(tiempoOff % 1000);
    }
  }
}