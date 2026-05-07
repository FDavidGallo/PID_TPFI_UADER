#include "rele.h"

// Constructor
Rele::Rele(int pin, int BaseDeTiempo, String Tipo)
  : actuador(BaseDeTiempo, Tipo) {
  _pin = pin;
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, LOW);   // Inicialmente apagado
}

// Método Aplicar
void Rele::Aplicar(int porcentaje) {
  // Validar porcentaje (1 a 100)
  if (porcentaje < 1) porcentaje = 1;
  if (porcentaje > 100) porcentaje = 100;

  int baseTiempo = getBaseDeTiempo();  // en milisegundos
  unsigned long tiempoOn = (unsigned long)porcentaje * baseTiempo / 100;
  unsigned long tiempoOff = baseTiempo - tiempoOn;

  // Lógica de encendido/apagado dentro de la base de tiempo
  digitalWrite(_pin, HIGH);   // Enciende el relé
  delay(tiempoOn);            // Permanece encendido
  digitalWrite(_pin, LOW);    // Apaga el relé
  delay(tiempoOff);           // Permanece apagado el resto
}