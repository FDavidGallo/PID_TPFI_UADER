#ifndef ACTUADOR_H
#define ACTUADOR_H
#include <Arduino.h>

class actuador {
  private:
    int _baseDeTiempo;
    String _tipo;

  public:
    actuador(int BaseDeTiempo, String Tipo) {
      _baseDeTiempo = BaseDeTiempo;
      _tipo = Tipo;
    }

    virtual void Aplicar(int valor) {
      if (valor < 0 || valor > 100) {
         Serial.println("ERROR: Valor fuera de rango");
        return;
      }
    }

    int getBaseDeTiempo() { return _baseDeTiempo; }
    String getTipo() { return _tipo; }
};
#endif