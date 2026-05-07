#ifndef CONTROL_H
#define CONTROL_H

#include <iostream>   // para std::cerr (solo para el mensaje de error)

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

    // Nuevas variables para salida forzada
    bool flagSalidaForzada;
    double valorSalidaForzada;   // debe estar en [0, 100]

public:
    Control(double kp, double ki, double kd, double ts,
            double anchoHisteresis,
            bool habilitarPID, bool habilitarOnOff);

    void SetBanderaPID(bool estado);
    void SetBanderaOnOff(bool estado);
    bool GetBanderaPID() const;
    bool GetBanderaOnOff() const;

    void Reset();

    // Nuevo método: activa/desactiva la salida forzada y establece el valor
    void ForzarSalida(bool forzar, double valor);

    // Calcula el error y la salida (tiene en cuenta la salida forzada si está activa)
    double CalcularError(double setpoint, double medicion);
};

#endif // CONTROL_H