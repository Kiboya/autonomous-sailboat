#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <Arduino.h>
#include <Servo.h>

// Value safran
const int min_ms_safran = 960;
const int max_ms_safran = 1640;
const int init_safran = 1300;

// value sails
const int min_ms_sail = 1320;
const int max_ms_sail = 1800;
const int init_sail = 1800;

const int safranPin = 3;
const int sailPin = 28;

class servoControl
{
private:
    Servo safranServo;
    Servo sailServo;

    // Command Parameters
    int targetAngle = 0;
    float Kp = 1.0;
    float Ki = 1.0;

    // Control Parameters
    int currentAngle = 0;
    int servoAnglePosition = 125;
    int ms_position;

    // Variables for PI control
    float previousError = 0.0;
    float integral = 0.0;

public:
    servoControl();
    void servo_control();
    void getCurrentAngleFromNorth();
    void simulateMovingBoat(int angleDifference);
    int calculateShortestPath(int current, int target);
    void getValue(String receivedMessage);

    // Getters for Command Parameters
    int getTargetAngle() const { return targetAngle; }
    float getKp() const { return Kp; }
    float getKi() const { return Ki; }

    // Getters for Control Parameters
    int getCurrentAngle() const { return currentAngle; }
    int getServoAnglePosition() const { return servoAnglePosition; }
    int getMsPosition() const { return ms_position; }

    // Getters for PI Control Variables
    float getPreviousError() const { return previousError; }
    float getIntegral() const { return integral; }
};

#endif // SERVO_CONTROL_H