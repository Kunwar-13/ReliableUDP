#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>
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

}

#endif // !OPTIONS_H
