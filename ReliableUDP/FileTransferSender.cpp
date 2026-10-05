#include "FileTransfer.h"

namespace ReliableUDP {

	bool FileTransfer::PromptForFileName()
	{
		bool chosen = false;
		std::string line;

		printf("Enter the path of the file to send: ");
		if (!std::getline(std::cin, line) || line.empty())
		{
			printf("no file chosen\n");
		}
		else
		{
			// strip surrounding quotes (drag and drop / copy as path)
			if (line.size() >= static_cast<size_t>(Quote_Characters_Length) &&
				line[0] == '"' && line[line.size() - 1] == '"')
			{
				line = line.substr(1, line.size() - Quote_Characters_Length);
			}
			options.SendPath = line;
			chosen = true;
		}

		return chosen;
	}

	bool FileTransfer::OpenFileToSend()
	{
		bool opened = false;

		if (!fileCrc32(options.SendPath, fileCrc, fileSize))
		{
			printf("cannot open '%s' for reading\n", options.SendPath.c_str());
		}
		else if (fileSize > Max_File_Size)
		{
			printf("'%s' is larger than the %llu byte limit\n",
				options.SendPath.c_str(), static_cast<unsigned long long>(Max_File_Size));
		}
		else if (OpenSendStream())
		{
			PrepareChunkTables();
			PrintSendBanner();
			opened = true;
		}

		return opened;
	}

	bool FileTransfer::OpenSendStream()
	{
		sendStream.open(options.SendPath.c_str(), std::ios::in | std::ios::binary);

		const bool isOpen = static_cast<bool>(sendStream);

		if (!isOpen)
		{
			printf("cannot open '%s' for reading\n", options.SendPath.c_str());
		}

		return isOpen;
	}

	void FileTransfer::PrepareChunkTables()
	{
		// base name (no directories) is what the receiver sees
		sendFileName = baseName(options.SendPath);
		if (sendFileName.size() > static_cast<size_t>(Max_Name_Length))
		{
			sendFileName = sendFileName.substr(sendFileName.size() - Max_Name_Length);
		}

		totalChunks = static_cast<uint32_t>(calculateTotalChunks(fileSize));
		chunkState.assign(totalChunks, Chunk_Unsent);
		lastSentTime.assign(totalChunks, 0.0);
		corruptChunkIndex = totalChunks / 2;
		role = ROLE_SENDER;
	}

	void FileTransfer::PrintSendBanner() const
	{
		printf("file to send: %s (%llu bytes, %u chunks of up to %d bytes)\n",
			sendFileName.c_str(), static_cast<unsigned long long>(fileSize), totalChunks, Chunk_Size);
		printf("whole-file CRC-32: %08X\n", fileCrc);
		if (options.Corrupt)
		{
			printf("TEST MODE: chunk %u will be deliberately corrupted - "
				"the CRC check should FAIL\n", corruptChunkIndex);
		}
		if (options.DropEvery > 0)
		{
			printf("TEST MODE: every %d-th data packet will be dropped on purpose\n",
				options.DropEvery);
		}
	}

	void FileTransfer::SenderTick(double now)
	{
		switch (phase)
		{
		case PHASE_SEND_INFO: SendInfoPacket(now); break;
		case PHASE_SEND_DATA: SendDataBurst(now);  break;
		case PHASE_FIN:       SendFin(now);        break;
		default:                                   break;
		}
	}

	void FileTransfer::SendInfoPacket(double now)
	{
		if (infoStartTime < 0.0)
		{
			infoStartTime = now;
		}

		if (now - infoStartTime > Response_Timeout)
		{
			Fail("receiver never accepted the file", Exit_Transfer_Failed);
		}
		else if (lastInfoTime < 0.0 || now - lastInfoTime >= Info_Resend_Interval)
		{
			unsigned char packet[Max_Info_Packet_Length];
			const int length = packInfo(packet, sendFileName, fileSize, totalChunks, fileCrc,
				static_cast<uint32_t>(Chunk_Size));

			connection.SendPacket(packet, length);
			lastInfoTime = now;
			printf("[send] file info sent, waiting for receiver to accept\n");
		}
	}

	void FileTransfer::OnAccept(const unsigned char* packetData, int packetLength)
	{
		uint32_t acceptedChunks = 0;

		if (role == ROLE_SENDER && phase == PHASE_SEND_INFO &&
			unpackAccept(packetData, packetLength, acceptedChunks) && acceptedChunks == totalChunks)
		{
			phase = PHASE_SEND_DATA;
			printf("[send] receiver accepted the file, sending data\n");
		}
	}

	void FileTransfer::OnReject(const unsigned char* packetData, int packetLength)
	{
		if (role == ROLE_SENDER && phase != PHASE_DONE)
		{
			const int reason = unpackRejectReason(packetData, packetLength);

			printf("[send] receiver rejected the file (reason %d)\n", reason);
			Fail("receiver rejected the file", Exit_Transfer_Failed);
		}
	}

	void FileTransfer::OnResult(const unsigned char* packetData, int packetLength)
	{
		ResultMessage result = { 0, 0, 0 };

		if (role == ROLE_SENDER && phase == PHASE_SEND_DATA && unpackResult(packetData, packetLength, result))
		{
			endTime = getNowSeconds();
			const double seconds = (firstDataTime >= 0.0) ? (endTime - firstDataTime) : 0.0;

			PrintSenderReport(result, seconds);
			if (result.status != Status_Ok)
			{
				exitCode = Exit_Verify_Failed;
			}

			phase = PHASE_FIN;
			finSentCount = 0;
		}
	}

	void FileTransfer::SendFin(double now)
	{
		if (finSentCount < Fin_Count)
		{
			unsigned char packet[Simple_Packet_Length];
			const int length = packSimple(packet, Type_Fin);

			connection.SendPacket(packet, length);
			++finSentCount;
			finTime = now;
		}
		else if (now - finTime > Fin_Settle_Time)
		{
			phase = PHASE_DONE;
			finished = true;
		}
	}

	void FileTransfer::SendDataBurst(double now)
	{
		int budget = options.Burst;

		ResendTimedOutChunks(now, budget);
		SendNewChunks(now, budget);
		ForgetOldSequences(now);
		PrintSendProgress(now);
	}

	void FileTransfer::ResendTimedOutChunks(double now, int& budget)
	{
		bool scanning = true;

		while (budget > 0 && scanning && !flightQueue.empty())
		{
			const uint32_t chunkIndex = flightQueue.front();

			if (chunkState[chunkIndex] == Chunk_Acked)
			{
				flightQueue.pop_front();
			}
			else if (now - lastSentTime[chunkIndex] < Retransmit_Timeout)
			{
				scanning = false;            // the oldest chunk is still within its timeout
			}
			else
			{
				flightQueue.pop_front();
				SendChunk(chunkIndex, true, now);
				flightQueue.push_back(chunkIndex);
				--budget;
			}
		}
	}

	void FileTransfer::SendNewChunks(double now, int& budget)
	{
		while (budget > 0 && inFlightCount < Window_Chunks && nextNewChunk < totalChunks && !finished)
		{
			const uint32_t chunkIndex = nextNewChunk;

			++nextNewChunk;
			SendChunk(chunkIndex, false, now);
			flightQueue.push_back(chunkIndex);
			--budget;
		}
	}

	void FileTransfer::SendChunk(uint32_t chunkIndex, bool isRetransmit, double now)
	{
		lastSentTime[chunkIndex] = now;
		if (chunkState[chunkIndex] == Chunk_Unsent)
		{
			chunkState[chunkIndex] = Chunk_In_Flight;
			++inFlightCount;
		}

		if (firstDataTime < 0.0)
		{
			firstDataTime = now;             // transfer clock starts at the first data packet
		}

		if (isRetransmit)
		{
			++retransmitCount;
		}

		// test hook: pretend the network ate this packet
		++transmitCount;
		if (options.DropEvery > 0 && (transmitCount % static_cast<unsigned>(options.DropEvery)) == 0)
		{
			++simulatedDropCount;
		}
		else
		{
			TransmitChunk(chunkIndex, now);
		}
	}

	void FileTransfer::TransmitChunk(uint32_t chunkIndex, double now)
	{
		const uint32_t payloadLength = calculateChunkLength(chunkIndex, fileSize);
		unsigned char packet[Max_Data_Packet_Length];

		packDataHeader(packet, chunkIndex, payloadLength);

		if (!ReadChunkPayload(chunkIndex, payloadLength, packet + Data_Header))
		{
			Fail("could not read the file being sent", Exit_Transfer_Failed);
		}
		else
		{
			// test hook: corrupt the data AFTER the CRC was calculated on the good file
			if (options.Corrupt && chunkIndex == corruptChunkIndex && payloadLength > 0)
			{
				packet[Data_Header] ^= Corrupt_Bit_Mask;
			}

			// the sequence number the reliability layer is about to give this packet
			const unsigned int sequence = connection.GetReliabilitySystem().GetLocalSequence();

			if (connection.SendPacket(packet, Data_Header + static_cast<int>(payloadLength)))
			{
				SentInfo info;

				info.Chunk = chunkIndex;
				info.Time = now;
				sequenceToChunk[sequence] = info;
				++dataPacketCount;
			}
		}
	}

	bool FileTransfer::ReadChunkPayload(uint32_t chunkIndex, uint32_t payloadLength, unsigned char* destination)
	{
		sendStream.clear();
		sendStream.seekg(static_cast<std::streamoff>(calculateChunkStart(chunkIndex)), std::ios::beg);
		sendStream.read(reinterpret_cast<char*>(destination), static_cast<std::streamsize>(payloadLength));

		return static_cast<uint32_t>(sendStream.gcount()) == payloadLength;
	}

	void FileTransfer::ProcessAcks()
	{
		if (role == ROLE_SENDER && phase == PHASE_SEND_DATA)
		{
			net::ReliabilitySystem& reliability = connection.GetReliabilitySystem();

			// GetAcks() indexes acks[0], which is invalid on an empty vector in a
			// Debug build, so only call it when the ack counter says there is news.
			const unsigned int nowAcked = reliability.GetAckedPackets();

			if (nowAcked != lastAckedCount)
			{
				unsigned int* acks = NULL;
				int ackCount = 0;

				lastAckedCount = nowAcked;
				reliability.GetAcks(&acks, ackCount);

				for (int ackIndex = 0; ackIndex < ackCount; ++ackIndex)
				{
					std::map<unsigned int, SentInfo>::iterator found = sequenceToChunk.find(acks[ackIndex]);

					if (found != sequenceToChunk.end())    // not found = ack for a control packet
					{
						MarkChunkAcked(found->second.Chunk);
						sequenceToChunk.erase(found);
					}
				}
			}
		}
	}

	void FileTransfer::MarkChunkAcked(uint32_t chunkIndex)
	{
		if (chunkIndex < totalChunks && chunkState[chunkIndex] != Chunk_Acked)
		{
			if (chunkState[chunkIndex] == Chunk_In_Flight && inFlightCount > 0)
			{
				--inFlightCount;
			}
			chunkState[chunkIndex] = Chunk_Acked;
			++ackedChunkCount;
		}
	}

	void FileTransfer::ForgetOldSequences(double now)
	{
		std::map<unsigned int, SentInfo>::iterator entry = sequenceToChunk.begin();

		while (entry != sequenceToChunk.end())
		{
			if (now - entry->second.Time > Sequence_Forget_Time)
			{
				sequenceToChunk.erase(entry++);
			}
			else
			{
				++entry;
			}
		}
	}

	void FileTransfer::PrintSendProgress(double now)
	{
		if (now - lastProgressTime >= Progress_Interval && totalChunks > 0)
		{
			lastProgressTime = now;
			printf("[send] %.1f%% acked (%u/%u chunks), %u resent, %u simulated drops\n",
				100.0 * ackedChunkCount / totalChunks, ackedChunkCount, totalChunks,
				retransmitCount, simulatedDropCount);
		}
	}

	void FileTransfer::PrintSenderReport(const ResultMessage& result, double seconds) const
	{
		printf("\n================ TRANSFER RESULT (sender) ================\n");
		printf("file            : %s\n", sendFileName.c_str());
		printf("size            : %llu bytes\n", static_cast<unsigned long long>(fileSize));
		printf("data packets    : %u sent, %u resent, %u simulated drops\n",
			dataPacketCount, retransmitCount, simulatedDropCount);
		printf("sender CRC-32   : %08X\n", fileCrc);
		printf("receiver CRC-32 : %08X (%llu bytes written)\n",
			result.Crc, static_cast<unsigned long long>(result.BytesWritten));
		printTiming(fileSize, seconds);
		PrintVerdict(result.status);
		printf("===========================================================\n\n");
	}

}