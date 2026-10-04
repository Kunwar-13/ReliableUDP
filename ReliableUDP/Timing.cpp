#include "TransferConstants.h"

namespace ReliableUDP {

	double getNowSeconds()
	{
		static const std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();

		return std::chrono::duration<double>(std::chrono::steady_clock::now() - startTime).count();
	}


	double calculateMegabitsPerSecond(uint64_t bytes, double seconds)
	{
		return static_cast<double>(bytes) * Bites_Per_Byte / seconds / Bits_Per_Megabit;
	}

	void printTiming(uint64_t bytes, double seconds)
	{
		printf("transfer time   : %.3f seconds\n", seconds);
		if (seconds > 0.0)
		{
			printf("transfer speed  : %.4f Mbit/s (%.2f KB/s)\n", calculateMegabitsPerSecond(bytes, seconds),
				static_cast<double>(bytes) / seconds / Bytes_Per_Kilobyte);
		}
		else
		{
			printf("transfer speed  : n/a (transfer too short to time)\n");
		}
	}
}

}