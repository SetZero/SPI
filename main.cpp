#include <iostream>
#include <array>
#include <string>
#include <memory>
#include <vector>
#include "src/SPIBridge.h"
#include "mcp2210_api.h"
#include "src/mcp2210_hal.h"
#include "src/25LC256.h"
#include "ATmega32U4SPI.h"

int main(int argc, char **argv) {
    int ictr;

    std::cout << "\nMCP2210 Evaluation Board Tests" << std::endl;
    std::cout << "\nParameters: " << argc << std::endl;
    for (ictr = 0; ictr < argc; ictr++)
	{
		std::cout << "\nParameter(" << ictr << ") -> " << argv[ictr] << std::endl;
	}

    if(argc > 1) {
        std::string str = argv[1];
        std::cout << "Trying to open: " << argv[1] << std::endl;

        std::unique_ptr<spi::SPIBridge> bridge = std::make_unique<MCP2210>(str);
        EEPROM eeprom{bridge};
        std::cout << "Status: " << std::hex << eeprom.readStatus() << std::endl;
    } else {
        using namespace spi::literals;

        std::cout << "Starting Atmega32u4..." << std::endl;
        LibUSBDeviceList deviceList;
        std::cout << "Found " << deviceList.getDevices().size() << " devices" << std::endl;
        if(auto atmega = deviceList.findDevice(ATmega32u4SPI::vendorID, ATmega32u4SPI::deviceID)) {
            std::cout << "One of them was the Atmega!" << std::endl;
            ATmega32u4SPI spi{*atmega};
            spi::SPIData data1 = spi.transfer(0x82_spi);
            spi::SPIData data2 = spi.transfer(0x83_spi);
            spi::SPIData data3 = spi.transfer(0x84_spi);
        }
    }
}



