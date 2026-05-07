#ifndef RELE_H
#define RELE_H

#include <Arduino.h>
#include "actuador.h"   // Clase base actuador

class Rele : public actuador {
  private:
    int _pin;   // Pin digital donde está conectado el relé

  public:
    // Constructor: recibe pin, base de tiempo (ms) y tipo (ej. "SSR")
    Rele(int pin, int BaseDeTiempo, String Tipo);

    // Sobrescritura del método Aplicar
    void Aplicar(int porcentaje) override;
};

#endif // RELE_H