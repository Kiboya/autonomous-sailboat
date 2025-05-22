#include <Arduino.h>

#include "BLEClientImpl.hpp"  // Include the BleClientImpl header

constexpr std::array<char, 37> kServiceUuid = { "4fafc201-1fb5-459e-8fcc-c5c9c331914b" };
constexpr std::array<char, 37> kCharacteristicUuid = { "beb5483e-36e1-4688-b7f5-ea07361b26a8" };

void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing BLE...");

  // Create an instance of BleClientImpl
  BleClientImpl ble_client;

  // Connect to the BLE server
  if (!ble_client.ConnectToServer("94:08:53:47:10:60"))
  {
    Serial.println("Failed to connect to the server.");
    return;
  }
  Serial.println("Connected to the server.");

  // Discover the service
  if (!ble_client.DiscoverService(kServiceUuid.data()))
  {
    Serial.println("Failed to find the service.");
    return;
  }
  Serial.println("Service found.");

  // Discover the characteristic
  if (!ble_client.DiscoverCharacteristic(kCharacteristicUuid.data()))
  {
    Serial.println("Failed to find the characteristic.");
    return;
  }
  Serial.println("Characteristic found.");

  // Subscribe to notifications
  if (ble_client.SubscribeToNotifications())
  {
    Serial.println("Subscribed to notifications.");
  }
  else
  {
    Serial.println("Failed to subscribe to notifications.");
  }

  // Write to the characteristic
  if (ble_client.WriteToCharacteristic("Hello from ESP32-C3!"))
  {
    Serial.println("Value written to the characteristic.");
  }
  else
  {
    Serial.println("Failed to write to the characteristic.");
  }
}

void loop()
{
  // Keep the loop running to handle notifications
  while (true)
    ;
}