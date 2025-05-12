#include <Arduino.h>

void xbee_setup();

void xbee_get_value(String receivedMessage);

void xbee_run();

// void sendRTKData(String hexString);

String xbee_create_message(String message);