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

	bool parsePort(const char* text, int& port) {

		bool valid = false;
		char* end = NULL;
		const long number = std::strtol(text, &end, 10);

		if (end != text && *end == '\0' && number >= Min_Port && number <= Max_Port) {

			port = static_cast<int>(number);
			valid = true;

		}
		else {

			printf("Port must be a number between %d and %d\n", Min_Port, Max_Port);

		}

		return valid;

	}

	bool parseIPv4(const char* text, int& first, int& second, int& third, int& fourth) {

		bool valid = true;
		int parts[Ipv4_Part_Count] = { 0 };
		const char* position = text;

		for (int i = 0; i < Ipv4_Part_Count && valid; ++i) {

			if (*position < '0' || *position>'9') {

				valid = false;

			}
			else
			{
				char* end = NULL;
				const long value = std::strtol(position, &end, 10);

				if (value < 0 || value>255) {

					valid = false;

				}
				else {

					parts[i] = static_cast<int>(value);
					position = end;

					if (i < Ipv4_Part_Count-1)
					{
						if (*position != '.') {

							valid = false;

						}
						else
						{
							++position;
						}
					}

				}

			}

		}

		if (valid && *position != '\0') {

			valid = false;

		}

		if (valid) {

			first = parts[0];
			second = parts[1];
			third = parts[2];
			fourth = parts[3];

		}
		
		return valid;
	}

	bool applyValueOption(const std::string& name, const char* value, Options& options) {

		bool accepted = true;

		if (name == "-o") {

			options.OutDir = value;

		}
		else
		{

			char* end = NULL;
			const long number = std::strtol(value, &end, 10);

			if (end == value || *end != '\0') {

				printf("options %s needs a number, got %s\n", name.c_str(), value);
				accepted = false;

			}
			else if (name == "-b") {

				if (number<1 || number>Max_Burst) {

					printf("-b must be between 1 and %d\n", Max_Burst);

				}
				else
				{
					options.Burst = static_cast<int>(number);
				}

			}
			else
			{
				if (number < Min_Drop_Packet) {

					printf("-d must be %d or more\n", Min_Drop_Packet);
					accepted = false;

				}
				else
				{
					options.DropEvery = static_cast<int>(number);
				}
			}
		}
		
		return accepted;

	}

	bool parseOptions(int argc, char* argv[], Options& options) {

		bool usable = true;
		int portIndex = 1;
		int firstOctet = 0;
		int secondOctet = 0;
		int thirdOctet = 0;
		int fourthOctet = 0;

		if (argc > 1 && (std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help" ||
			std::string(argv[1]) == "-?"))
		{
			options.Help = true;
			printUsage();
			usable = false;
		}
		else
		{
			// a client gives the server IP first then the port and server gives only the port
			if (argc > 1 && parseIPv4(argv[1], firstOctet, secondOctet, thirdOctet, fourthOctet))
			{
				portIndex = 2;
			}

			if (argc <= portIndex)
			{
				printf("the server port is required\n");
				printUsage();
				usable = false;
			}
			else
			{
				usable = parsePort(argv[portIndex], options.Port);
			}
		}

		for (int argIndex = portIndex + 1; argIndex < argc && usable; ++argIndex)
		{
			const std::string argument = argv[argIndex];

			if (argument == "-h" || argument == "--help" || argument == "-?")
			{
				options.Help = true;
				printUsage();
				usable = false;
			}
			else if (argument == "-c")
			{
				options.Corrupt = true;
			}
			else if (argument == "-f")
			{
				if (argIndex + 1 < argc && argv[argIndex + 1][0] != '-')
				{
					++argIndex;
					options.SendPath = argv[argIndex];
				}
				else
				{
					options.PromptForFile = true;
				}
			}
			else if (argument == "-o" || argument == "-b" || argument == "-d")
			{
				if (argIndex + 1 >= argc)
				{
					printf("option %s needs a value\n", argument.c_str());
					printUsage();
					usable = false;
				}
				else
				{
					++argIndex;
					usable = applyValueOption(argument, argv[argIndex], options);
				}
			}
			else
			{
				printf("unknown option '%s'\n", argument.c_str());
				printUsage();
				usable = false;
			}
		}

		return usable;
	}

	

}