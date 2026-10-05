#include "ByteCode.h"

namespace ReliableUDP 
{

	void putUnsinged(unsigned char* destination, uint32_t value, int byteCount){

		for (int i = 0; i < byteCount; ++i) {

			const int shift = (byteCount - 1 - i) * Bits_Per_Byte;

			destination[i] = static_cast<unsigned char>((value >> shift) & Byte_Mask);

		}

	}

	uint64_t getUnsigned(const unsigned char* source, int byteCount) {

		uint64_t value = 0;

		for (int i = 0; i < byteCount; ++i) {

			value = (value << Bits_Per_Byte) | source[i];

		}

		return value;

	}

	void putU16(unsigned char* destination, uint32_t value) {

		putUnsinged(destination, value, U16_Bytes);

	}

	void putU32(unsigned char* destination, uint32_t value) {

		putUnsinged(destination, value, U32_Bytes);

	}

	void putU64(unsigned char* destination, uint64_t value) {

		putUnsinged(destination, value, U32_Bytes);

	}

	uint32_t getU16(const unsigned char* source) {

		return static_cast<uint32_t>(getUnsigned(source, U16_Bytes));

	}

	uint32_t getU32(const unsigned char* source) {

		return static_cast<uint32_t>(getUnsigned(source, U32_Bytes));

	}

	uint32_t getU64(const unsigned char* source) {

		return (getUnsigned(source, U64_Bytes));

	}

}