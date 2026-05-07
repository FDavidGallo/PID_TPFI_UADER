#ifndef ACTUADOR_H
#define ACTUADOR_H

#include <Arduino.h>

class actuador {
  private:
    int _baseDeTiempo;
    String _tipo;

  public:
    // Constructor: recibe el tiempo base y el tipo de actuador
    actuador(int BaseDeTiempo, String Tipo) {
      _baseDeTiempo = BaseDeTiempo;
      _tipo = Tipo;
    }

    // Método virtual que recibe un valor entre 1 y 100
    // Por ahora es virtual puro o con implementación vacía (según se prefiera)
    virtual void Aplicar(int valor) {
      // Por el momento no hace nada. Será sobrescrito en clases derivadas.
      // Se puede agregar validación del rango si se desea.
      if (valor < 1 || valor > 100) {
         Serial.println("ERROR: Valor fuera de rango");
        return;
      }
      // Implementación base vacía
    }

    // Métodos para acceder a los atributos (opcionales)
    int getBaseDeTiempo() { return _baseDeTiempo; }
    String getTipo() { return _tipo; }
};
#endif // TERMOCUPLA_H