// ===============================================================================
// SensorDeTemperatura.h
// Clase abstracta (interfaz) para cualquier sensor de temperatura.
// No se necesita archivo .cpp porque todos los métodos son virtuales puros
// es decir que dicen que hacer, no como hacerlo. Las clases hijas implementarán
// la lógica espeficada acá. Esto lo hacemos, principalmente, para poder usar 
// polimorfismo y así manejar diferentes tipos de sensores con el mismo código. 
// ===============================================================================

#ifndef SENSOR_DE_TEMPERATURA_H
#define SENSOR_DE_TEMPERATURA_H

#include <Arduino.h>

/**
 * Clase abstracta que define el CONTRATO que deben cumplir TODOS los sensores
 * de temperatura. Cualquier clase que herede de ella está OBLIGADA a
 * implementar los métodos marcados con "= 0".
 *
 * No se puede crear un objeto de esta clase directamente (es como un molde ).
 * Solo se pueden crear objetos de las clases hijas (ej. SensorDHT11, termocupla, etc.).
 */
class SensorDeTemperatura {
public:
    /**
     * Destructor virtual.
     * - "virtual" permite que al destruir un objeto mediante un puntero a
     *   esta clase, se llame al destructor correcto de la clase hija.
     * - "= default" le pide al compilador que genere automáticamente el código
     *   (vacío, suficiente para esta interfaz).
     */
    virtual ~SensorDeTemperatura() = default;

    /**
     * Inicializa el sensor (configura pines, inicia comunicación, etc.).
     * Debe llamarse una vez en el setup() del programa.
     * Es un método VIRTUAL PURO ("= 0") → las clases hijas SÍ o SÍ deben
     * implementarlo con su propia lógica.
     */
    virtual void Inciar() = 0;

    /**
     * Lee la temperatura actual en grados Celsius.
     * Retorna un float con la temperatura. Si hay error, puede retornar NaN
     * (Not a Number) o un valor especial como -273.15.
     * Método virtual puro → cada sensor implementa su forma de leer.
     */
    virtual float LeerTemperatura() = 0;

    /**
     * Verifica si el sensor está conectado y responde correctamente.
     * Retorna true si todo está bien, false si hay problema (cable suelto,
     * sensor roto, etc.).
     * Método virtual puro → cada sensor implementa su propia verificación.
     */
    virtual bool EstaAndado() = 0;

    /**
     * Devuelve un String que identifica el tipo de sensor.
     * Ejemplos: "DHT11", "DS18B20", "LM35".
     * Útil para mostrar en un monitor serial o para diagnósticos.
     * Método virtual puro → cada sensor dice su nombre.
     */
    virtual String ObtenerTipoDeSensor() = 0;
 
};

#endif // SENSOR_DE_TEMPERATURA_H