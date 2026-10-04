#include "FileNames.h"

namespace ReliableUDP
{
	std::string baseName(const std::string& path)
	{
		std::string name = path;
		const size_t slash = name.find_last_of("/\\");

		if (slash != std::string::npos)
		{
			name = name.substr(slash + 1);
		}

		return name;
	}

	std::string makeSafeName(const unsigned char* nameBytes, int length)
	{
		std::string name = baseName(std::string(reinterpret_cast<const char*>(nameBytes),
			static_cast<size_t>(length)));

		for (size_t index = 0; index < name.size(); ++index)
		{
			const char character = name[index];

			if (static_cast<unsigned char>(character) < ' ' || character == '<' || character == '>' ||
				character == ':' || character == '"' || character == '|' || character == '?' ||
				character == '*')
			{
				name[index] = '_';
			}
		}

		if (name.empty() || name == "." || name == "..")
		{
			name = Default_File_Name;
		}

		return name;
	}
	std::string makeUniquePath(const std::string& name, const std::string& folder)
	{
		const std::string directory = (folder.empty() || folder == ".") ?
			std::string() : folder + "/";
		std::string stem = name;
		std::string extension;
		const size_t dot = name.find_last_of('.');
		std::string candidate = directory + name;
		bool unique = false;

		if (dot != std::string::npos && dot > 0)
		{
			stem = name.substr(0, dot);
			extension = name.substr(dot);
		}

		for (int attempt = 1; attempt < 100000 && !unique; ++attempt)
		{
			std::ifstream existing(candidate.c_str(), std::ios::in | std::ios::binary);

			if (!existing)
			{
				unique = true;
			}
			else
			{
				candidate = directory + stem + "_" + std::to_string(attempt) + extension;
			}
		}

		return candidate;
	}
	std::string placeFinishedFile(const std::string& partialPath, const std::string& finalPath, int status)
	{
		std::string shownPath = finalPath;

		if (status == Status_Ok)
		{
			if (std::rename(partialPath.c_str(), finalPath.c_str()) != 0)
			{
				shownPath = partialPath;
			}
		}
		else
		{
			shownPath = finalPath + Corrupt_Suffix;
			std::remove(shownPath.c_str());
			if (std::rename(partialPath.c_str(), shownPath.c_str()) != 0)
			{
				shownPath = partialPath;
			}
		}

		return shownPath;
	}
}
