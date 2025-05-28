#include <Arduino.h>
#include <Servo.h>
#include "servoControl.h"
#include "xbeeImpl.h"
#include "shared_data.h"

// Constructor
servoControl::servoControl()
{
    // Safran setup
    safranServo.attach(safranPin);
    safranServo.writeMicroseconds(init_safran);

    // Sail setup
    sailServo.attach(sailPin);
    sailServo.writeMicroseconds(init_sail);
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
    sailServo.writeMicroseconds(max_ms_sail);

    Serial.print("servoAnglePosition:");
    Serial.println(servoAnglePosition);
    Serial.print("ms_safran_position:");
    Serial.println(ms_safran_position);
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