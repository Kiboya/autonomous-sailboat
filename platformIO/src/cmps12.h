#ifndef CMPS12_H
#define CMPS12_H

#include <Arduino.h>
#include <Wire.h>

class CMPS12 {
public:
  // Constructeur : on passe l'instance TwoWire et l'adresse I2C (par défaut 0x60)
  CMPS12(TwoWire &wire, uint8_t addr = 0x60);

  void begin();
  void startCalibration();
  void endCalibration();
  void saveCalibration();

  // Méthodes de lecture
  uint16_t readCompassBearing();
  int8_t   readPitch();
  int8_t   readRoll();
  uint8_t  readCalibrationState();

private:
  TwoWire &_wire;
  uint8_t _addr;
  uint8_t read8BitRegister(uint8_t reg);
  int16_t read16BitRegister(uint8_t reg);
};

#endif

