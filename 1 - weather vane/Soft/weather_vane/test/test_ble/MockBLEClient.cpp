#include "MockBLEClient.h"
#include <iostream>

MockBLEClient::MockBLEClient() : isConnected(false), serviceDiscovered(false), characteristicDiscovered(false) {}

bool MockBLEClient::connectToServer(const std::string& serverAddress) {
    if (serverAddress == "94:08:53:47:10:60") {
        isConnected = true;
        std::cout << "Mock: Successfully connected to server at " << serverAddress << std::endl;
        return true;
    }
    std::cout << "Mock: Failed to connect to server at " << serverAddress << std::endl;
    return false;
}

bool MockBLEClient::discoverService(const std::string& serviceUUID) {
    if (isConnected && serviceUUID == "4fafc201-1fb5-459e-8fcc-c5c9c331914b") {
        serviceDiscovered = true;
        std::cout << "Mock: Successfully discovered service with UUID " << serviceUUID << std::endl;
        return true;
    }
    std::cout << "Mock: Failed to discover service with UUID " << serviceUUID << std::endl;
    return false;
}

bool MockBLEClient::discoverCharacteristic(const std::string& characteristicUUID) {
    if (serviceDiscovered && characteristicUUID == "beb5483e-36e1-4688-b7f5-ea07361b26a8") {
        characteristicDiscovered = true;
        std::cout << "Mock: Successfully discovered characteristic with UUID " << characteristicUUID << std::endl;
        return true;
    }
    std::cout << "Mock: Failed to discover characteristic with UUID " << characteristicUUID << std::endl;
    return false;
}

bool MockBLEClient::writeToCharacteristic(const std::string& value) {
    if (characteristicDiscovered) {
        std::cout << "Mock: Successfully wrote value '" << value << "' to characteristic" << std::endl;
        return true;
    }
    std::cout << "Mock: Failed to write value '" << value << "' to characteristic" << std::endl;
    return false;
}

bool MockBLEClient::subscribeToNotifications() {
    if (characteristicDiscovered) {
        std::cout << "Mock: Successfully subscribed to notifications" << std::endl;
        return true;
    }
    std::cout << "Mock: Failed to subscribe to notifications" << std::endl;
    return false;
}