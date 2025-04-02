#include "BLEClientImpl.hpp"

BleClientImpl::BleClientImpl() : p_client_(BLEDevice::createClient())
{
  Serial.println("Initializing BLE client...");
  BLEDevice::init("");
  Serial.println("BLE client initialized.");
}

auto BleClientImpl::ConnectToServer(const std::string& server_address) -> bool
{
  Serial.print("Attempting to connect to server at address: ");
  Serial.println(server_address.c_str());
  bool connected = false;
  connected = p_client_->connect(BLEAddress(server_address.c_str()));

  if (connected)
  {
    Serial.println("Successfully connected to the server.");
  }
  else
  {
    Serial.println("Failed to connect to the server.");
  }
  return connected;
}

auto BleClientImpl::DiscoverService(const std::string& service_uuid) -> bool
{
  bool service_discovered = false;

  Serial.print("Discovering service with UUID: ");
  Serial.println(service_uuid.c_str());
  p_remote_service_ = p_client_->getService(BLEUUID(service_uuid.c_str()));

  service_discovered = (p_remote_service_ != nullptr);
  if (service_discovered)
  {
    Serial.println("Service discovered successfully.");
  }
  else
  {
    Serial.println("Failed to discover the service.");
  }
  return service_discovered;
}

auto BleClientImpl::DiscoverCharacteristic(const std::string& characteristic_uuid) -> bool
{
  if (p_remote_service_ == nullptr)
  {
    Serial.println("Error: Remote service is null. Cannot discover characteristic.");
    return false;
  }

  Serial.print("Discovering characteristic with UUID: ");
  Serial.println(characteristic_uuid.c_str());
  p_remote_characteristic_ =
      p_remote_service_->getCharacteristic(BLEUUID(characteristic_uuid.c_str()));
  if (p_remote_characteristic_ != nullptr)
  {
    Serial.println("Characteristic discovered successfully.");
    return true;
  }

  Serial.println("Failed to discover the characteristic.");
  return false;
}

auto BleClientImpl::WriteToCharacteristic(const std::string& value) -> bool
{
  bool can_write = false;

  if (p_remote_characteristic_ == nullptr)
  {
    Serial.println("Error: Remote characteristic is null. Cannot write value.");
    return false;
  }

  Serial.print("Writing to characteristic: ");
  Serial.println(value.c_str());

  can_write = p_remote_characteristic_->canWrite();

  if (can_write)
  {
    p_remote_characteristic_->writeValue(value);
    Serial.println("Value written successfully.");
    return can_write;
  }

  Serial.println("Failed to write to the characteristic. Writing is not supported.");
  return can_write;
}

auto BleClientImpl::SubscribeToNotifications() -> bool
{
  if (p_remote_characteristic_ == nullptr)
  {
    Serial.println("Error: Remote characteristic is null. Cannot subscribe to notifications.");
    return false;
  }

  Serial.println("Subscribing to notifications...");
  if (p_remote_characteristic_->canNotify())
  {
    p_remote_characteristic_->registerForNotify(
        [](BLERemoteCharacteristic* p_ble_remote_characteristic,
           uint8_t* p_data,
           size_t length,
           bool is_notify)
        {
          Serial.print("Notification received: ");
          for (size_t i = 0; i < length; i++)
          {
            Serial.print((char)p_data[i]);
          }
          Serial.println();
        });
    Serial.println("Subscribed to notifications successfully.");
    return true;
  }
  Serial.println("Failed to subscribe to notifications. Notifications are not supported.");
  return false;
}