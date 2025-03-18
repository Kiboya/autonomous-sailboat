#include <Arduino.h>

void xbee_setup();

void servo_control();

int getCurrentAngleFromNorth();

int simulateMovingBoat(int angleDifference);

int calculateShortestPath(int current, int target);

void getValue(String receivedMessage);