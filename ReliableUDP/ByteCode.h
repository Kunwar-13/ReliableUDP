#ifndef BYTECODE_H
#define BYTECODE_H

#include "TransferConstants.h"

namespace ReliableUDP 
{

	void putUnsinged(unsigned char* destination, uint32_t value, int byteCount);
	uint64_t getUnsigned(const unsigned char* source, int byteCount);

	void putU16(unsigned char* destination, uint32_t value);
	void putU32(unsigned char* destination, uint32_t value);
	void putU64(unsigned char* destination, uint64_t value);

	uint32_t getU16(const unsigned char* source);
	uint32_t getU32(const unsigned char* source);
	uint32_t getU64(const unsigned char* source);

}

#endif // !BYTECODE_H
