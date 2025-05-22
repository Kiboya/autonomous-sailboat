#ifndef XBEE_IMPL_H
#define XBEE_IMPL_H

#include <Arduino.h>

const int XBee_reset_pin = 21;
const int XBee_rssi_pin = 27;
const int XBee_dout_pin = 1;
const int XBee_din_pin = 0;

const int rtk_rx_pin = 9;
const int rtk_tx_pin = 8;

class xbeeImpl
{
private:
    // PID Parameters
    float Kp = 1.0;
    float Ki = 1.0;

    // RTK Parameter
    String rtk = "";

    // Message 
    String receivedMessage;

public:
    xbeeImpl();
    void Read();
    void getValue();
    void Send(int currentTension, int currentAngle, int targetAngle, int servoAnglePosition) const;

    // Getters for PID Parameters
    float getKp() const { return Kp; }
    float getKi() const { return Ki; }
};

#endif // XBEE_IMPL_H