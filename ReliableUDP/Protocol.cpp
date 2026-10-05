#include "Protocol.h"

namespace ReliableUDP
{

	int packInfo(unsigned char* packet, const std::string& fileName, uint64_t fileSize,
		uint32_t totalChunks, uint32_t fileCrc, uint32_t chunkSize) 
	{


		memset(packet, 0, Max_Info_Packet_Length);
		packet[Type_Offset] = Type_Info;
		putU16(packet + Info_Name_Length_Offset, static_cast<uint32_t>(fileName.size()));
		putU64(packet + Info_File_Size_Offset, fileSize);
		putU32(packet + Info_Total_Chunks_Offset, totalChunks);
		putU32(packet + Info_Crc_Offset, fileCrc);
		putU16(packet + Info_Chunk_Size_Offset, chunkSize);
		memcpy(packet + Info_Name_Offset, fileName.data(), fileName.size());

		return Info_Name_Offset + static_cast<int>(fileName.size());

	}

	bool unpackInfo(const unsigned char* packet, int length, MessageInfo& info)
	{
		const bool longEnough = length >= Info_Name_Offset;

		if (longEnough)
		{
			info.NameLength = getU16(packet + Info_Name_Length_Offset);
			info.FileSize = getU64(packet + Info_File_Size_Offset);
			info.TotalChunks = getU32(packet + Info_Total_Chunks_Offset);
			info.FileCrc = getU32(packet + Info_Crc_Offset);
			info.ChunkSize = getU16(packet + Info_Chunk_Size_Offset);
		}

		return longEnough;
	}


	bool isInfoConsistent(const MessageInfo& info, int packetLength)
	{
		const int nameLength = static_cast<int>(info.NameLength);

		return nameLength > 0 && nameLength <= Max_Name_Length &&
			Info_Name_Offset + nameLength <= packetLength &&
			static_cast<int>(info.ChunkSize) == Chunk_Size &&
			static_cast<uint64_t>(info.TotalChunks) == calculateTotalChunks(info.FileSize);
	}


	int packAccept(unsigned char* packet, uint32_t totalChunks)
	{
		memset(packet, 0, Accept_Length);
		packet[Type_Offset] = Type_Accept;
		putU32(packet + Accept_Total_Chunks_Offset, totalChunks);

		return Accept_Length;
	}

	bool unpackAccept(const unsigned char* packet, int length, uint32_t& totalChunks)
	{
		const bool longEnough = length >= Accept_Length;

		if (longEnough)
		{
			totalChunks = getU32(packet + Accept_Total_Chunks_Offset);
		}

		return longEnough;
	}

	int packReject(unsigned char* packet, int reason)
	{
		packet[Type_Offset] = Type_Reject;
		packet[Zero_Offset] = 0;
		packet[Reject_Reason_Offset] = static_cast<unsigned char>(reason);

		return Reject_Length;
	}

	int unpackRejectReason(const unsigned char* packet, int length)
	{
		return (length >= Reject_Length) ? packet[Reject_Reason_Offset] : 0;
	}

	int packDataHeader(unsigned char* packet, uint32_t chunkIndex, uint32_t payloadLength)
	{
		packet[Type_Offset] = Type_Data;
		packet[Zero_Offset] = 0;
		putU32(packet + Data_Chunk_Index_Offset, chunkIndex);
		putU16(packet + Data_Length_Offset, payloadLength);

		return Data_Header;
	}

	bool unpackDataHeader(const unsigned char* packet, int length, uint32_t& chunkIndex, uint32_t& payloadLength)
	{
		const bool longEnough = length >= Data_Header;

		if (longEnough)
		{
			chunkIndex = getU32(packet + Data_Chunk_Index_Offset);
			payloadLength = getU16(packet + Data_Length_Offset);
		}

		return longEnough;
	}

	int packResult(unsigned char* packet, const ResultMessage& result)
	{
		memset(packet, 0, Result_Length);
		packet[Type_Offset] = Type_Result;
		packet[Result_Status_Offset] = static_cast<unsigned char>(result.status);
		putU32(packet + Result_Crc_Offset, result.Crc);
		putU64(packet + Result_Bytes_Offset, result.BytesWritten);

		return Result_Length;
	}

	bool unpackResult(const unsigned char* packet, int length, ResultMessage& result)
	{
		const bool longEnough = length >= Result_Length;

		if (longEnough)
		{
			result.status = packet[Result_Status_Offset];
			result.Crc = getU32(packet + Result_Crc_Offset);
			result.BytesWritten = getU64(packet + Result_Bytes_Offset);
		}

		return longEnough;
	}

	int packSimple(unsigned char* packet, unsigned char type)
	{
		packet[Type_Offset] = type;
		packet[Zero_Offset] = 0;

		return Simple_Packet_Length;
	}

}