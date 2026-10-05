#include "FileTransfer.h"

namespace ReliableUDP
{
	FileTransfer::FileTransfer(net::ReliableConnection& reliableConnection, const Options& transferOptions)
		: connection(reliableConnection), options(transferOptions), role(ROLE_IDLE),
		phase(PHASE_SEND_INFO), finished(false), exitCode(0), wasConnected(false),
		fileSize(0), totalChunks(0), fileCrc(0), nextNewChunk(0), inFlightCount(0),
		ackedChunkCount(0), infoStartTime(-1.0), lastInfoTime(-1.0), firstDataTime(-1.0),
		endTime(-1.0), lastProgressTime(0.0), transmitCount(0), dataPacketCount(0),
		retransmitCount(0), simulatedDropCount(0), corruptChunkIndex(0), lastAckedCount(0),
		finSentCount(0), finTime(-1.0), receiveActive(false), resultReady(false),
		receiveTotalChunks(0), receiveFileSize(0), receiveExpectedCrc(0),
		receivedChunkCount(0), packetsSinceNudge(0), duplicateCount(0),
		receiveFirstTime(-1.0), receiveLastTime(-1.0), resultStatus(Status_Ok),
		resultCrc(0), resultBytes(0), resultSentTime(-1.0), resultFirstSentTime(-1.0),
		receiveLastProgressTime(0.0)
	{
	}

	bool FileTransfer::Prepare()
	{
		bool ready = true;

		if (!Crc32::SelfTest())
		{
			printf("CRC-32 self test FAILED\n");
			ready = false;
		}

		if (ready && options.PromptForFile && options.SendPath.empty())
		{
			ready = PromptForFileName();
		}

		if (ready && !options.SendPath.empty())
		{
			ready = OpenFileToSend();
		}

		return ready;
	}

	void FileTransfer::Update()
	{
		if (!finished)
		{
			const bool connected = connection.IsConnected();

			if (connected)
			{
				wasConnected = true;
			}

			// the provided code drops the connection after 10s of silence
			if (wasConnected && !connected && (role == ROLE_SENDER || receiveActive))
			{
				if (role == ROLE_SENDER && (phase == PHASE_DONE || phase == PHASE_FIN))
				{
					finished = true;
				}
				else if (receiveActive && resultReady)
				{
					finished = true;             // result was produced; peer just left
				}
				else
				{
					Fail("connection lost during the transfer", Exit_Transfer_Failed);
				}
			}
		}
	}

	void FileTransfer::OnPacket(const unsigned char* packetData, int packetLength)
	{
		if (packetLength >= Simple_Packet_Length && packetData[Zero_Offset] == 0)
		{
			switch (packetData[Type_Offset])
			{
			case Type_Info:   OnInfo(packetData, packetLength);   break;
			case Type_Data:   OnData(packetData, packetLength);   break;
			case Type_Fin:    OnFin();                            break;
			case Type_Accept: OnAccept(packetData, packetLength); break;
			case Type_Reject: OnReject(packetData, packetLength); break;
			case Type_Result: OnResult(packetData, packetLength); break;
			default:                                              break;   // keep-alive etc.
			}
		}
	}

	void FileTransfer::OnSendTick()
	{
		if (!finished && connection.IsConnected())
		{
			const double now = getNowSeconds();

			if (role == ROLE_SENDER)
			{
				SenderTick(now);
			}
			else
			{
				ReceiverTick(now);
			}
		}
	}

	bool FileTransfer::IsFinished() const
	{
		return finished;
	}

	int FileTransfer::ExitCode() const
	{
		return exitCode;
	}

	void FileTransfer::Fail(const char* why, int code)
	{
		printf("*** TRANSFER FAILED: %s ***\n", why);
		exitCode = code;
		finished = true;
	}

	void FileTransfer::PrintVerdict(int status)
	{
		if (status == Status_Ok)
		{
			printf("verdict         : SUCCESS - whole file verified\n");
		}
		else
		{
			printf("verdict         : FAILED - %s\n",
				status == Status_Crc_Mismatch ? "CRC-32 mismatch, file is corrupt" : "size mismatch");
		}
	}

}