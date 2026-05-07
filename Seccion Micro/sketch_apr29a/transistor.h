#ifndef TRANSISTOR_H
#define TRANSISTOR_H

#include <Arduino.h>
#include "actuador.h"

class Transistor : public actuador {
  private:
    int _pin;                // Pin de salida conectado a la base/puerta del transistor
    unsigned long _periodo;  // Período en microsegundos (inverso de la frecuencia)

  public:
    // Constructor: recibe pin, período en microsegundos (BaseDeTiempo) y tipo descriptivo
    Transistor(int pin, int periodoMicros, String tipo);

    // Aplica un ciclo de trabajo (1-100) durante un período completo
    void Aplicar(int porcentaje) override;
};

#endif // TRANSISTOR_H