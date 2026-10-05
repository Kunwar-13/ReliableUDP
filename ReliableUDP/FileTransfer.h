#ifndef FILETRANSFER_H
#define FILETRANSFER_H

#include "Options.h"
#include "Protocol.h"
#include "TransferConstants.h"
#include "Chunking.h"
#include "Crc32.h"
#include "FileNames.h"
#include "Timing.h"


namespace ReliableUDP
{

	class FileTransfer
	{
	public:

		FileTransfer(net::ReliableConnection& reliableConnection, const Options& transferOptions);

		// ---- called from main() (FileTransfer.cpp, FileTransferSender.cpp) ----
		bool Prepare();
		void Update();
		void OnPacket(const unsigned char* packetData, int packetLength);
		void ProcessAcks();
		void OnSendTick();
		bool IsFinished() const;
		int ExitCode() const;

	private:

		enum Role
		{
			ROLE_IDLE,
			ROLE_SENDER
		};

		enum Phase
		{
			PHASE_SEND_INFO,
			PHASE_SEND_DATA,
			PHASE_FIN,
			PHASE_DONE
		};

		struct SentInfo
		{
			uint32_t Chunk;
			double   Time;
		};

		// ---------------- shared state ----------------

		net::ReliableConnection& connection;
		Options options;
		Role    role;
		Phase   phase;
		bool    finished;
		int     exitCode;
		bool    wasConnected;

		// ---------------- sender state ----------------

		std::ifstream sendStream;
		std::string   sendFileName;
		uint64_t      fileSize;
		uint32_t      totalChunks;
		uint32_t      fileCrc;
		std::vector<unsigned char> chunkState;            // kChunkUnsent / InFlight / Acked
		std::vector<double>        lastSentTime;
		std::deque<uint32_t>       flightQueue;           // chunks in the order they were sent
		std::map<unsigned int, SentInfo> sequenceToChunk; // reliability sequence -> chunk
		uint32_t nextNewChunk;
		uint32_t inFlightCount;
		uint32_t ackedChunkCount;
		double   infoStartTime;
		double   lastInfoTime;
		double   firstDataTime;
		double   endTime;
		double   lastProgressTime;
		unsigned transmitCount;
		unsigned dataPacketCount;
		unsigned retransmitCount;
		unsigned simulatedDropCount;
		uint32_t corruptChunkIndex;
		unsigned int lastAckedCount;
		int      finSentCount;
		double   finTime;

		// ---------------- receiver state ----------------

		std::fstream  receiveStream;
		std::string   partialPath;
		std::string   finalPath;
		bool          receiveActive;
		bool          resultReady;
		uint32_t      receiveTotalChunks;
		uint64_t      receiveFileSize;
		uint32_t      receiveExpectedCrc;                 // CRC the sender says the file has
		std::vector<unsigned char> chunkReceived;
		uint32_t      receivedChunkCount;
		int           packetsSinceNudge;
		unsigned      duplicateCount;
		double        receiveFirstTime;
		double        receiveLastTime;
		int           resultStatus;
		uint32_t      resultCrc;
		uint64_t      resultBytes;
		double        resultSentTime;
		double        resultFirstSentTime;
		double        receiveLastProgressTime;

		// ---- shared (FileTransfer.cpp) ----
		void Fail(const char* why, int code);
		static void PrintVerdict(int status);

		// ---- sender: setup (FileTransferSender.cpp) ----
		bool PromptForFileName();
		bool OpenFileToSend();
		bool OpenSendStream();
		void PrepareChunkTables();
		void PrintSendBanner() const;

		// ---- sender: protocol steps ----
		void SenderTick(double now);
		void SendInfoPacket(double now);
		void OnAccept(const unsigned char* packetData, int packetLength);
		void OnReject(const unsigned char* packetData, int packetLength);
		void OnResult(const unsigned char* packetData, int packetLength);
		void SendFin(double now);

		// ---- sender: moving chunks ----
		void SendDataBurst(double now);
		void ResendTimedOutChunks(double now, int& budget);
		void SendNewChunks(double now, int& budget);
		void SendChunk(uint32_t chunkIndex, bool isRetransmit, double now);
		void TransmitChunk(uint32_t chunkIndex, double now);
		bool ReadChunkPayload(uint32_t chunkIndex, uint32_t payloadLength, unsigned char* destination);
		void MarkChunkAcked(uint32_t chunkIndex);
		void ForgetOldSequences(double now);

		// ---- sender: reporting ----
		void PrintSendProgress(double now);
		void PrintSenderReport(const ResultMessage& result, double seconds) const;

		// ---- receiver: protocol steps (FileTransferReceiver.cpp) ----
		void ReceiverTick(double now);
		void OnInfo(const unsigned char* packetData, int packetLength);
		void AcceptNewFile(const unsigned char* packetData, int packetLength);
		void CreateReceiveFile(const MessageInfo& info, const unsigned char* nameBytes);
		void OnData(const unsigned char* packetData, int packetLength);
		void OnFin();
		void SendReject(int reason);
		void SendAccept();
		void SendResult(double now);

		// ---- receiver: storing chunks ----
		void StoreChunk(const unsigned char* packetData, uint32_t chunkIndex, uint32_t payloadLength, uint64_t start);
		bool WriteChunk(const unsigned char* packetData, uint32_t chunkIndex, uint32_t payloadLength,
			uint64_t start, double now);
		void NudgeSender();
		void PrintReceiveProgress(double now);

		// ---- receiver: finishing ----
		void FinishReceive();
		int VerifyReceivedFile(uint32_t& crc, uint64_t& bytes, double& verifySeconds);
		void PrintReceiverReport(int status, uint32_t crc, uint64_t bytes, double verifySeconds,
			const std::string& shownPath) const;
		void BeginSendingVerdict(int status, uint32_t crc, uint64_t bytes);
	};

}

#endif // !FILETRANSFER_H