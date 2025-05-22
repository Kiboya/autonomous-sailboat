#include <Arduino.h>

void xbee_setup();

void getValue(String receivedMessage);

uint32_t xbee_create_message(uint32_t message);

// void sendValue(int currentAngle, int targetAngle, int servoAnglePosition, Stream &output)

void run();