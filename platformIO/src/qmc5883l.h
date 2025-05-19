#ifndef QMC5883L_H
#define QMC5883L_H

#include <Arduino.h>
#include <Wire.h>

class QMC5883L {
public:
  // Constructeur : on passe l'instance TwoWire et l'adresse I2C (par défaut 0x0D)
  QMC5883L(TwoWire &wire, uint8_t addr = 0x0D);

  void begin();
  float getHeading(); // Retourne l'orientation en degrés

private:
  TwoWire &_wire;
  uint8_t _addr;
  int16_t read16BitRegister(uint8_t reg);
};

#endif

