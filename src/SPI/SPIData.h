//
// Created by keven on 22.09.18.
//

#pragma once

#pragma once
#include <stdint.h>
#include "../utils/utils.h"
#include <vector>
#include <string>
#include <exception>
#include <byteswap.h>
#include <assert.h>
#include <memory>
#include <utility>
#include <iostream>


namespace spi {

	template<typename t>
	inline auto swapEndian(t value)
	{
		if constexpr (utils::isEqual<t, uint8_t>::value || utils::isEqual<t,int8_t>::value) return value;
		else if constexpr (utils::isEqual<t,uint16_t>::value|| utils::isEqual<t,int16_t>::value) return bswap_16(value);
		else if constexpr (utils::isEqual<t,uint32_t>::value|| utils::isEqual<t,int32_t>::value) return bswap_32(value);
		else /*if constexpr (utils::isEqual<t,uint64_t>::value|| utils::isEqual<t,int64_t>::value)*/ return bswap_64(value);
	}

	class SPI_Exception : public std::exception {
		const char* info;
	public:
		inline explicit SPI_Exception(const char* what) : info(what){

		};

		inline const char *what() const noexcept override {
			return info;
		}
	};

	class Data {
	protected:
		std::vector<uint8_t> mData = std::vector<uint8_t>();
	public:

		Data() = default;

		Data(const Data&) = default;


		const std::vector<uint8_t>& data() {return mData;};

		virtual void swap(Data& other) { std::swap(mData,other.mData);};

		inline uint8_t operator[](const uint8_t& index) const {
			return mData[index];
		}

		inline uint8_t& operator[](const uint8_t& index) {
			return mData[index];
		}

		auto begin() const {
			return mData.begin();
		}

		auto end() const {
			return mData.end();
		}

        virtual void operator+=(const Data& rhs) = 0;

        virtual void operator+=(uint8_t data) = 0;

		virtual explicit operator uint8_t () const = 0;

		virtual explicit operator uint16_t() const = 0;

		virtual explicit operator uint32_t() const = 0;

        virtual explicit operator uint64_t() const = 0;
        //virtual copy constructor
        virtual std::unique_ptr<Data> clone() const = 0;
        //virtual copy constructor
        virtual std::unique_ptr<Data> create() const = 0;

        inline void operator-(const Data& rhs) = delete;
        inline void operator*(const Data& rhs) = delete;
        inline void operator/(const Data& rhs) = delete;
        inline void operator-=(const Data& rhs) = delete;
        inline void operator*=(const Data& rhs) = delete;
        inline void operator/=(const Data& rhs) = delete;

		inline uint8_t bytesUsed() const { return static_cast<uint8_t>(mData.size()); };

		virtual ~Data() = default;
	};

	enum EndianMode{
		little_endian, //small letters because macro is already named this way
		big_endian
	};

	template<unsigned char numberOfBytes = 1, EndianMode endian = little_endian >
	class SPIData : public Data {
		static_assert((numberOfBytes &(numberOfBytes -1)) == 0  , " the number of bytes have to be a pow of 2");
		static_assert(numberOfBytes != 0, " 0 means no data, so this is not possible");
	public:

		SPIData() {mData.reserve(numberOfBytes);};

		template<typename T, typename ... args>
		explicit SPIData(T first,args ... ss) : SPIData() {
            static constexpr uint8_t bytesSum = (sizeof...(args)+1) * sizeof(T);
            static_assert(utils::sameTypes<T,args...>(),"there are different types in the constructor, this is not allowed");
			static_assert(bytesSum <= numberOfBytes, "too much bytes for this data type" );
            static_assert(std::is_integral<T>::value,"the data type have to be a integral type");
			auto ins = {first, ss...};
            for(auto elem : ins){
                if constexpr(endian == big_endian)
                    for(uint8_t i = sizeof(T); i > 0; i--)
                        mData.emplace_back(static_cast<uint8_t>(elem >> ((i-1) * 8)));
                else for(uint8_t i = 0; i < sizeof(T); i++)
                        mData.emplace_back(static_cast<uint8_t>(elem >> (i * 8)));
            }
		}


		explicit SPIData(const Data& other) : SPIData() {
			mData.insert(std::end(mData), std::begin(other), std::end(other));
		}

		explicit SPIData(const std::vector<uint8_t >& other) : SPIData() {
			mData.insert(std::end(mData), std::begin(other), std::end(other));
		}

		inline void swap(Data& other) override{
			other.swap(*this);
		}

		inline std::unique_ptr<Data> clone() const override {
            return std::make_unique<SPIData>(*this);
		}
        inline std::unique_ptr<Data> create() const override {
            return std::make_unique<SPIData>();
        }

		inline void operator+=(const Data& rhs) override {
            for (size_t i = 0; i < rhs.bytesUsed(); i++) {
                if (mData.size() < numberOfBytes) {
                    mData.emplace_back(rhs[i]);
                } else {
                    throw SPI_Exception{"Data Overflow"};
                }
            }
		};

		inline void operator+=(uint8_t data) override{
			if (mData.size() < numberOfBytes)
				mData.push_back(data);
			else
				throw SPI_Exception{"Data Overflow"};
		}


		explicit operator uint8_t () const override {
            if (mData.size() > 1) throw SPI_Exception{"SPIData did not fit into a uint8_t type"};
			return mData[0];
		}

		explicit operator uint16_t() const override {
            if (mData.size() > 2) throw SPI_Exception{"SPIData did not fit into a uint16_t type"};
            uint16_t erg = (mData[1] << 8) | mData[0];
            return erg;
		}

		explicit operator uint32_t() const override {
            if (mData.size() > 4) throw SPI_Exception{"SPIData did not fit into a uint32_t type"};
            uint32_t erg = (mData[3] << 24) | (mData[2] << 16) | (mData[1] << 8) | mData[0];
            return erg;
            /*if constexpr (endian == big_endian)
                return swapEndian<uint32_t>(erg);
            else
                return erg;*/
		}

		explicit operator uint64_t() const override {
            if (mData.size() > 8) throw SPI_Exception{"SPIData did not fit into a uint64_t type"};
            uint64_t erg = ((uint64_t)(mData[7]) << 56) | ((uint64_t)(mData[6]) << 48) | ((uint64_t)(mData[5]) << 40) | ((uint64_t)(mData[4]) << 32)
                           |(mData[3] << 24) | (mData[2] << 16) | (mData[1] << 8)  | mData[0];
            return erg;
		}


		inline void operator-(const SPIData& rhs) = delete;
		inline void operator*(const SPIData& rhs) = delete;
		inline void operator/(const SPIData& rhs) = delete;
		inline void operator+(const SPIData& rhs) = delete;
        inline void operator-=(const SPIData& rhs) = delete;
        inline void operator*=(const SPIData& rhs) = delete;
        inline void operator/=(const SPIData& rhs) = delete;

		~SPIData() override = default;
	};

	inline std::ostream &operator<<(std::ostream &os, const Data &data) {
		uint32_t i = 0;
		for(auto& elem: data) {
			os << "data " << i << ": " << std::to_string(elem) <<" ";
			i++;
		}
		os << std::endl;
		return os;
	}

	inline std::unique_ptr<Data> operator+(const Data& lhs,const Data& rhs) {
	    auto tmp = lhs.clone();
	    *tmp += rhs;
		return	tmp;
	 }

	inline void swap(Data& lhs, Data& rhs) {
		lhs.swap(rhs);
	}

	inline namespace literals {

        inline std::unique_ptr<Data> operator ""_spi8(unsigned long long element) {
            return std::make_unique<SPIData<>>(static_cast<uint8_t>(element));
        }
        inline std::unique_ptr<Data> operator ""_spi16(unsigned long long element) {
            return std::make_unique<SPIData<2>>(static_cast<uint16_t>(element));
        }
        inline std::unique_ptr<Data> operator ""_spi32(unsigned long long element) {
            return std::make_unique<SPIData<4>>(static_cast<uint32_t>(element));
        }
        inline std::unique_ptr<Data> operator ""_spi64(unsigned long long element) {
            return std::make_unique<SPIData<8>>(static_cast<uint64_t>(element));
        }
        inline std::unique_ptr<Data> operator ""_spi8_big(unsigned long long element) {
            return std::make_unique<SPIData<1,big_endian>>(static_cast<uint8_t>(element));
        }
        inline std::unique_ptr<Data> operator ""_spi16_big(unsigned long long element) {
            return std::make_unique<SPIData<2,big_endian>>(static_cast<uint16_t>(element));
        }
        inline std::unique_ptr<Data> operator ""_spi32_big(unsigned long long element) {
            return std::make_unique<SPIData<4,big_endian>>(static_cast<uint32_t>(element));
        }
        inline std::unique_ptr<Data> operator ""_spi64_big(unsigned long long element) {
            return std::make_unique<SPIData<8,big_endian>>(static_cast<uint64_t>(element));
        }
    }

}
