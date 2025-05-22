#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <Arduino.h>
#include <Servo.h>
#include <xbeeImpl.h>

// Value safran
const int min_angle_safran = 70;
const int max_angle_safran = 170;
const int init_angle_safran = 125;
const int min_ms_safran = 960;
const int max_ms_safran = 1640;
const int init_safran = 1300;

// value sails
const int min_angle_sail = 0;
const int max_angle_sail = 100;
const int init_angle_sail = 100;
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

    // Control Parameters
    int currentTension = 0;
    int servoAnglePosition = 125;
    int voileTensionPosition = 100;
    int ms_safran_position;
    int ms_sail_position;

    // Variables for PI control
    float adjustment = 0.0;
    float cumulateError = 0.0;

public:
    servoControl();
    void servo_control(const xbeeImpl &xbee);
    int calculateShortestPath(int current, int target);

    // Getters for Control Parameters
    int getCurrentTension() const { return currentTension; }
    int getServoAnglePosition() const { return servoAnglePosition; }
    int getVoileTensionPosition() const { return voileTensionPosition; }
    int getSafranPosition() const { return ms_safran_position; }
    int getSailPosition() const { return ms_sail_position; }

    // Getters for PI Control Variables
    float getAdjustement() const { return adjustment; }
    float getCumulateError() const { return cumulateError; }
};

#endif // SERVO_CONTROL_H