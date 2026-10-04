#ifndef FILENAMES_H
#define FILENAMES_H

#include "TransferConstants.h"

namespace ReliableUDP
{
	std::string baseName(const std::string& path);
	std::string makeSafeName(const unsigned char* name, int length);
	std::string makeUniquePath(const std::string& name, const std::string& folder);
	std::string placeFinishedFile(const std::string& partialPath, const std::string& finalPath, int status);
}

#endif // !FILENAMES_H
