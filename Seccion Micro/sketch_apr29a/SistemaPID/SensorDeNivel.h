#ifndef SENSORDENIVEL_H
#define SENSORDENIVEL_H



class SensorDeNivel {
private:
    int _minRaw, _maxRaw;   // umbrales en unidades del ADC (0-4095)
    int _pin;
    bool _inicializado;

public:
    SensorDeNivel(int minRaw, int maxRaw, int pin);
    static SensorDeNivel* crear(int minRaw, int maxRaw, int pin);
    void Iniciar();
    int ChequearNivel();   // retorna: 0=ok, 1=alto, -1=bajo, -2=error
};

#endif
