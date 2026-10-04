#ifndef TRANSFER_CONSTANTS_H
#define TRANSFER_CONSTANTS_H

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <chrono>
#include "Net.h"

namespace ReliableUDP 
{
	const int      Default_Burst = 8;
	const int	   Max_Burst = 64;
	const int      Max_Name_Length = 200;
	const int      Data_Header = 8;
	const int      Chunk_Size = 240;

	const unsigned char Type_Info = 'F';
	const unsigned char Type_Accept = 'K';
	const unsigned char Type_Reject = 'E';
	const unsigned char Type_Data = 'D';
	const unsigned char Type_Nudge = 'A';
	const unsigned char Type_Result = 'R';
	const unsigned char Type_Fin = 'Z';

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
	const int	   Max_Port = 65534;
	const int	   Min_Drop_Packet = 2;
	const int	   Min_Port = 1;
	const int	   Bites_Per_Byte = 8;
	const double   Bits_Per_Megabit = 1000000.0;
	const double   Bytes_Per_Kilobyte = 1024.0;
	const uint32_t Byte_Mask = 0xFFu;
	const int	   U16_Bytes = 2;
	const int	   U32_Bytes = 4;
	const int	   U64_Bytes = 8;
	const int	   Ipv4_Part_Count = 4;
	const char* const Current_Folder = ".";

}

#endif // !TRANSFER_CONSTANTS_H

