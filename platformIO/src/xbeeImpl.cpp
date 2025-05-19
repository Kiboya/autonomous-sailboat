#include <Arduino.h>
#include "xbeeImpl.h"

xbeeImpl::xbeeImpl()
{
    pinMode(XBee_rssi_pin, INPUT);
    pinMode(XBee_dout_pin, INPUT_PULLUP);
    pinMode(XBee_reset_pin, OUTPUT);
    pinMode(XBee_din_pin, OUTPUT);
    pinMode(rtk_rx_pin, INPUT);
    pinMode(rtk_tx_pin, OUTPUT);
    digitalWrite(XBee_reset_pin, LOW);
    delay(10);
    digitalWrite(XBee_reset_pin, HIGH);
    pinMode(XBee_dout_pin, INPUT);
    Serial1.setRX(XBee_dout_pin);
    Serial1.setTX(XBee_din_pin);
    Serial1.begin(9600, SERIAL_8N1);

    Serial2.setRX(rtk_rx_pin);
    Serial2.setTX(rtk_tx_pin);
    Serial2.begin(38400, SERIAL_8N1);
}

void xbeeImpl::Read()
{
    if (Serial1.available())
    {
        receivedMessage = "";
        char c = ' ';
        while (c != '|')
        {
            if (Serial1.available())
            {
                c = Serial1.read();
                if (c != '|')
                {
                    receivedMessage += c;
                }
            }
        }
        Serial.println(receivedMessage);
        receivedMessage.trim();
        Serial2.write((const uint8_t *)receivedMessage.c_str(), receivedMessage.length());
    }

    delay(100);
}

void xbeeImpl::getValue()
{
    // Find the position of the ':'
    int separatorIndex = receivedMessage.indexOf(':');

    if (separatorIndex != -1)
    {
        // Extract the key and value
        String key = receivedMessage.substring(0, separatorIndex);
        String value = receivedMessage.substring(separatorIndex + 1);

        // Convert value to integer or float depending on the key
        if (key == "ki")
        {
            Ki = value.toFloat();
            Serial.print("ki value: ");
            Serial.println(Ki);
        }
        else if (key == "kp")
        {
            Kp = value.toFloat();
            Serial.print("kp value: ");
            Serial.println(Kp);
        }
        else if (key == "rtk") {
            rtk = value;
            Serial.println(rtk);
            Serial2.print(rtk);
            Serial.println("rtk value sended to GPS");

        }
        else {
            Serial.println("Invalid key. Expected 'kp', 'ki' or 'rtk.");
        }
    }
    else
    {
        Serial.println("Invalid format. Expected 'key:value'.");
    }
}

void xbeeImpl::Send(int currentTension, int currentAngle, int targetAngle, int servoAnglePosition) const
{
    static int previous_curr = -1;
    static int previous_target = -1;
    static int previous_serv = -1;
    static int previous_tension = -1;

    if (currentAngle != previous_curr)
    {
        Serial1.print("current angle:");
        Serial1.println(currentAngle);
        previous_curr = currentAngle;
    }

    if (currentTension != previous_tension)
    {
        Serial1.print("current tension:");
        Serial1.println(currentTension);
        previous_tension = currentTension;
    }

    if (targetAngle != previous_target)
    {
        Serial1.print("target angle:");
        Serial1.println(targetAngle);
        previous_target = targetAngle;
    }

    if (servoAnglePosition != previous_serv)
    {
        Serial1.print("servo angle position:");
        Serial1.println(servoAnglePosition);
        previous_serv = servoAnglePosition;
    }
}
