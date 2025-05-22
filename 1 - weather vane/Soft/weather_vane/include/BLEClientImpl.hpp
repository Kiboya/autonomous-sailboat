#ifndef BLE_CLIENT_IMPL_H
#define BLE_CLIENT_IMPL_H

#include <Arduino.h>
#include "BLEClientInterface.hpp"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>

class BleClientImpl : public BLEClientInterface {
private:
    BLEClient* p_client_ = nullptr;
    BLERemoteService* p_remote_service_ = nullptr;
    BLERemoteCharacteristic* p_remote_characteristic_ = nullptr;

   public:
    BleClientImpl();
    auto ConnectToServer(const std::string& server_address) -> bool override;
    auto DiscoverService(const std::string& service_uuid) -> bool override;
    auto DiscoverCharacteristic(const std::string& characteristic_uuid) -> bool override;
    auto WriteToCharacteristic(const std::string& value) -> bool override;
    auto SubscribeToNotifications() -> bool override;
};

#endif // BLE_CLIENT_IMPL_H