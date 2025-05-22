#!/usr/bin/env python3

"""
Example for a BLE 4.0 Server
"""
import sys
import logging
import asyncio
import threading

from typing import Any, Union

from bless import (  # type: ignore
    BlessServer,
    BlessGATTCharacteristic,
    GATTCharacteristicProperties,
    GATTAttributePermissions,
)

logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(name=__name__)

# NOTE: Some systems require different synchronization methods.
trigger: Union[asyncio.Event, threading.Event]
if sys.platform in ["darwin", "win32"]:
    trigger = threading.Event()
else:
    trigger = asyncio.Event()


def read_request(characteristic: BlessGATTCharacteristic, **kwargs) -> bytearray:
    logger.debug(f"Read request received for characteristic {characteristic.uuid}")
    logger.debug(f"Current value: {characteristic.value}")
    return characteristic.value


def write_request(characteristic: BlessGATTCharacteristic, value: Any, **kwargs):
    logger.debug(f"Write request received for characteristic {characteristic.uuid}")
    logger.debug(f"New value: {value}")
    characteristic.value = value
    logger.debug(f"Characteristic value updated to: {characteristic.value}")
    if characteristic.value == b"\x0f":
        logger.debug("Trigger condition met (value is 0x0F).")
        trigger.set()


async def run(loop):
    logger.info("Starting BLE server...")
    trigger.clear()

    # Instantiate the server
    my_service_name = "test_service"
    server = BlessServer(name=my_service_name, loop=loop)
    logger.info(f"BLE server initialized with name: {my_service_name}")

    server.read_request_func = read_request
    server.write_request_func = write_request

    # Add Service
    my_service_uuid = "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
    logger.info(f"Adding service with UUID: {my_service_uuid}")
    await server.add_new_service(my_service_uuid)

    # Add a Characteristic to the service
    my_char_uuid = "beb5483e-36e1-4688-b7f5-ea07361b26a8"
    char_flags = (
        GATTCharacteristicProperties.read
        | GATTCharacteristicProperties.write
        | GATTCharacteristicProperties.indicate
    )
    permissions = GATTAttributePermissions.readable | GATTAttributePermissions.writeable
    logger.info(f"Adding characteristic with UUID: {my_char_uuid}")
    await server.add_new_characteristic(
        my_service_uuid, my_char_uuid, char_flags, None, permissions
    )

    logger.debug(f"Characteristic added: {server.get_characteristic(my_char_uuid)}")
    await server.start()
    logger.info("BLE server is now advertising.")

    if trigger.__module__ == "threading":
        logger.debug("Using threading for synchronization.")
        trigger.wait()
    else:
        logger.debug("Using asyncio for synchronization.")
        await trigger.wait()

    logger.info("Trigger condition met. Proceeding with updates.")
    await asyncio.sleep(2)
    logger.debug("Updating characteristic value...")
    server.get_characteristic(my_char_uuid)
    server.update_value(my_service_uuid, my_char_uuid)
    logger.debug("Characteristic value updated.")
    await asyncio.sleep(5)

    logger.info("Stopping BLE server...")
    await server.stop()
    logger.info("BLE server stopped.")


loop = asyncio.get_event_loop()
loop.run_until_complete(run(loop))