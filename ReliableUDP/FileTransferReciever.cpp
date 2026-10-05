#include "FileTransfer.h"

namespace ReliableUDP
{

	void FileTransfer::ReceiverTick(double now)
	{
		if (receiveActive && resultReady)
		{
			// keep telling the sender the verdict until it says goodbye
			if (now - resultSentTime >= Result_Resend_Interval)
			{
				SendResult(now);
			}

			if (now - resultFirstSentTime > Result_Give_Up_Time)
			{
				printf("[receive] sender did not confirm; finishing anyway\n");
				finished = true;
			}
		}
	}

	void FileTransfer::OnInfo(const unsigned char* packetData, int packetLength)
	{
		if (role != ROLE_SENDER)             // a sender ignores announcements
		{
			if (receiveActive)
			{
				SendAccept();                // our 'K' was lost, repeat it
			}
			else
			{
				AcceptNewFile(packetData, packetLength);
			}
		}
	}

	void FileTransfer::AcceptNewFile(const unsigned char* packetData, int packetLength)
	{
		MessageInfo info = { 0, 0, 0, 0, 0 };

		if (unpackInfo(packetData, packetLength, info))
		{
			if (!isInfoConsistent(info, packetLength))
			{
				SendReject(Reject_Bad_Info);
			}
			else if (info.FileSize > Max_File_Size)
			{
				SendReject(Reject_Too_Big);
			}
			else
			{
				CreateReceiveFile(info, packetData + Info_Name_Offset);
			}
		}
	}

	void FileTransfer::CreateReceiveFile(const MessageInfo& info, const unsigned char* nameBytes)
	{
		finalPath = makeUniquePath(makeSafeName(nameBytes, static_cast<int>(info.NameLength)), options.OutDir);
		partialPath = finalPath + Partial_Suffix;
		receiveStream.open(partialPath.c_str(),
			std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

		if (!receiveStream)
		{
			printf("[receive] cannot create '%s' (does the folder exist?)\n", partialPath.c_str());
			SendReject(Reject_Cannot_Create);
		}
		else
		{
			receiveFileSize = info.FileSize;
			receiveTotalChunks = info.TotalChunks;
			receiveExpectedCrc = info.FileCrc;
			chunkReceived.assign(info.TotalChunks, 0);
			receivedChunkCount = 0;
			receiveActive = true;

			printf("\n[receive] incoming file '%s': %llu bytes in %u chunks, sender CRC-32 %08X\n",
				finalPath.c_str(), static_cast<unsigned long long>(info.FileSize), info.TotalChunks,
				info.FileCrc);
			SendAccept();

			if (info.TotalChunks == 0)
			{
				FinishReceive();             // empty file: nothing to wait for
			}
		}
	}

	void FileTransfer::OnData(const unsigned char* packetData, int packetLength)
	{
		uint32_t chunkIndex = 0;
		uint32_t payloadLength = 0;

		if (receiveActive && !resultReady && unpackDataHeader(packetData, packetLength, chunkIndex, payloadLength))
		{
			if (chunkIndex < receiveTotalChunks && static_cast<int>(Data_Header + payloadLength) <= packetLength)
			{
				if (payloadLength == calculateChunkLength(chunkIndex, receiveFileSize))
				{
					StoreChunk(packetData, chunkIndex, payloadLength, calculateChunkStart(chunkIndex));
				}
			}
		}
	}

	void FileTransfer::OnFin()
	{
		if (receiveActive && resultReady)
		{
			finished = true;
		}
	}

	void FileTransfer::SendReject(int reason)
	{
		unsigned char packet[Reject_Length];
		const int length = packReject(packet, reason);

		connection.SendPacket(packet, length);
	}

	void FileTransfer::SendAccept()
	{
		unsigned char packet[Accept_Length];
		const int length = packAccept(packet, receiveTotalChunks);

		connection.SendPacket(packet, length);
	}

	void FileTransfer::SendResult(double now)
	{
		unsigned char packet[Result_Length];
		ResultMessage result;

		result.status = resultStatus;
		result.Crc = resultCrc;
		result.BytesWritten = resultBytes;
		const int length = packResult(packet, result);

		connection.SendPacket(packet, length);
		resultSentTime = now;
	}

	void FileTransfer::StoreChunk(const unsigned char* packetData, uint32_t chunkIndex,
		uint32_t payloadLength, uint64_t start)
	{
		const double now = getNowSeconds();
		bool writeFailed = false;

		if (chunkReceived[chunkIndex])
		{
			++duplicateCount;                // a resend of something we already have
		}
		else
		{
			writeFailed = !WriteChunk(packetData, chunkIndex, payloadLength, start, now);
		}

		if (!writeFailed)
		{
			NudgeSender();
			PrintReceiveProgress(now);

			if (receivedChunkCount == receiveTotalChunks)
			{
				FinishReceive();
			}
		}
	}

	bool FileTransfer::WriteChunk(const unsigned char* packetData, uint32_t chunkIndex,
		uint32_t payloadLength, uint64_t start, double now)
	{
		bool written = true;

		receiveStream.seekp(static_cast<std::streamoff>(start), std::ios::beg);
		receiveStream.write(reinterpret_cast<const char*>(packetData + Data_Header),
			static_cast<std::streamsize>(payloadLength));

		if (!receiveStream)
		{
			SendReject(Reject_Write_Error);
			Fail("could not write the received file (disk full?)", Exit_Transfer_Failed);
			written = false;
		}
		else
		{
			chunkReceived[chunkIndex] = 1;
			++receivedChunkCount;
			if (receiveFirstTime < 0.0)
			{
				receiveFirstTime = now;
			}
			receiveLastTime = now;
		}

		return written;
	}

	void FileTransfer::NudgeSender()
	{
		++packetsSinceNudge;
		if (packetsSinceNudge >= Ack_Nudge_Every)
		{
			unsigned char nudge[Simple_Packet_Length];
			const int length = packSimple(nudge, Type_Nudge);

			connection.SendPacket(nudge, length);
			packetsSinceNudge = 0;
		}
	}

	void FileTransfer::PrintReceiveProgress(double now)
	{
		if (now - receiveLastProgressTime >= Progress_Interval)
		{
			receiveLastProgressTime = now;
			printf("[receive] %.1f%% (%u/%u chunks), %u duplicates\n",
				100.0 * receivedChunkCount / receiveTotalChunks, receivedChunkCount,
				receiveTotalChunks, duplicateCount);
		}
	}

	void FileTransfer::FinishReceive()
	{
		uint32_t crc = 0;
		uint64_t bytes = 0;
		double verifySeconds = 0.0;

		receiveStream.flush();
		receiveStream.close();

		const int status = VerifyReceivedFile(crc, bytes, verifySeconds);
		const std::string shownPath = placeFinishedFile(partialPath, finalPath, status);

		PrintReceiverReport(status, crc, bytes, verifySeconds, shownPath);
		if (status != Status_Ok)
		{
			exitCode = Exit_Verify_Failed;
		}

		BeginSendingVerdict(status, crc, bytes);
	}

	int FileTransfer::VerifyReceivedFile(uint32_t& crc, uint64_t& bytes, double& verifySeconds)
	{
		const double verifyStart = getNowSeconds();
		const bool readable = fileCrc32(partialPath, crc, bytes);
		int status = Status_Ok;

		verifySeconds = getNowSeconds() - verifyStart;

		if (!readable || bytes != receiveFileSize)
		{
			status = Status_Size_Mismatch;
		}
		else if (crc != receiveExpectedCrc)
		{
			status = Status_Crc_Mismatch;
		}

		return status;
	}

	void FileTransfer::PrintReceiverReport(int status, uint32_t crc, uint64_t bytes, double verifySeconds,
		const std::string& shownPath) const
	{
		const double seconds = (receiveFirstTime >= 0.0) ? (receiveLastTime - receiveFirstTime) : 0.0;

		printf("\n================ TRANSFER RESULT (receiver) ==============\n");
		printf("saved as        : %s\n", shownPath.c_str());
		printf("size            : %llu bytes\n", static_cast<unsigned long long>(bytes));
		printf("duplicates      : %u chunks received more than once\n", duplicateCount);
		printf("expected CRC-32 : %08X (from sender)\n", receiveExpectedCrc);
		printf("computed CRC-32 : %08X (of the saved file, %.3f s to verify)\n", crc, verifySeconds);
		printTiming(receiveFileSize, seconds);
		PrintVerdict(status);
		printf("===========================================================\n\n");
	}

	void FileTransfer::BeginSendingVerdict(int status, uint32_t crc, uint64_t bytes)
	{
		resultStatus = status;
		resultCrc = crc;
		resultBytes = bytes;
		resultReady = true;
		resultFirstSentTime = getNowSeconds();
		SendResult(resultFirstSentTime);
	}

}