#include <Arduino.h>
#include <Servo.h>
#include "servoControl.h"
#include "xbeeImpl.h"
#include "shared_data.h"

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

void servoControl::servo_control(const xbeeImpl &xbee)
{
    int targetAngle = sharedData.targetAngle;
    int targetTension = sharedData.targetTension;
    int angleFromNorth = sharedData.angleFromNorth;
    float Kp = xbee.getKp();
    float Ki = xbee.getKi();

    // Calculate the angle angle between the current angle and the target angle
    int error = calculateShortestPath(angleFromNorth, targetAngle);

    // PI Controller: calculate the proportional and integral terms
    cumulateError += error;                           // accumulate the error over time
    adjustment = Kp * error + Ki * cumulateError; // PI control

    // Update safran servo position with the new adjustment
    servoAnglePosition = constrain(servoAnglePosition - adjustment, min_angle_safran, max_angle_safran);
    ms_safran_position = map(servoAnglePosition, min_angle_safran, max_angle_safran, min_ms_safran, max_ms_safran);
    safranServo.writeMicroseconds(ms_safran_position);

    // Update sail servo position with the new adjustment
    ms_sail_position = map(voileTensionPosition, min_angle_sail, max_angle_sail, min_ms_sail, max_ms_sail);
    safranServo.writeMicroseconds(ms_sail_position);

    // // Reset values for the next loop if needed
    // servoAnglePosition = init_angle_safran;
    // ms_safran_position = init_safran;
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