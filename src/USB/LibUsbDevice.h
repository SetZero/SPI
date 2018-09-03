//
// Created by sebastian on 03.09.18.
//

#pragma once

#include <memory>
#include <libusb.h>
#include <iostream>
#include "src/SPI/SPIBridge.h"
#include "USBUtils.h"

namespace usb {
    //TODO: Move this in own class, create HID device if detected from map


    class LibUSBDevice {
    public:
        LibUSBDevice(const VendorID& vendorID, const DeviceID& deviceID, libusb_device *device);
        virtual ~LibUSBDevice();
        //There should always only be one device with a given device handle (and it should be managed by the device list)!
        LibUSBDevice(const LibUSBDevice &other) = delete;
        LibUSBDevice &operator=(const LibUSBDevice &other) = delete;
        LibUSBDevice(LibUSBDevice &&other) noexcept = delete;
        LibUSBDevice &operator=(LibUSBDevice &&other) noexcept = delete;
        VendorID getVendorID() const;
        DeviceID getDeviceID() const;
        void openDevice();
        void closeDevice();

        //TODO: change this to something else, not spi::SPIData... (This is not yet a spi device, but more generic)
        virtual bool sendData(const spi::SPIData &data);

    protected:
        void _closeDevice();

        VendorID vendorID;
        DeviceID deviceID;
        libusb_device *device = nullptr;
        libusb_device_handle *handle = nullptr;
        bool isOpen = false;
    };

    inline namespace literals {
        DeviceID operator ""_did(unsigned long long element);
        VendorID operator ""_vid(unsigned long long element);
    }
}
