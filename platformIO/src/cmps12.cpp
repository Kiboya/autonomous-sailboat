#include "cmps12.h"

CMPS12::CMPS12(TwoWire &wire, uint8_t addr) : _wire(wire), _addr(addr) {}

void CMPS12::begin() {
  _wire.begin();
  delay(1000); // Stabilisation du bus
}

uint8_t CMPS12::read8BitRegister(uint8_t reg) {
  _wire.beginTransmission(_addr);
  _wire.write(reg);
  _wire.endTransmission(false);
  _wire.requestFrom(_addr, (uint8_t)1);
  if (_wire.available() == 1)
    return _wire.read();
  else
    return 0;
}

int16_t CMPS12::read16BitRegister(uint8_t reg) {
  _wire.beginTransmission(_addr);
  _wire.write(reg);
  _wire.endTransmission(false);
  _wire.requestFrom(_addr, (uint8_t)2);
  if (_wire.available() >= 2) {
    uint8_t highByte = _wire.read();
    uint8_t lowByte  = _wire.read();
    return (int16_t)((highByte << 8) | lowByte);
  }
  return 0;
}

uint16_t CMPS12::readCompassBearing() {
  return read16BitRegister(0x02);
}

int8_t CMPS12::readPitch() {
  return (int8_t)read8BitRegister(0x04);
}

int8_t CMPS12::readRoll() {
  return (int8_t)read8BitRegister(0x05);
}

uint8_t CMPS12::readCalibrationState() {
  return read8BitRegister(0x1E);
}

void CMPS12::startCalibration() {
  _wire.beginTransmission(_addr);
  _wire.write(0x00);
  _wire.write(0xF0); // Active le mode calibration
  uint8_t error = _wire.endTransmission();
  if (error == 0)
    Serial.println("Calibration mode activated. Rotate the sensor.");
  else {
    Serial.print("Failed to activate calibration mode. Erreur I2C: ");
    Serial.println(error);
  }
}

void CMPS12::endCalibration() {
  _wire.beginTransmission(_addr);
  _wire.write(0x00);
  _wire.write(0xF1); // Quitte le mode calibration
  if (_wire.endTransmission() == 0) {
    Serial.println("Calibration completed.");
    saveCalibration();
  } else
    Serial.println("Failed to end calibration.");
}

void CMPS12::saveCalibration() {
  const uint8_t commands[] = {0xF0, 0xF5, 0xF6};
  for (int i = 0; i < 3; i++) {
    _wire.beginTransmission(_addr);
    _wire.write(0x00);
    _wire.write(commands[i]);
    if (_wire.endTransmission() != 0) {
      Serial.print("Failed to send command: 0x");
      Serial.println(commands[i], HEX);
      return;
    }
    delay(20);
  }
  Serial.println("Calibration saved successfully.");
}

