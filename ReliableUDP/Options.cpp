#include "Options.h"

namespace ReliableUDP 
{

	void printUsage() 
	{

		printf("\nReliableUDP - reliable file transfer over UDP\n\n");
		printf("	Recieve a file (server, waits for a sender):\n");
		printf("		ReliableUDP.exe <port> [-o folder]\n\n");
		printf("	Send a file (client)\n");
		printf("		ReliableUDP.exe <server-ip> <port> -f <file> [-b n]	[-c] [-d n]\n\n");
		printf("	Options:\n");
		printf("	 -f <file>   file to send (omit the name to be prompted for it)\n");
		printf("	 -o <folder> folder to save received files in (default: current folder)\n");
		printf("	 -b <n>      data packets sent per send tick, 1-%d (default: %d)\n", Max_Burst, Default_Burst);
		printf("	 -c          TEST: corrupt one byte of one chunk so the whole-file CRC must fail\n");
		printf("	 -d <n>      TEST: pretend every n-th data packet is lost \n");
		printf("	 -h          show this help\n\n");
		printf("	The port is required (%d-%d). The client uses port+1 for its own socket.\n\n",Min_Port, Max_Port);

	}

}