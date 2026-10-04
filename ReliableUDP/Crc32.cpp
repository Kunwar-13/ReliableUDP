#include "Crc32.h"

namespace ReliableUDP {

	uint32_t Crc32::Update(uint32_t crc, const unsigned char* data, size_t length)
	{
		const uint32_t* table = Table();
		uint32_t workingCrc = crc ^ Crc_Mask;

		for (size_t index = 0; index < length; ++index)
		{
			workingCrc = table[(workingCrc ^ data[index]) & Byte_Mask] ^ (workingCrc >> Bites_Per_Byte);
		}

		return workingCrc ^ Crc_Mask;
	}

	bool Crc32::SelfTest()
	{
		const unsigned char checkBytes[] = { '1', '2', '3', '4', '5', '6', '7', '8', '9' };

		return Update(0, checkBytes, sizeof(checkBytes)) == Crc_Check_Value;
	}

	const uint32_t* Crc32::Table()
	{
		static uint32_t table[Crc_Table_Size];
		static bool built = false;

		if (!built)
		{
			for (uint32_t entry = 0; entry < Crc_Table_Size; ++entry)
			{
				uint32_t value = entry;

				for (int bit = 0; bit < Bites_Per_Byte; ++bit)
				{
					value = (value & 1u) ? (Crc_Polynomial ^ (value >> 1)) : (value >> 1);
				}
				table[entry] = value;
			}
			built = true;
		}

		return table;
	}

	bool fileCrc32(const std::string& path, uint32_t& crc, uint64_t& size)
	{
		bool readable = false;
		std::ifstream input(path.c_str(), std::ios::in | std::ios::binary);

		if (input)
		{
			std::vector<unsigned char> block(Crc_Block_Size);
			bool moreData = true;

			crc = 0;
			size = 0;
			while (moreData && input)
			{
				input.read(reinterpret_cast<char*>(&block[0]), static_cast<std::streamsize>(block.size()));
				const std::streamsize bytesRead = input.gcount();

				if (bytesRead <= 0)
				{
					moreData = false;
				}
				else
				{
					crc = Crc32::Update(crc, &block[0], static_cast<size_t>(bytesRead));
					size += static_cast<uint64_t>(bytesRead);
				}
			}
			readable = true;
		}

		return readable;
	}

}