#include "Chunking.h"

namespace ReliableUDP
{
	uint64_t calculateTotalChunks(uint64_t fileSize)
	{
		return (fileSize + Chunk_Size - 1) / Chunk_Size;
	}

	uint64_t calculateChunkStart(uint64_t chunkIndex)
	{
		return static_cast<uint64_t>(chunkIndex) * Chunk_Size;
	}

	uint64_t calculateChunkLength(uint64_t fileSize, uint64_t chunkIndex)
	{
		const uint64_t start = calculateChunkStart(chunkIndex);
		const uint64_t remaining = fileSize - start;

		return static_cast<uint64_t>(remaining < Chunk_Size ? remaining : Chunk_Size);
	}
}