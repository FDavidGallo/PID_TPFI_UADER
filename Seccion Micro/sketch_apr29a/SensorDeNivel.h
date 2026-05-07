#ifndef SENSORDENIVEL_H
#define SENSORDENIVEL_H

#include <Arduino.h>

class SensorDeNivel {
private:
    // -------------------------------------------------------
    // Atributos privados
    // -------------------------------------------------------
    int _minmV;            // Tensión mínima normal (en mV, ej: 1750)
    int _maxmV;            // Tensión máxima normal (en mV, ej: 2700)
    uint8_t _pin;          // Número de pin analógico (constante de Arduino)
    bool _valido;          // Indica si el objeto se creó correctamente

    /**
     * @brief Convierte una cadena como "A0" al pin correspondiente.
     *        Si el pin no es válido devuelve false y deja _pin sin tocar.
     */
    bool _parsePin(const char* pinStr);

    /**
     * @brief Lee el pin analógico y devuelve la tensión en milivoltios.
     *        Supone que _pin ya está correctamente configurado.
     */
    int _leerMilivoltios();

    // -------------------------------------------------------
    // Constructor privado → solo accesible desde el factory.
    // -------------------------------------------------------
    SensorDeNivel(int minmV, int maxmV, const char* pin);

public:
    // -------------------------------------------------------
    // Factory público estático
    // -------------------------------------------------------
    /**
     * @brief Método de fábrica que crea una instancia de SensorDeNivel
     *        solo si el pin es válido.
     *
     * @param minmV  Tensión mínima normal (en mV)
     * @param maxmV  Tensión máxima normal (en mV)
     * @param pin    Identificador del pin analógico (ej: "A0")
     * @return       Puntero a SensorDeNivel si todo OK, nullptr si el pin es inválido.
     */
    static SensorDeNivel* crear(int minmV, int maxmV, const char* pin);

    // -------------------------------------------------------
    // Destructor (necesario para borrar objetos dinámicos)
    // -------------------------------------------------------
    ~SensorDeNivel() {
        // Si necesitáramos liberar recursos, acá lo haríamos.
    }

    // -------------------------------------------------------
    // Métodos públicos principales
    // -------------------------------------------------------

    /**
     * @brief Verifica que el sensor tenga valores crudos razonables
     *        (no esté en corto ni desconectado).
     * @return true si raw está entre 2 y 1020, false en caso contrario.
     */
    bool comprobarEstado();

    /**
     * @brief Realiza dos lecturas de prueba y las imprime por Serial.
     *        Usado para confirmar que el sensor está funcionando.
     */
    void Iniciar();

    /**
     * @brief Evalúa el nivel de agua respecto a _minmV y _maxmV.
     *
     * @return -1 → nivel bajo (mV < _minmV)
     *          0 → nivel normal (dentro del rango)
     *          1 → nivel alto/rebalse (mV > _maxmV)
     *         -2 → objeto inválido (nunca debería ocurrir si se usa crear)
     */
    int ChequearNivel();
};

#endif