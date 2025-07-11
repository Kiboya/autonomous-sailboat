#include <Arduino.h>
#include "xbeeImpl.h"
#include "shared_data.h"
#include "FreeRTOS.h"
#include "task.h"

xbeeImpl::xbeeImpl()
{
    pinMode(XBee_rssi_pin, INPUT);
    pinMode(XBee_dout_pin, INPUT_PULLUP);
    pinMode(XBee_reset_pin, OUTPUT);
    pinMode(XBee_din_pin, OUTPUT);
    pinMode(rtk_rx_pin, INPUT);
    pinMode(rtk_tx_pin, OUTPUT);
}

void xbeeImpl::initialize()
{
    digitalWrite(XBee_reset_pin, LOW);
    delay(10);
    digitalWrite(XBee_reset_pin, HIGH);
    // pinMode(XBee_dout_pin, INPUT);

    Serial1.setRX(XBee_dout_pin);
    Serial1.setTX(XBee_din_pin);
    Serial1.begin(115200, SERIAL_8N1);

    Serial2.setRX(rtk_rx_pin);
    Serial2.setTX(rtk_tx_pin);
    Serial2.begin(38400, SERIAL_8N1);
}

void xbeeImpl::read()
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
        // Serial.print("receivedMessage: ");
        // Serial.println(receivedMessage);
        receivedMessage.trim();
        Serial2.write((const uint8_t *)receivedMessage.c_str(), receivedMessage.length());

        getValue(receivedMessage);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
}

void xbeeImpl::getValue(String receivedMessage)
{
    if (receivedMessage.length() == 0)
    {
        return;
    }
    // Find the position of the ':'
    int separatorIndex = receivedMessage.indexOf(':');
    Serial.println(receivedMessage);

    if (separatorIndex != -1)
    {
        // Extract the key and value
        String key = receivedMessage.substring(0, separatorIndex);
        String value = receivedMessage.substring(separatorIndex + 1);

        // Convert value to integer or float depending on the key
        if (key == "ki")
        {
            Ki = value.toFloat();
            // Serial.print("ki value: ");
            // Serial.println(Ki);
        }
        else if (key == "kp")
        {
            Kp = value.toFloat();
            // Serial.print("kp value: ");
            // Serial.println(Kp);
        }
        if (key == "tension")
        {
            sharedData.targetTension = value.toFloat();
            Serial.print("targetTension value: ");
            Serial.println(sharedData.targetTension);
        }
        else if (key == "cap")
        {
            sharedData.targetAngle = value.toFloat();
            Serial.print("targetAngle value: ");
            Serial.println(sharedData.targetAngle);
        }
        else if (key == "rtk")
        {
            rtk = value;
            // Serial.println(rtk);
            Serial2.print(rtk);
            // Serial.println("rtk value sended to GPS");
        }
        else if (key == "point_lon")
        {
            lon = value.toDouble();
            sharedData.waypoint_lon = lon;
            // Serial.println(lon);
            // Serial.println("rtk value sended to GPS");
        }
        else if (key == "point_lat")
        {
            lat = value.toDouble();
            sharedData.waypoint_lat = lat;
            // Serial.println(rtk);
            // Serial.println("rtk value sended to GPS");
        }
        else
        {
            Serial.println("Invalid key. Expected 'kp', 'ki', 'point_lat', 'point_lon' or 'rtk'.");
        }
    }
    else
    {
        Serial.println("Invalid format. Expected 'key:value'.");
    }
}

void xbeeImpl::send(const SharedData& data) const
{
    static double prev_lat = -9999.0;
    static double prev_lon = -9999.0;
    static double prev_compass = -9999.0;
    static double prev_wind = -9999.0;
    static double prev_h_tilt = -9999.0;
    static double prev_v_tilt = -9999.0;
    static int prev_target_angle = -1;
    static int prev_target_tension = -1;
    static int prev_angle_from_north = -1;

    if (data.latitude != prev_lat) {
        Serial1.print("latitude:");
        Serial1.println(data.latitude, 6);  // 6 decimal precision
        prev_lat = data.latitude;
    }

    if (data.longitude != prev_lon) {
        Serial1.print("longitude:");
        Serial1.println(data.longitude, 6);
        prev_lon = data.longitude;
    }

    if (data.compass != prev_compass) {
        Serial1.print("compass:");
        Serial1.println(data.compass, 2);
        prev_compass = data.compass;
    }

    if (data.wind_vane != prev_wind) {
        Serial1.print("wind_vane:");
        Serial1.println(data.wind_vane, 2);
        prev_wind = data.wind_vane;
    }

    if (data.horizontal_tilt != prev_h_tilt) {
        Serial1.print("horizontal_tilt:");
        Serial1.println(data.horizontal_tilt, 2);
        prev_h_tilt = data.horizontal_tilt;
    }

    if (data.vertical_tilt != prev_v_tilt) {
        Serial1.print("vertical_tilt:");
        Serial1.println(data.vertical_tilt, 2);
        prev_v_tilt = data.vertical_tilt;
    }

    if (data.targetAngle != prev_target_angle) {
        Serial1.print("target_angle:");
        Serial1.println(data.targetAngle);
        prev_target_angle = data.targetAngle;
    }

    if (data.targetTension != prev_target_tension) {
        Serial1.print("target_tension:");
        Serial1.println(data.targetTension);
        prev_target_tension = data.targetTension;
    }

    if (data.angleFromNorth != prev_angle_from_north) {
        Serial1.print("angle_from_north:");
        Serial1.println(data.angleFromNorth);
        prev_angle_from_north = data.angleFromNorth;
    }
}

