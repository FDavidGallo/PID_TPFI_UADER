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

void Control::setKp(double kp) {
    pid.setKp(kp);
}

void Control::setKi(double ki) {
    pid.setKi(ki);
}

void Control::setKd(double kd) {
    pid.setKd(kd);
}

void Control::setTs(double ts) {
    pid.setTs(ts);
    pid.Reset();   // reiniciamos el acumulador y derivador
}

void Control::setHisteresis(double ancho) {
    binario.setAnchoBanda(ancho);
}

void Control::setModo(bool pidOn, bool onOffOn) {
    banderaPID = pidOn;
    banderaOnOff = onOffOn;
    Reset(); // opcional: reiniciar estados internos
}

void Control::setBanderaPID(bool estado) {
    banderaPID = estado;
}

void Control::setBanderaOnOff(bool estado) {
    banderaOnOff = estado;
}

bool Control::getBanderaPID() const {
    return banderaPID;
}

bool Control::getBanderaOnOff() const {
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
            Serial.println("Error: Valor de salida forzada debe estar entre 0 y 100. No se forza.");
            flagSalidaForzada = false;
        }
    } else {
        flagSalidaForzada = false;
    }
}

double Control::CalcularError(double setpoint, double medicion) {
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