#include "BLEClientImpl.h"

BLEClientImpl::BLEClientImpl() {
    Serial.println("Initializing BLE client...");
    BLEDevice::init("");
    pClient = BLEDevice::createClient();
    Serial.println("BLE client initialized.");
}

bool BLEClientImpl::connectToServer(const std::string& serverAddress) {
    Serial.print("Attempting to connect to server at address: ");
    Serial.println(serverAddress.c_str());
    bool connected = pClient->connect(BLEAddress(serverAddress.c_str()));
    if (connected) {
        Serial.println("Successfully connected to the server.");
    } else {
        Serial.println("Failed to connect to the server.");
    }
    return connected;
}

bool BLEClientImpl::discoverService(const std::string& serviceUUID) {
    Serial.print("Discovering service with UUID: ");
    Serial.println(serviceUUID.c_str());
    pRemoteService = pClient->getService(BLEUUID(serviceUUID.c_str()));
    if (pRemoteService) {
        Serial.println("Service discovered successfully.");
        return true;
    } else {
        Serial.println("Failed to discover the service.");
        return false;
    }
}

bool BLEClientImpl::discoverCharacteristic(const std::string& characteristicUUID) {
    if (!pRemoteService) {
        Serial.println("Error: Remote service is null. Cannot discover characteristic.");
        return false;
    }

    Serial.print("Discovering characteristic with UUID: ");
    Serial.println(characteristicUUID.c_str());
    pRemoteCharacteristic = pRemoteService->getCharacteristic(BLEUUID(characteristicUUID.c_str()));
    if (pRemoteCharacteristic) {
        Serial.println("Characteristic discovered successfully.");
        return true;
    } else {
        Serial.println("Failed to discover the characteristic.");
        return false;
    }
}

bool BLEClientImpl::writeToCharacteristic(const std::string& value) {
    if (!pRemoteCharacteristic) {
        Serial.println("Error: Remote characteristic is null. Cannot write value.");
        return false;
    }

    Serial.print("Writing to characteristic: ");
    Serial.println(value.c_str());
    if (pRemoteCharacteristic->canWrite()) {
        pRemoteCharacteristic->writeValue(value);
        Serial.println("Value written successfully.");
        return true;
    } else {
        Serial.println("Failed to write to the characteristic. Writing is not supported.");
        return false;
    }
}

bool BLEClientImpl::subscribeToNotifications() {
    if (!pRemoteCharacteristic) {
        Serial.println("Error: Remote characteristic is null. Cannot subscribe to notifications.");
        return false;
    }

    Serial.println("Subscribing to notifications...");
    if (pRemoteCharacteristic->canNotify()) {
        pRemoteCharacteristic->registerForNotify([](BLERemoteCharacteristic* pBLERemoteCharacteristic,
                                                    uint8_t* pData, size_t length, bool isNotify) {
            Serial.print("Notification received: ");
            for (size_t i = 0; i < length; i++) {
                Serial.print((char)pData[i]);
            }
            Serial.println();
        });
        Serial.println("Subscribed to notifications successfully.");
        return true;
    } else {
        Serial.println("Failed to subscribe to notifications. Notifications are not supported.");
        return false;
    }
}