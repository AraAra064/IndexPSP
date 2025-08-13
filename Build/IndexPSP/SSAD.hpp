#include <vector>
#include <string>

#include "ConsoleGraphics.hpp"

#ifndef __INDEX_SSAD__
#define __INDEX_SSAD__

namespace IndexSSAD
{
	//Full SSAD sub file including header
	std::vector<uint8> GetDataByTag(std::vector<uint8>& ssadData, std::string tag, uint32* size = nullptr)
	{
		std::vector<uint8> data;

		uint32 _size;
		uint32 index = 0x20;

		return data;
	}

	//From whole file, get one SSAD file
	std::vector<uint8> GetSubSSADFile(std::vector<uint8> &fileData, uint32 index)
	{
		std::vector<uint8> data;

		//Traverse to ssad sub file

		//Get data

		return data;
	}
};

#endif