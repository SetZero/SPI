#include <utility>
#include <bitset>

#include "A4963.h"

namespace NS_A4963 {
    void A4963::clearRegister(const A4963::RegisterCodes &reg) {
        mRegisterData[reg].data = 0;
    }


    void A4963::clearRegister(const A4963::RegisterCodes &reg, const A4963::RegisterMask &mask) {
        mRegisterData[reg].data &= ~(static_cast<A4963::size_type>(mask));
    }

    std::unique_ptr<spi::Data> A4963::send16bitRegister(size_type address) {
        return mBridge->transfer(spi::SPIData<sizeof(size_type)>{address});
    }

    A4963::A4963(std::shared_ptr<spi::SPIBridge> mBridge) : mBridge(std::move(mBridge)) {
        //reload all register
        markRegisterForReload(A4963::RegisterCodes::Config0);
        markRegisterForReload(A4963::RegisterCodes::Config1);
        markRegisterForReload(A4963::RegisterCodes::Config2);
        markRegisterForReload(A4963::RegisterCodes::Config3);
        markRegisterForReload(A4963::RegisterCodes::Config4);
        markRegisterForReload(A4963::RegisterCodes::Config5);
        markRegisterForReload(A4963::RegisterCodes::Mask);
        markRegisterForReload(A4963::RegisterCodes::Run);
    }

    void A4963::writeRegisterEntry(const A4963::RegisterCodes &reg, const A4963::RegisterMask &mask, size_type data) {
        clearRegister(reg, RegisterMask::RegisterAndWriteAddress);
        clearRegister(reg, mask);

        mRegisterData[reg].data |= createRegisterEntry(reg, RegisterMask::RegisterAddress);
        mRegisterData[reg].data |= createRegisterEntry(WriteBit::Write, RegisterMask::WriteAddress);
        mRegisterData[reg].data |= createRegisterEntry(data, RegisterMask::GeneralData);
        mRegisterData[reg].dirty = true;
    }

    void A4963::markRegisterForReload(const A4963::RegisterCodes &reg) {
        clearRegister(reg);
        mRegisterData[reg].data |= createRegisterEntry(reg, RegisterMask::RegisterAddress);
        mRegisterData[reg].data |= createRegisterEntry(WriteBit::Read, RegisterMask::WriteAddress);
        mRegisterData[reg].dirty = true;
    }

    template<typename T>
    A4963::size_type A4963::createRegisterEntry(T data, const A4963::RegisterMask &mask) {
        A4963::size_type registerData = 0;
        registerData = (static_cast<size_type>(data) << utils::getFirstSetBitPos(static_cast<size_type>(mask))) &
                       static_cast<size_type>(mask);
        return registerData;
    }


    A4963::size_type
    A4963::getRegisterEntry(const A4963::RegisterCodes &registerEntry, const A4963::RegisterMask &mask) {
        A4963::size_type registerData = 0;
        registerData = (mRegisterData[registerEntry].data & static_cast<size_type>(mask))
                >> utils::getFirstSetBitPos(static_cast<size_type>(mask));
        return registerData;
    }

    void A4963::commit() {
        for (auto &data : mRegisterData) {
            commit(data.first);
        }
    }

    void A4963::commit(const A4963::RegisterCodes &registerCodes) {
        if (mRegisterData[registerCodes].dirty) {
            mBridge->slaveSelect(shared_from_this());
            if (getRegisterEntry(registerCodes, RegisterMask::WriteAddress) ==
                static_cast<A4963::size_type>(WriteBit::Read))
            {
                mRegisterData[registerCodes].data = createRegisterEntry(registerCodes, RegisterMask::RegisterAddress) |
                                                    (static_cast<A4963::size_type>(*send16bitRegister(
                                                            mRegisterData[registerCodes].data)) &
                                                     static_cast<A4963::size_type>(RegisterMask::GeneralData));
            } else {
                send16bitRegister(mRegisterData[registerCodes].data);
            }
            mBridge->slaveDeselect(shared_from_this());
            mRegisterData[registerCodes].dirty = false;
        }
    }

    void A4963::show_register() {
        for (auto registerData : mRegisterData) {
            std::bitset<16> set(readRegister(registerData.first));
            std::cout << static_cast<A4963::size_type>(registerData.first) << ": " << set << std::endl;
        }
    }

    A4963::size_type A4963::readRegister(const A4963::RegisterCodes &registerCodes, bool forceNoReload) {
        if (mRegisterData[registerCodes].dirty && forceNoReload) {
            commit(registerCodes);
        }
        return mRegisterData[registerCodes].data;
    }

    void A4963::setRecirculationMode(const A4963::RecirculationModeTypes &type) {
        setRegisterEntry(type,RegisterMask::RecirculationModeAddress,RegisterCodes::Config0);
    }


    void A4963::setPercentFastDecay(const A4963::PercentFastDecayTypes &type) {
        setRegisterEntry(type,RegisterMask::PercentFastDecayAddress,RegisterCodes::Config1);
    }

    void A4963::invertPWMInput(const A4963::InvertPWMInputTypes &type) {
        setRegisterEntry(type,RegisterMask::InvertPWMInputAddress,RegisterCodes::Config1);
    }

    void A4963::setBemfTimeQualifier(const A4963::BemfTimeQualifier &type) {
        setRegisterEntry(type,RegisterMask::BemfTimeQualifierAddress,RegisterCodes::Config1);
    }

    template<typename T>
    void
    A4963::setRegisterEntry(T data, const A4963::RegisterMask &mask, const A4963::RegisterCodes &registerEntry) {
        auto val = createRegisterEntry(data,mask);
        writeRegisterEntry(registerEntry,mask,val);
    }
}