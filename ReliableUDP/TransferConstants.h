#ifndef TRANSFER_CONSTANTS_H
#define TRANSFER_CONSTANTS_H

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include "Net.h"

namespace ReliableUDP 
{
	const int      Default_Burst = 8;
	const int	   Max_Burst = 64;
	const int	   Max_Port = 65534;
	const int	   Min_Port = 1;
	const int	   Bites_Per_Byte = 8;
	const uint32_t Byte_Mask = 0xFFu;
	const int	   U16_Bytes = 2;
	const int	   U32_Bytes = 4;
	const int	   U64_Bytes = 8;
	const char* const Current_Folder = ".";
	const int    Chunk_Size = 240;

}

#endif // !TRANSFER_CONSTANTS_H

