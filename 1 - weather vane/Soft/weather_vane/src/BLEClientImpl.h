#ifndef BLE_CLIENT_IMPL_H
#define BLE_CLIENT_IMPL_H

#include <Arduino.h>
#include "BLEClientInterface.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>

class BLEClientImpl : public BLEClientInterface {
private:
    BLEClient* pClient;
    BLERemoteService* pRemoteService;
    BLERemoteCharacteristic* pRemoteCharacteristic;

public:
    BLEClientImpl();
    bool connectToServer(const std::string& serverAddress) override;
    bool discoverService(const std::string& serviceUUID) override;
    bool discoverCharacteristic(const std::string& characteristicUUID) override;
    bool writeToCharacteristic(const std::string& value) override;
    bool subscribeToNotifications() override;
};

#endif // BLE_CLIENT_IMPL_H