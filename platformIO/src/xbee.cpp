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
String complete_message;
size_t receivedMessageLength;


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
    Serial2.begin(38400, SERIAL_8N1);

    Serial.begin(9600);
}


// void getValue(String receivedMessage)
// {
//     static String previousKey = "";
//     static String previousValue = "";

//     // Trim whitespace or newlines
//     receivedMessage.trim();

//     // Ignore empty strings
//     if (receivedMessage.length() == 0) return;

//     // Find the position of the ':'
//     int separatorIndex = receivedMessage.indexOf(':');
//     if (separatorIndex == -1) {
//         Serial.println("Invalid format. Expected 'key:value'.");
//         return;
//     }

//     // Extract key and value
//     String key = receivedMessage.substring(0, separatorIndex);
//     String value = receivedMessage.substring(separatorIndex + 1);

//     // Si on collectait les caractères pour un nouveau message et que la clé change, l'envoyer tout de suite
//     if (previousKey != "" && key != previousKey)
//     {
//         if (previousKey == "cap")
//         {
//             int val = previousValue.toInt();
//             if (val > 0 && val < 360)
//             {
//                 targetAngle = val;
//                 Serial.print("cap value: ");
//                 Serial.println(targetAngle);
//             }
//             else
//             {
//                 Serial.println("Error: 'cap' must be between 0 and 360. Ignoring invalid value.");
//             }
//         }
//         else if (previousKey == "kp")
//         {
//             Kp = previousValue.toFloat();
//             Serial.print("kp value: ");
//             Serial.println(Kp);
//         }
//         else if (previousKey == "ki")
//         {
//             Ki = previousValue.toFloat();
//             Serial.print("ki value: ");
//             Serial.println(Ki);
//         }
//         else if (previousKey == "rtk")
//         {
//             rtk = previousValue;
//             Serial.print("rtk value: ");
//             Serial.println(rtk);
//             sendRTKData(rtk);
//             Serial.println("rtk binary data sent to GPS");
//         }
//         else
//         {
//             Serial.print("Invalid key. Expected 'cap', 'kp', 'ki' or 'rtk'. Received: ");
//             Serial.println(previousKey);
//         }
//     }
//     // Stocker la valeur et la clé pour la prochaine fois
//     previousKey = key;
//     previousValue = value;
// }

void xbee_get_value(String receivedMessage)
{
    // Serial.print("StartGetValue---");
    // Serial.print(receivedMessage);
    // Serial.println("   ---FinStartGetValue");
    // Trim whitespace or newlines
    receivedMessage.trim();

    // Find the position of the ':'
    int separatorIndex = receivedMessage.indexOf(':');

    if (separatorIndex != -1) {
        // Extract the key and value
        String key = receivedMessage.substring(0, separatorIndex);
        String value = receivedMessage.substring(separatorIndex + 1);

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
            Serial.println(rtk);
            Serial2.print(rtk);
            Serial.println("rtk value sended to GPS");

        }
        else {
            Serial.println("Invalid key. Expected 'cap','kp', 'ki' or 'rtk.");
        }
    } else {
        Serial.println("Invalid format. Expected 'key:value'.");
    }
}

// void xbee_run()
// {
    // static String receivedMessage = ""; // Variable pour stocker le message reçu

    // while (Serial1.available())
    // {
    //     uint8_t c = Serial1.read(); // Lire un octet depuis XBee

    //     // Ajouter l'octet à la chaîne de message
    //     receivedMessage += (char)c;

    //     // Vérifier si la fin d'une balise est atteinte (le caractère ':' pour la balise)
    //     if (c == ':')
    //     {
    //         // Si on a une nouvelle balise, traiter le message
    //         xbee_create_message(receivedMessage); // Traiter la balise et sa donnée
    //         receivedMessage = ""; // Réinitialiser pour le prochain message
    //     }

    //     // Vérifier si on a un message complet
    //     if (c == '\n')
    //     { // Si on rencontre un saut de ligne (fin de message)
    //         getValue(receivedMessage); // Traiter le message complet
    //         receivedMessage = ""; // Réinitialiser pour le prochain message
    //     }
    // }
// }

void xbee_run()
{
    if (Serial1.available())
    {
        String receivedMessage = "";
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
        receivedMessageLength = Serial2.write((const uint8_t *)receivedMessage.c_str(), receivedMessage.length());
        Serial.print("Taille du Message RTCM reçu : ");
        Serial.println(receivedMessageLength);
        //xbee_get_value(receivedMessage);
    }

    delay(100);
}

// void sendRTKData(String hexString)
// {
//     int len = hexString.length();
//     for (int i = 0; i < len; i += 2)
//     {
//         String byteString = hexString.substring(i, i + 2);
//         uint8_t byteVal = (uint8_t) strtol(byteString.c_str(), nullptr, 16);
//         Serial2.write(byteVal);
//     }
// }