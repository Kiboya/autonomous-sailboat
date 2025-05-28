#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <Arduino.h>
#include <Servo.h>
#include <xbeeImpl.h>

// Value safran
const int min_angle_safran = 70;
const int max_angle_safran = 170;
const int min_ms_safran = 1260;
const int max_ms_safran = 1740;
const int init_safran = 1500;

// value sails
const int min_ms_sail = 1200;
const int max_ms_sail = 1780;
const int init_sail = 1700;

const int safranPin = 5;
const int sailPin = 28;

class servoControl
{
private:
    Servo safranServo;
    Servo sailServo;

    // Control Parameters
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
    int getServoAnglePosition() const { return servoAnglePosition; }
    int getVoileTensionPosition() const { return voileTensionPosition; }
    int getSafranPosition() const { return ms_safran_position; }
    int getSailPosition() const { return ms_sail_position; }

    // Getters for PI Control Variables
    float getAdjustement() const { return adjustment; }
    float getCumulateError() const { return cumulateError; }
};

#endif // SERVO_CONTROL_H