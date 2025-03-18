#include <Arduino.h>
#include <Servo.h>

const int XBee_reset_pin = 21;
const int XBee_rssi_pin  = 27;
const int XBee_dout_pin  = 1;
const int XBee_din_pin   = 0;

const int minAngle_ms = 960;
const int maxAngle_ms = 1640;
const int minAngle = 70;
const int maxAngle = 170;

Servo myservo;

// Command Parameters
int targetAngle = 0;
float Kp = 1.0;
float Ki = 1.0;

// Control Parameters
int currentAngle = 0;
int servoAnglePosition = 125;
int ms_position = 1300;

// Variables for PI control
float previousError = 0.0;
float integral = 0.0;

void xbee_setup() {
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
    Serial.begin(9600);
    myservo.attach(3);
    myservo.writeMicroseconds(ms_position);
    Serial.println("Servo control initialized.");
}

void simulateMovingBoat(int angleDifference) {
    int stepSize = 1;

    if (angleDifference > 0) {
        currentAngle += stepSize;
    } else if (angleDifference < 0) {
        currentAngle -= stepSize;
    }
    if (abs(currentAngle - targetAngle) <= stepSize) {
        currentAngle = targetAngle;
    }
    currentAngle = currentAngle + 360 % 360;
}

int calculateShortestPath(int current, int target) {
    int angleDifference = target - current;
    if (angleDifference > 180) {
        angleDifference -= 360;
    } else if (angleDifference < -180) {
        angleDifference += 360;
    }
    return angleDifference;
}

void getValue(String receivedMessage) {
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
        else {
            Serial.println("Invalid key. Expected 'cap','kp' or 'ki'.");
        }
    } else {
        Serial.println("Invalid format. Expected 'key:value'.");
    }
}

void sendValue(int currentAngle, int targetAngle, int servoAnglePosition, Stream &output)
{
    static int previous_curr = -1;
    static int previous_target = -1;
    static int previous_serv = -1;

    if(currentAngle != previous_curr){
        output.print("current:");
        output.println(currentAngle);
        previous_curr = currentAngle;
    }

    if(targetAngle != previous_target){
        output.print("target:");
        output.println(targetAngle);
        previous_target = targetAngle;
    }

    if(servoAnglePosition != previous_serv){
        output.print("servo:");
        output.println(servoAnglePosition);
        previous_serv = servoAnglePosition;
    }
    
}

// Simulation for now but will use a compass 
void getCurrentAngleFromNorth() {
    // Calculate the movement needed to go from the current angle to the target angle
    int angleDifference = calculateShortestPath(currentAngle, targetAngle);
    // Simulate the angle slowly getting smaller
    simulateMovingBoat(angleDifference);
}

void servo_control() {
    if (Serial1.available()) {
        String receivedMessage = "";
        
        while (Serial1.available()) {
            char c = Serial1.read();
            receivedMessage += c;
        }
        getValue(receivedMessage);
    }
    
    getCurrentAngleFromNorth();
    // Calculate the angle angle between the current angle and the target angle
    int error = calculateShortestPath(currentAngle, targetAngle);
    
    // PI Controller: calculate the proportional and integral terms
    integral += error;  // accumulate the error over time
    int adjustment = Kp * error + Ki * integral;  // PI control

    // Update servo position with the new adjustment
    servoAnglePosition = constrain(servoAnglePosition - adjustment, minAngle, maxAngle);
    ms_position = map(servoAnglePosition, minAngle, maxAngle, minAngle_ms, maxAngle_ms);
    myservo.writeMicroseconds(ms_position);

    // Send value to own Serial for debugg
    // sendValue(currentAngle, targetAngle, servoAnglePosition, Serial);
    // Send value to own XBee
    sendValue(currentAngle, targetAngle, servoAnglePosition, Serial1);

    // Reset values for the next loop if needed
    servoAnglePosition = 125;
    ms_position = 1300;
    delay(100);
}