#include "Control.h"

// ========== Implementación PIDDiscreto ==========
PIDDiscreto::PIDDiscreto(double kp, double ki, double kd, double ts)
    : Kp(kp), Ki(ki), Kd(kd), Ts(ts), integral_acum(0.0), error_anterior(0.0)
{}

void PIDDiscreto::Reset() {
    integral_acum = 0.0;
    error_anterior = 0.0;
}

double PIDDiscreto::Calcular(double error) {
    double P = Kp * error;
    integral_acum += error * Ts;
    double I = Ki * integral_acum;
    double D = (Kd / Ts) * (error - error_anterior);
    error_anterior = error;
    return P + I + D;
}

// ========== Implementación ControlBinario ==========
ControlBinario::ControlBinario(double anchoBanda)
    : anchoBanda(anchoBanda), ultimaSalida(0.0)
{}

void ControlBinario::Reset() {
    ultimaSalida = 0.0;
}

double ControlBinario::Calcular(double setpoint, double medicion) {
    double mitad = anchoBanda / 2.0;
    if (medicion < setpoint - mitad) {
        ultimaSalida = 1.0;
    } else if (medicion > setpoint + mitad) {
        ultimaSalida = 0.0;
    }
    return ultimaSalida;
}

// ========== Implementación Control ==========
Control::Control(double kp, double ki, double kd, double ts,
                 double anchoHisteresis,
                 bool habilitarPID, bool habilitarOnOff)
    : pid(kp, ki, kd, ts),
      binario(anchoHisteresis),
      banderaPID(habilitarPID),
      banderaOnOff(habilitarOnOff),
      flagSalidaForzada(false),
      valorSalidaForzada(0.0)
{}

void Control::SetBanderaPID(bool estado) {
    banderaPID = estado;
}

void Control::SetBanderaOnOff(bool estado) {
    banderaOnOff = estado;
}

bool Control::GetBanderaPID() const {
    return banderaPID;
}

bool Control::GetBanderaOnOff() const {
    return banderaOnOff;
}

void Control::Reset() {
    pid.Reset();
    binario.Reset();
}

void Control::ForzarSalida(bool forzar, double valor) {
    if (forzar) {
        if (valor >= 0.0 && valor <= 100.0) {
            flagSalidaForzada = true;
            valorSalidaForzada = valor;
        } else {
            std::cerr << "Error: Valor de salida forzada (" << valor
                      << ") debe estar entre 0 y 100. No se forza la salida.\n";
            flagSalidaForzada = false;  // no se fuerza
        }
    } else {
        flagSalidaForzada = false;
        // No se modifica valorSalidaForzada, pero ya no se usa
    }
}

double Control::CalcularError(double setpoint, double medicion) {
    // Si la salida está forzada, retornar el valor prefijado
    if (flagSalidaForzada) {
        return valorSalidaForzada;
    }

    double error = setpoint - medicion;
    double salida = 0.0;

    if (banderaPID) {
        salida += pid.Calcular(error);
    }
    if (banderaOnOff) {
        salida += binario.Calcular(setpoint, medicion);
    }
    return salida;
}