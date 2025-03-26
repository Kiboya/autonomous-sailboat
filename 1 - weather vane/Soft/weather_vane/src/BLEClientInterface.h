#ifndef BLE_CLIENT_INTERFACE_H
#define BLE_CLIENT_INTERFACE_H

#include <string>

class BLEClientInterface {
public:
    virtual ~BLEClientInterface() = default;

    virtual bool connectToServer(const std::string& serverAddress) = 0;
    virtual bool discoverService(const std::string& serviceUUID) = 0;
    virtual bool discoverCharacteristic(const std::string& characteristicUUID) = 0;
    virtual bool writeToCharacteristic(const std::string& value) = 0;
    virtual bool subscribeToNotifications() = 0;
};

#endif // BLE_CLIENT_INTERFACE_H