#include "mcp2210_hal.h"
#include "utils.h"
//#define DEBUG_MCP
/*use udev or other similar mechanisms to get the system path "/dev/hidraw1" */

//auto try up to number 9
MCP2210::MCP2210() {
    //TODO: find out witch hid number is the right for the device
    std::string device = "/dev/hidraw";
    std::unique_ptr<stChipStatus_T> x = std::make_unique<stChipStatus_T>();

    unsigned char z = '0';
    for(int z = '0'; z < 58; z++) {
        device.push_back(z);
#ifdef DEBUG_MCP
        std::cout << "Debug dev_str: " << device << std::endl;
#endif
        fd = open_device(device.c_str());
        if(fd > 0){
            if(get_chip_status(fd,x.get()) == ERR_NOERR) {
                std::cout<< "connection established with device: " << z-48 << std::endl;
                connection = true;
                break;
            }
        }
        device.pop_back();
    }
    if(fd <= 0) std::cout << "Auto detect could not find your device, try manually " << fd << std::endl;
    else{
    gpio_setdir(fd,0);
    gpio_setval(fd,0);}
}

//use ls /dev grep | "hid" to find devices, then give the correct number of the spi bridge
MCP2210::MCP2210(char number) {
    std::string device = "/dev/hidraw";
    //TODO: find out witch hid number is the right for the device
    device.push_back(number);
    fd = open_device(device.c_str());
    if(fd <= 0) std::cout << "Error with device: " << fd << std::endl;
    else{
        connection = true;
        std::cout << "Device successfully opened" << std::endl;
    }
}

MCP2210::~MCP2210() {
    close_device(fd);
}

spi::SPIData MCP2210::transfer(const spi::SPIData& input) const {
    //txdata[0] = input;
    int err = 0;
    std::vector<unsigned char> temp = input.getData();
    int i = 0;
    for(unsigned char elem: temp){
        if(i >= SPI_BUF_LEN){
            err = 42;
            break;
        };
        txdata[i] = elem;
        i++;
    }
    if(err == 0)
            err = spi_data_xfer(fd, txdata.get(), rxdata.get(), static_cast<unsigned char>(temp.size()),
            static_cast<uint16_t >(spiSettings::mode), static_cast<uint16_t >(spiSettings::speed), static_cast<uint16_t >(spiSettings::actcsval),
            static_cast<uint16_t >(spiSettings::idlecsval), static_cast<uint16_t >(spiSettings::gpcsmask), static_cast<uint16_t >(spiSettings::cs2datadly),
            static_cast<uint16_t >(spiSettings::data2datadly), static_cast<uint16_t >(spiSettings::data2csdly));
    if(err != 0) std::cout << " error: " << err << std::endl;
    temp.clear();
    for(i = 0; i < SPI_BUF_LEN ; i++)
        temp.emplace_back(rxdata[i]);
    return spi::SPIData( temp );
}

void MCP2210::writeGPIO(const gpio::gpioState& state, const gpio::GPIOPin& pin) {
    if(state == gpio::gpioState::off) {
        gpio_write(fd, ~pin, pin);
    } else {
        gpio_write(fd, pin, pin);
    }
}

void MCP2210::setGPIODirection(const gpio::gpioDirection& direction, const gpio::GPIOPin& pin) {
    if(direction == gpio::gpioDirection::in) {
        gpio_direction(fd, 0x01FF, pin);
    } else if(direction == gpio::gpioDirection::out) {
        gpio_direction(fd, 0x0000, pin);
    }
}

void MCP2210::slaveSelect(const SPIDevice& slave) {
    writeGPIO(gpio::gpioState::off, slave.getSlavePin());
}

void MCP2210::slaveDeselect(const SPIDevice& slave) {
    writeGPIO(gpio::gpioState::on, slave.getSlavePin());
}

void MCP2210::slaveRegister(SPIDevice& device, const gpio::GPIOPin& pin) {
    device.selectPin(pin);
}

gpio::gpioState MCP2210::readGPIO(const gpio::GPIOPin &pin) const {
    return pin == 1 ? gpio::gpioState::on : gpio::gpioState::off;
}

int MCP2210::getFd() const {
    return fd;
}