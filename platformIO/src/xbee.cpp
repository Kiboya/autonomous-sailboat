#include <Arduino.h>
#include "xbee.hpp"

const int XBee_reset_pin = 21;
const int XBee_rssi_pin  = 27;
const int XBee_dout_pin  = 1;
const int XBee_din_pin   = 0;

const int rtk_rx_pin = 9;
const int rtk_tx_pin = 8;

// Command Parameters
int targetAngle = 0;
float Kp = 1.0;
float Ki = 1.0;
String rtk = "";

void xbee_setup() {
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
    Serial2.begin(9600, SERIAL_8N1);

    Serial.begin(9600);
}


void getValue(String receivedMessage) {
    Serial.println(receivedMessage);
    // Trim whitespace or newlines
    receivedMessage.trim();

    // Find the position of the ':'
    int separatorIndex = receivedMessage.indexOf(':');

    if (separatorIndex != -1) {
        // Extract the key and value
        String key = receivedMessage.substring(0, separatorIndex);
        String value = receivedMessage.substring(separatorIndex + 1);

        if ()
        {
            // Convert value to integer or float depending on the key
            if (key == "cap") {
                if (value.toInt() <= 0 || value.toInt() >= 360) {
                    Serial.println("Error: 'cap' must be between 0 and 360. Ignoring invalid value.");
                } else {
                    Serial.print("cap value: ");
                    Serial.println(value.toInt());
                    targetAngle = value.toInt();
                }
            }
            else if (key == "ki") {
                Ki = value.toFloat();
                Serial.print("ki value: ");
                Serial.println(Ki);
            }
            else if (key == "kp") {
                Kp = value.toFloat();
                Serial.print("kp value: ");
                Serial.println(Kp);
            }
            else if (key == "rtk") {
                rtk = value;
                Serial.print("rtk value: ");
                Serial.println(rtk);
                Serial2.print(rtk);
                Serial.print("rtk value sended to GPS");

            }
            else {
                Serial.printf("Invalid key. Expected 'cap','kp' or 'ki'. Received : %s", key);
            }
        } else {
            Serial.println("Invalid format. Expected 'key:value'.");
        }
    }
}

// void sendValue(int currentAngle, int targetAngle, int servoAnglePosition, Stream &output)
// {
//     static int previous_curr = -1;
//     static int previous_target = -1;
//     static int previous_serv = -1;

//     if(currentAngle != previous_curr){
//         output.print("current:");
//         output.println(currentAngle);
//         previous_curr = currentAngle;
//     }

//     if(targetAngle != previous_target){
//         output.print("target:");
//         output.println(targetAngle);
//         previous_target = targetAngle;
//     }

//     if(servoAnglePosition != previous_serv){
//         output.print("servo:");
//         output.println(servoAnglePosition);
//         previous_serv = servoAnglePosition;
//     }
// }


void xbee_run() {
    if (Serial1.available()) {
        String receivedMessage = "";

        while (Serial1.available()) {
            char c = Serial1.read();
            receivedMessage += c;
        }
        getValue(receivedMessage);
    }
}