#include <Arduino.h>
#include "BLEClientImpl.h" // Include the BLEClientImpl header

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b" // UUID of the test_service
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8" // UUID of the characteristic

void setup() {
  Serial.begin(115200);
  Serial.println("Initializing BLE...");

  // Create an instance of BLEClientImpl
  BLEClientImpl bleClient;

  // Connect to the BLE server
  if (!bleClient.connectToServer("94:08:53:47:10:60")) {
    Serial.println("Failed to connect to the server.");
    return;
  }
  Serial.println("Connected to the server.");

  // Discover the service
  if (!bleClient.discoverService(SERVICE_UUID)) {
    Serial.println("Failed to find the service.");
    return;
  }
  Serial.println("Service found.");

  // Discover the characteristic
  if (!bleClient.discoverCharacteristic(CHARACTERISTIC_UUID)) {
    Serial.println("Failed to find the characteristic.");
    return;
  }
  Serial.println("Characteristic found.");

  // Subscribe to notifications
  if (bleClient.subscribeToNotifications()) {
    Serial.println("Subscribed to notifications.");
  } else {
    Serial.println("Failed to subscribe to notifications.");
  }

  // Write to the characteristic
  if (bleClient.writeToCharacteristic("Hello from ESP32-C3!")) {
    Serial.println("Value written to the characteristic.");
  } else {
    Serial.println("Failed to write to the characteristic.");
  }
}

void loop() {
  // Keep the loop running to handle notifications
  delay(1000);
}