#ifndef TIMING_H
#define TIMING_H

#include "TransferConstants.h"

namespace ReliableUDP
{
	double getNowSeconds();
	double calculateMegabitsPerSecond(uint64_t bytes, double seconds);
	void printTiming(uint64_t bytes, double seconds);
}

#endif // TIMING_H

