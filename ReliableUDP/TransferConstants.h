#ifndef TRANSFER_CONSTANTS_H
#define TRANSFER_CONSTANTS_H

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <chrono>
#include <fstream>
#include <iostream>
#include <deque>
#include "Net.h"

namespace ReliableUDP 
{
	const int      Default_Burst = 8;
	const int	   Max_Burst = 64;
	const int      Max_Name_Length = 200;
	const int      Data_Header = 8;
	const int      Chunk_Size = 240;
	const uint64_t Max_File_Size = 2147483648ULL;
	const double   Info_Resend_Interval = 0.5;
	const double   Response_Timeout = 30.0;
	const int      Ack_Nudge_Every = 8;           // receiver sends an 'A' every N data packets
	const int      Fin_Count = 3;
	const unsigned char Corrupt_Bit_Mask = 0x01;
	const uint32_t Window_Chunks = 256;
	const double   Retransmit_Timeout = 0.5;


	const unsigned char Type_Info = 'F';
	const unsigned char Type_Accept = 'K';
	const unsigned char Type_Reject = 'E';
	const unsigned char Type_Data = 'D';
	const unsigned char Type_Nudge = 'A';
	const unsigned char Type_Result = 'R';
	const unsigned char Type_Fin = 'Z';

	const int Status_Ok = 0;
	const int Status_Crc_Mismatch = 1;
	const int Status_Size_Mismatch = 2;

	const double Result_Resend_Interval = 0.25;   
	const double Result_Give_Up_Time = 5.0;        
	const double Progress_Interval = 1.0;        
	const double Sequence_Forget_Time = 3.0;      
	const double Fin_Settle_Time = 0.3;

	const int Reject_Cannot_Create = 1;
	const int Reject_Bad_Info = 2;
	const int Reject_Too_Big = 3;
	const int Reject_Write_Error = 4;

	const int Exit_Transfer_Failed = 2;
	const int Exit_Verify_Failed = 3;

	const int Type_Offset = 0;
	const int Zero_Offset = 1;
	const int Simple_Packet_Length = 2;           
	const int Info_Name_Length_Offset = 2;
	const int Info_File_Size_Offset = 4;
	const int Info_Total_Chunks_Offset = 12;
	const int Info_Crc_Offset = 16;
	const int Info_Chunk_Size_Offset = 20;
	const int Info_Name_Offset = 22;              
	const int Max_Info_Packet_Length = Info_Name_Offset + Max_Name_Length;
	const int Accept_Total_Chunks_Offset = 2;
	const int Accept_Length = 6;
	const int Reject_Reason_Offset = 2;
	const int Reject_Length = 3;
	const int Data_Chunk_Index_Offset = 2;
	const int Data_Length_Offset = 6;
	const int Max_Data_Packet_Length = Data_Header + Chunk_Size;
	const int Result_Status_Offset = 2;
	const int Result_Crc_Offset = 4;
	const int Result_Bytes_Offset = 8;
	const int Result_Length = 16;

	const unsigned char Chunk_Unsent = 0;
	const unsigned char Chunk_In_Flight = 1;
	const unsigned char Chunk_Acked = 2;

	const int  Quote_Characters_Length = 2;
	const int	   Max_Port = 65534;
	const int	   Min_Drop_Packet = 2;
	const int	   Min_Port = 1;
	const char* const Partial_Suffix = ".part";
	const char* const Corrupt_Suffix = ".corrupt";
	const int	   Bites_Per_Byte = 8;
	const double   Bits_Per_Megabit = 1000000.0;
	const double   Bytes_Per_Kilobyte = 1024.0;
	const uint32_t Byte_Mask = 0xFFu;
	const int	   U16_Bytes = 2;
	const int	   U32_Bytes = 4;
	const int	   U64_Bytes = 8;
	const int	   Ipv4_Part_Count = 4;
	const char* const Current_Folder = ".";
	const char* const Default_File_Name = "received_file";

	// CRC-32 constants
	const uint32_t Crc_Polynomial = 0xEDB88320u; // reflected IEEE 802.3 polynomial
	const uint32_t Crc_Mask = 0xFFFFFFFFu;       // initial and final XOR value
	const uint32_t Crc_Table_Size = 256;
	const uint32_t Crc_Check_Value = 0xCBF43926u; // CRC-32 of the text "123456789"
	const size_t   Crc_Block_Size = 64 * 1024;    // bytes read per block when hashing a file

}

#endif // !TRANSFER_CONSTANTS_H

