#ifndef CONTROL_H
#define CONTROL_H

#include <Arduino.h>   // para Serial

// --------------------------------------------------------------
// Clase PID discreto (forma posicional)
// --------------------------------------------------------------
class PIDDiscreto {
private:
    double Kp, Ki, Kd;
    double Ts;
    double integral_acum;
    double error_anterior;

public:
    PIDDiscreto(double kp, double ki, double kd, double ts);
    void Reset();
    double Calcular(double error);
    // Métodos para modificar parámetros
    void setKp(double kp) { Kp = kp; }
    void setKi(double ki) { Ki = ki; }
    void setKd(double kd) { Kd = kd; }
    void setTs(double ts) { Ts = ts; }
};

// --------------------------------------------------------------
// Clase Control Binario (On/Off con banda de histéresis)
// --------------------------------------------------------------
class ControlBinario {
private:
    double anchoBanda;
    double ultimaSalida;

public:
    ControlBinario(double anchoBanda);
    void Reset();
    double Calcular(double setpoint, double medicion);
    void setAnchoBanda(double ancho) { anchoBanda = ancho; }
};

// --------------------------------------------------------------
// Clase Control que COMPONE un PID y un ControlBinario
// --------------------------------------------------------------
class Control {
private:
    PIDDiscreto pid;
    ControlBinario binario;

    bool banderaPID;
    bool banderaOnOff;

    bool flagSalidaForzada;
    double valorSalidaForzada;   // [0, 100]

public:
    Control(double kp, double ki, double kd, double ts,
            double anchoHisteresis,
            bool habilitarPID, bool habilitarOnOff);

    // Métodos para modificar parámetros del PID
    void setKp(double kp);
    void setKi(double ki);
    void setKd(double kd);
    void setTs(double ts);          // cambia Ts y resetea el PID
    void setHisteresis(double ancho); // cambia el ancho de banda del ON‑OFF

    // Métodos para cambiar modo
    void setModo(bool pidOn, bool onOffOn);
    void setBanderaPID(bool estado);
    void setBanderaOnOff(bool estado);
    bool getBanderaPID() const;
    bool getBanderaOnOff() const;

    void Reset();

    void ForzarSalida(bool forzar, double valor);

    double CalcularError(double setpoint, double medicion);
};

#endif // CONTROL_H