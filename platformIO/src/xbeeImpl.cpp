#include <Arduino.h>
#include "xbeeImpl.h"

xbeeImpl::xbeeImpl()
{
    pinMode(XBee_rssi_pin, INPUT);
    pinMode(XBee_dout_pin, INPUT_PULLUP);
    pinMode(XBee_reset_pin, OUTPUT);
    digitalWrite(XBee_reset_pin, LOW);
    delay(10);
    digitalWrite(XBee_reset_pin, HIGH);
    pinMode(XBee_dout_pin, INPUT);
    Serial1.setRX(XBee_dout_pin);
    Serial1.setTX(XBee_din_pin);
    Serial1.begin(9600, SERIAL_8N1);
}

String xbeeImpl::Read()
{
    if (Serial1.available())
    {
        String receivedMessage = "";

        while (Serial1.available())
        {
            char c = Serial1.read();
            receivedMessage += c;
        }
        return receivedMessage;
    }
}

void xbeeImpl::Send(int currentAngle, int targetAngle, int servoAnglePosition, Stream &output)
{
    static int previous_curr = -1;
    static int previous_target = -1;
    static int previous_serv = -1;

    if (currentAngle != previous_curr)
    {
        output.print("current:");
        output.println(currentAngle);
        previous_curr = currentAngle;
    }

    if (targetAngle != previous_target)
    {
        output.print("target:");
        output.println(targetAngle);
        previous_target = targetAngle;
    }

    if (servoAnglePosition != previous_serv)
    {
        output.print("servo:");
        output.println(servoAnglePosition);
        previous_serv = servoAnglePosition;
    }
}
