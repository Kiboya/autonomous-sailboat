#include <Arduino.h>
#include <Servo.h>
#include "servoControl.h"
#include "xbeeImpl.h"

xbeeImpl xbee;

// Constructor
servoControl::servoControl()
{
    Serial.begin(9600);

    // Safran setup
    safranServo.attach(safranPin);
    safranServo.writeMicroseconds(init_safran);

    // Sail setup
    sailServo.attach(sailPin);
    sailServo.writeMicroseconds(init_sail);

    Serial.println("Servo control initialized.");
}

void servoControl::servo_control()
{
    getValue(xbee.Read());

    getCurrentAngleFromNorth();
    // Calculate the angle angle between the current angle and the target angle
    int error = calculateShortestPath(currentAngle, targetAngle);

    // PI Controller: calculate the proportional and integral terms
    integral += error;                           // accumulate the error over time
    int adjustment = Kp * error + Ki * integral; // PI control

    // Update safran servo position with the new adjustment
    servoAnglePosition = constrain(servoAnglePosition - adjustment, min_angle_safran, max_angle_safran);
    ms_safran_position = map(servoAnglePosition, min_angle_safran, max_angle_safran, min_ms_safran, max_ms_safran);
    safranServo.writeMicroseconds(ms_safran_position);

    // Update sail servo position with the new adjustment
    ms_sail_position = map(voileTensionPosition, min_angle_sail, max_angle_sail, min_ms_sail, max_ms_sail);
    safranServo.writeMicroseconds(ms_sail_position);

    // Send value to own XBee
    xbee.Send(currentTension, currentAngle, targetAngle, servoAnglePosition, Serial1);

    // Reset values for the next loop if needed
    servoAnglePosition = init_angle_safran;
    ms_safran_position = init_safran;
}

void servoControl::simulateMovingBoat(int angleDifference)
{
    int stepSize = 1;

    if (angleDifference > 0)
    {
        currentAngle += stepSize;
    }
    else if (angleDifference < 0)
    {
        currentAngle -= stepSize;
    }
    if (abs(currentAngle - targetAngle) <= stepSize)
    {
        currentAngle = targetAngle;
    }
    currentAngle = currentAngle + 360 % 360;
}

int servoControl::calculateShortestPath(int current, int target)
{
    int angleDifference = target - current;
    if (angleDifference > 180)
    {
        angleDifference -= 360;
    }
    else if (angleDifference < -180)
    {
        angleDifference += 360;
    }
    return angleDifference;
}

void servoControl::getValue(String receivedMessage)
{
    // Trim whitespace or newlines
    receivedMessage.trim();

    // Find the position of the ':'
    int separatorIndex = receivedMessage.indexOf(':');

    if (separatorIndex != -1)
    {
        // Extract the key and value
        String key = receivedMessage.substring(0, separatorIndex);
        String value = receivedMessage.substring(separatorIndex + 1);

        // Convert value to integer or float depending on the key
        if (key == "cap")
        {
            if (value.toInt() <= 0 || value.toInt() >= 360)
            {
                Serial.println("Error: 'cap' must be between 0 and 360. Ignoring invalid value.");
            }
            else
            {
                Serial.print("cap value: ");
                Serial.println(value.toInt());
                targetAngle = value.toInt();
            }
        }
        if (key == "voile")
        {
            if (value.toInt() <= 0 || value.toInt() >= 100)
            {
                Serial.println("Error: 'voile' must be between 0 and 100. Ignoring invalid value.");
            }
            else
            {
                Serial.print("voile value: ");
                Serial.println(value.toInt());
                targetAngle = value.toInt();
            }
        }
        else if (key == "ki")
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
        else
        {
            Serial.println("Invalid key. Expected 'cap','kp' or 'ki'.");
        }
    }
    else
    {
        Serial.println("Invalid format. Expected 'key:value'.");
    }
}

// Simulation for now but will use a compass
void servoControl::getCurrentAngleFromNorth()
{
    // Calculate the movement needed to go from the current angle to the target angle
    int angleDifference = calculateShortestPath(currentAngle, targetAngle);
    // Simulate the angle slowly getting smaller
    simulateMovingBoat(angleDifference);
}