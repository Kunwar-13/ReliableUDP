#ifndef CHUNKING_H
#define CHUNKING_H

#include "TransferConstants.h"

namespace ReliableUDP
{
	
	uint64_t calculateTotalChunks(uint64_t fileSize);
	uint64_t calculateChunkStart(uint64_t chunkIndex);
	uint64_t calculateChunkLength(uint64_t fileSize, uint64_t chunkIndex);

}

#endif // !CHUNKING_H
