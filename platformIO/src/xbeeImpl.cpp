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
    return "";
}

void xbeeImpl::Send(int currentTension, int currentAngle, int targetAngle, int servoAnglePosition, Stream &output)
{
    static int previous_curr = -1;
    static int previous_target = -1;
    static int previous_serv = -1;
    static int previous_tension = -1;

    if (currentAngle != previous_curr)
    {
        output.print("current angle:");
        output.println(currentAngle);
        previous_curr = currentAngle;
    }

    if (currentTension != previous_tension)
    {
        output.print("current tension:");
        output.println(currentTension);
        previous_tension = currentTension;
    }

    if (targetAngle != previous_target)
    {
        output.print("target angle:");
        output.println(targetAngle);
        previous_target = targetAngle;
    }

    if (servoAnglePosition != previous_serv)
    {
        output.print("servo angle position:");
        output.println(servoAnglePosition);
        previous_serv = servoAnglePosition;
    }
}
