//
// Created by keven on 30.09.18.
//

#include "spi16.h"

std::shared_ptr<spi::SPIData> spi::spi16::operator+(const spi::SPIData &rhs) const {
    auto temp = std::make_shared<spi16>(*this);
    for (size_t i = 0; i < rhs.bytesUsed(); i++) {
        if (temp->mData.size() < numberOfBytes) {
            temp->mData.push_back(rhs[i]);
            //ss
        } else {
            throw std::exception{};
        }
    }
    return temp;
}

spi::spi16::spi16() : SPIData(BYTES) {
}

spi::spi16::spi16(const spi16 &data) = default;

spi::spi16::spi16(const std::initializer_list<uint8_t> &data) : SPIData(data,BYTES) {

}

spi::spi16::spi16(const uint16_t &data) : spi16({static_cast<uint8_t>(data),static_cast<uint8_t>(data >> 8)}) {
}

spi::spi16::spi16(const SPIData &other) : SPIData(other) {
}


spi::spi16::~spi16() = default;