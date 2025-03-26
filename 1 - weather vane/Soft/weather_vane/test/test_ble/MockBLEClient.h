#ifndef MOCK_BLE_CLIENT_H
#define MOCK_BLE_CLIENT_H

#include "../src/BLEClientInterface.h"
#include <string>

class MockBLEClient : public BLEClientInterface {
private:
    bool isConnected;
    bool serviceDiscovered;
    bool characteristicDiscovered;

public:
    MockBLEClient();

    bool connectToServer(const std::string& serverAddress) override;
    bool discoverService(const std::string& serviceUUID) override;
    bool discoverCharacteristic(const std::string& characteristicUUID) override;
    bool writeToCharacteristic(const std::string& value) override;
    bool subscribeToNotifications() override;
};

#endif // MOCK_BLE_CLIENT_H