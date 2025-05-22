#ifndef BLE_CLIENT_INTERFACE_H
#define BLE_CLIENT_INTERFACE_H

#include <string>

class BLEClientInterface {
public:
    BLEClientInterface() = default;
    BLEClientInterface(const BLEClientInterface&) = delete;
    auto operator=(const BLEClientInterface&) -> BLEClientInterface& = delete;
    BLEClientInterface(BLEClientInterface&&) = delete;
    auto operator=(BLEClientInterface&&) -> BLEClientInterface& = delete;
    virtual ~BLEClientInterface() = default;
    virtual auto ConnectToServer(const std::string& server_address) -> bool = 0;
    virtual auto DiscoverService(const std::string& service_uuid) -> bool = 0;
    virtual auto DiscoverCharacteristic(const std::string& characteristic_uuid) -> bool = 0;
    virtual auto WriteToCharacteristic(const std::string& value) -> bool = 0;
    virtual auto SubscribeToNotifications() -> bool = 0;
};

#endif // BLE_CLIENT_INTERFACE_H