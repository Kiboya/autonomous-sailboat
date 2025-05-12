#include "qmc5883l.h"
#include <math.h>

QMC5883L::QMC5883L(TwoWire &wire, uint8_t addr) : _wire(wire), _addr(addr) {}

void QMC5883L::begin() {
  _wire.begin();
  // Configuration : registre 0x09, mode continu, 50Hz, 2G sensitivity, etc.
  _wire.beginTransmission(_addr);
  _wire.write(0x09);
  _wire.write(0x05);
  _wire.endTransmission();
  delay(10);
}

int16_t QMC5883L::read16BitRegister(uint8_t reg) {
  _wire.beginTransmission(_addr);
  _wire.write(reg);
  _wire.endTransmission(false);
  _wire.requestFrom(_addr, (uint8_t)2);
  if (_wire.available() >= 2) {
    uint8_t lsb = _wire.read();
    uint8_t msb = _wire.read();
    return (int16_t)((msb << 8) | lsb);
  }
  return 0;
}

float QMC5883L::getHeading() {
  int16_t x = read16BitRegister(0x00);
  int16_t y = read16BitRegister(0x02);
  float angle_rad = atan2((float)y, (float)x);
  float angle_deg = angle_rad * 180.0 / PI;
  if (angle_deg < 0)
    angle_deg += 360;
  return angle_deg;
}

