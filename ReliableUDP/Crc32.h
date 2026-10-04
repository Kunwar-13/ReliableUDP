#ifndef CRC32_H
#define CRC32_H

#include "TransferConstants.h"

namespace ReliableUDP {

	/*
	* TITLE : Table-driven CRC-32 algorithm
	* AUTHOR : P. Deutsch (RFC 1952 section 8); also the PNG specification, Annex D
	* DATE : <<YYYY-MM-DD the algorithm description was obtained>>
	* VERSION : RFC 1952, GZIP file format specification version 4.3
	* AVAILABILITY : https://www.rfc-editor.org/rfc/rfc1952
	*
	* The code in Crc32.cpp was written for this assignment from that description.
	* Polynomial 0xEDB88320 (reflected). Check value: CRC-32("123456789") = 0xCBF43926.
	*/

	class Crc32
	{
	public:

		static uint32_t Update(uint32_t crc, const unsigned char* data, size_t length);
		static bool SelfTest();

	private:

		static const uint32_t* Table();
	};

	bool fileCrc32(const std::string& path, uint32_t& crc, uint64_t& size);

}

#endif // !CRC32_H
