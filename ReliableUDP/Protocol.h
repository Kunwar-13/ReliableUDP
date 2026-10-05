#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "TransferConstants.h"
#include "ByteCode.h"
#include "Chunking.h"

namespace ReliableUDP
{
struct MessageInfo {

	uint32_t NameLength;
	uint64_t FileSize;
	uint32_t TotalChunks;
	uint32_t FileCrc;
	uint32_t ChunckSize;

};

struct ResultMessage
{
	int status;
	uint32_t Crc;
	uint64_t BytesWritten;

};

int packInfo(unsigned char* packet, const std::string& fileName, uint64_t fileSize,
	uint32_t totalChunks, uint32_t fileCrc, uint32_t chunkSize);

bool unpackInfo(const unsigned char* packet, int length, MessageInfo& info);
bool isInfoConsistent(const MessageInfo& info, int packetLength);

int packAccept(unsigned char* packet, uint32_t totalChunks);
bool unpackAccept(const unsigned char* packet, int length, uint32_t& totalChunks);

int packReject(unsigned char* packet, int reason);
int unpackRejectReason(const unsigned char* packet, int length); 

int packDataHeader(unsigned char* packet, uint32_t chunkIndex, uint32_t payloadLength);
bool unpackDataHeader(const unsigned char* packet, int length, uint32_t& chunkIndex, uint32_t& payloadLength);

int packResult(unsigned char* packet, const ResultMessage& result);
bool unpackResult(const unsigned char* packet, int length, ResultMessage& result);

int packSimple(unsigned char* packet, unsigned char type);

}

#endif // !PROTOCOL_H

