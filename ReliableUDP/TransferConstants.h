#ifndef TRANSFER_CONSTANTS_H
#define TRANSFER_CONSTANTS_H

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace ReliableUDP 
{
	const int      Default_Burst = 8;
	const int	   Max_Burst = 64;
	const int	   Max_port = 65534;
	const int	   Min_port = 1;
	const char* const Current_Folder = ".";

}

#endif // !TRANSFER_CONSTANTS_H

