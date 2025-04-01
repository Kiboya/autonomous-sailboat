#include <Arduino.h>
#include <unity.h>

#include "MockBLEClient.h"

// Constants for testing
const char* VALID_SERVER_ADDRESS = "94:08:53:47:10:60";
const char* INVALID_SERVER_ADDRESS = "invalid_address";

const char* VALID_SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
const char* INVALID_SERVICE_UUID = "invalid_service_uuid";

const char* VALID_CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";
const char* INVALID_CHARACTERISTIC_UUID = "invalid_characteristic_uuid";

const char* TEST_WRITE_VALUE = "Hello from ESP32-C3!";
const char* INVALID_WRITE_VALUE = "Invalid value";

// Global variables
String STR_TO_TEST;
MockBLEClient mockClient;

void setUp(void)
{
  // Initialize global variables and reset the mock client
  STR_TO_TEST = "Test String";
  mockClient = MockBLEClient();  // Reset the mock client state
}

void tearDown(void)
{
  // Clean up or reset global variables
  STR_TO_TEST = "";
}

void test_connectToServer_success(void)
{
  TEST_ASSERT_TRUE(mockClient.connectToServer(VALID_SERVER_ADDRESS));
}

void test_connectToServer_failure(void)
{
  TEST_ASSERT_FALSE(mockClient.connectToServer(INVALID_SERVER_ADDRESS));
}

void test_discoverService_success(void)
{
  mockClient.connectToServer(VALID_SERVER_ADDRESS);
  TEST_ASSERT_TRUE(mockClient.discoverService(VALID_SERVICE_UUID));
}

void test_discoverService_failure(void)
{
  mockClient.connectToServer(VALID_SERVER_ADDRESS);
  TEST_ASSERT_FALSE(mockClient.discoverService(INVALID_SERVICE_UUID));
}

void test_discoverCharacteristic_success(void)
{
  mockClient.connectToServer(VALID_SERVER_ADDRESS);
  mockClient.discoverService(VALID_SERVICE_UUID);
  TEST_ASSERT_TRUE(mockClient.discoverCharacteristic(VALID_CHARACTERISTIC_UUID));
}

void test_discoverCharacteristic_failure(void)
{
  mockClient.connectToServer(VALID_SERVER_ADDRESS);
  mockClient.discoverService(VALID_SERVICE_UUID);
  TEST_ASSERT_FALSE(mockClient.discoverCharacteristic(INVALID_CHARACTERISTIC_UUID));
}

void test_writeToCharacteristic_success(void)
{
  mockClient.connectToServer(VALID_SERVER_ADDRESS);
  mockClient.discoverService(VALID_SERVICE_UUID);
  mockClient.discoverCharacteristic(VALID_CHARACTERISTIC_UUID);
  TEST_ASSERT_TRUE(mockClient.writeToCharacteristic(TEST_WRITE_VALUE));
}

void test_writeToCharacteristic_failure(void)
{
  TEST_ASSERT_FALSE(mockClient.writeToCharacteristic(INVALID_WRITE_VALUE));
}

void setup()
{
  delay(2000);  // Service delay
  UNITY_BEGIN();

  RUN_TEST(test_connectToServer_success);
  RUN_TEST(test_connectToServer_failure);
  RUN_TEST(test_discoverService_success);
  RUN_TEST(test_discoverService_failure);
  RUN_TEST(test_discoverCharacteristic_success);
  RUN_TEST(test_discoverCharacteristic_failure);
  RUN_TEST(test_writeToCharacteristic_success);
  RUN_TEST(test_writeToCharacteristic_failure);

  UNITY_END();
}

void loop()
{
  // Empty loop
}