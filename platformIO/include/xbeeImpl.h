#ifndef XBEE_IMPL_H
#define XBEE_IMPL_H

#include <Arduino.h>
#include "shared_data.h"

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
    double lon = 0.0;
    double lat = 0.0;

    // RTK Parameter
    String rtk = "";

    // Message
    String receivedMessage;

public:
    xbeeImpl();

    // Initialize the XBee and RTK serial ports
    void initialize();
    // Read from Serial1(xbee) and write to Serial2 (for RTK)
    void read();
    // Parse the last received message and extract key-value pairs
    void getValue(String receivedMessage);
    // Send shared data to Serial1(xbee) if values have changed
    void send(const SharedData& data) const;

    // Getters for PID Parameters
    float getKp() const { return Kp; }
    float getKi() const { return Ki; }
};

#endif // XBEE_IMPL_H