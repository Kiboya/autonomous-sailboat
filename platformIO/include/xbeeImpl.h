#ifndef XBEE_IMPL_H
#define XBEE_IMPL_H

#include <Arduino.h>

const int XBee_reset_pin = 21;
const int XBee_rssi_pin = 27;
const int XBee_dout_pin = 1;
const int XBee_din_pin = 0;

const int minAngle_ms = 960;
const int maxAngle_ms = 1640;
const int minAngle = 70;
const int maxAngle = 170;

class xbeeImpl
{
public:
    xbeeImpl();
    String xbeeImpl::Read();
    void xbeeImpl::Send(int currentAngle, int targetAngle, int servoAnglePosition, Stream &output);
};

#endif // XBEE_IMPL_H