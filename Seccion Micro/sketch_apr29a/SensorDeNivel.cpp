#include "SensorDeNivel.h"

// =============================================================
// Método PRIVADO: parseo del pin analógico
// =============================================================
bool SensorDeNivel::_parsePin(const char* pinStr) {
    // Si la cadena coincide con un pin válido, asignamos la constante
    if (strcmp(pinStr, "A0") == 0)      { _pin = A0; return true; }
    if (strcmp(pinStr, "A1") == 0)      { _pin = A1; return true; }
    if (strcmp(pinStr, "A2") == 0)      { _pin = A2; return true; }
    if (strcmp(pinStr, "A3") == 0)      { _pin = A3; return true; }
    if (strcmp(pinStr, "A4") == 0)      { _pin = A4; return true; }
    if (strcmp(pinStr, "A5") == 0)      { _pin = A5; return true; }
   

    // Si llegamos acá, el pin no es válido
    return false;
}

// =============================================================
// Constructor PRIVADO
// =============================================================
SensorDeNivel::SensorDeNivel(int minmV, int maxmV, const char* pin)
    : _minmV(minmV), _maxmV(maxmV), _valido(false) {

    // Intentamos asignar el pin
    _valido = _parsePin(pin);

    // Si el pin es inválido, el objeto queda marcado como no válido.
    // El factory se encargará de destruirlo si es necesario.
}

// =============================================================
// Método PRIVADO: lectura de tensión en milivoltios
// =============================================================
int SensorDeNivel::_leerMilivoltios() {
    int raw = analogRead(_pin);
    // Conversión: 0–1023 → 0–5000 mV
    return (long)raw * 5000 / 1023;
}

// =============================================================
// FACTORY PÚBLICO ESTÁTICO
// =============================================================
SensorDeNivel* SensorDeNivel::crear(int minmV, int maxmV, const char* pin) {
    // Pedimos memoria dinámica para un nuevo objeto
    SensorDeNivel* obj = new SensorDeNivel(minmV, maxmV, pin);

    // Verificamos si el objeto se creó de forma válida
    if (!obj->_valido) {
        // PIN INVÁLIDO → el objeto se AUTODESTRUYE
        Serial.println(F("Error: Pin inválido. El objeto será destruido."));
        delete obj;      // liberamos la memoria
        return nullptr;  // devolvemos nulo para que el usuario lo detecte
    }

    // Si el pin era correcto, devolvemos el puntero listo para usar
    return obj;
}

// =============================================================
// Método público: comprobarEstado()
// =============================================================
bool SensorDeNivel::comprobarEstado() {
    // Aunque el objeto ya debería ser válido, chequeamos igual por seguridad
    if (!_valido) {
        Serial.println(F("SensorDeNivel inválido."));
        return false;
    }

    int raw = analogRead(_pin);
    // El sensor debe estar entre 2 y 1020 en crudo
    return (raw >= 2 && raw <= 1020);
}

// =============================================================
// Método público: Iniciar()
// =============================================================
void SensorDeNivel::Iniciar() {
    if (!_valido) {
        Serial.println(F("Sensor no inicializado (objeto inválido)."));
        return;
    }

    Serial.println(F("Inicializando sensor de nivel..."));
    delay(300);

    // Primera lectura de prueba
    int raw1 = analogRead(_pin);
    float vol1 = (raw1 * 5.0) / 1023.0;
    Serial.print(F("Lectura 1 - RAW: "));
    Serial.print(raw1);
    Serial.print(F(" | Voltaje: "));
    Serial.print(vol1);
    Serial.println(F(" V"));

    delay(500);

    // Segunda lectura de prueba
    int raw2 = analogRead(_pin);
    float vol2 = (raw2 * 5.0) / 1023.0;
    Serial.print(F("Lectura 2 - RAW: "));
    Serial.print(raw2);
    Serial.print(F(" | Voltaje: "));
    Serial.print(vol2);
    Serial.println(F(" V"));
    Serial.println(F("Sensor listo."));
}

// =============================================================
// Método público: ChequearNivel()
// =============================================================
int SensorDeNivel::ChequearNivel() {
    if (!_valido) {
        Serial.println(F("SensorDeNivel inválido."));
        return -2;  // código de error especial
    }

    int mv = _leerMilivoltios();

    if (mv < _minmV) {
        Serial.println(F("¡Nivel bajo!"));
        return -1;
    }
    else if (mv > _maxmV) {
        Serial.println(F("¡se rebalsa !"));
        return 1;
    }
    else {
        Serial.println(F("Todo OK"));
        return 0;
    }
}