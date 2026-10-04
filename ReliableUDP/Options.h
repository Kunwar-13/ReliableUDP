#ifndef OPTIONS_H
#define OPTIONS_H

#include "TransferConstants.h"

namespace ReliableUDP 
{

	struct Options {

		std::string SendPath;
		bool		PromptForFile;
		std::string OutDir;
		int			Port;
		int			Burst;
		bool		Corrupt;
		int			DropEvery;
		bool		Help;

		Options() : PromptForFile(false), OutDir(Current_Folder), Port(0), Burst(Default_Burst),
			Corrupt(false), DropEvery(0), Help(false)
		{
		}

	};

	void printUsage();
	bool parsePort(const char* text, int& port);
	bool parseIPv4(const char* text, int& first, int& second, int& third, int& fourth);
	bool applyValueOption(const std::string& name, const char* value, Options& options);
	bool parseOptions(int argc, char* argv[], Options& options);

}

#endif // !OPTIONS_H
