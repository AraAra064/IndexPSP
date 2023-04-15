#include <vector>
#include <unordered_map>

#include "ConsoleGraphics1.hpp"

//Version 2, more efficient
#ifndef __SHTXPS_PARSER__
#define __SHTXPS_PARSER__

namespace IndexSHTXPS
{
	const uint32 HeaderOffset = 0x14; //20
	
	bool IsValid(uint8 *data)
	{
		std::string magic((char*)&data[0], 6);
		return (magic == "SHTXPS");
	}
	uint16 GetColourTableSize(std::vector<uint8> &data){return *(uint16*)&data[0x06];
	}
	uint16 GetWidth(std::vector<uint8> &data){return *(uint16*)&data[0x0A];
	}
	uint16 GetHeight(std::vector<uint8> &data){return *(uint16*)&data[0x0C];
	}
	
	//BGRA
	std::pair<uint32, uint8> ToCGPixel(uint32 p){return std::make_pair(_byteswap_ulong(p) >> 8, (p & 0xFF000000) >> 24);
	}
	uint32 ToSHTXPSPixel(std::pair<uint32, uint8> p){return (_byteswap_ulong(p.first << 8));
	}
	
	//Create CG Image from SHTXPS file
	Image LoadImage(std::vector<uint8> &data, bool *isLoaded = nullptr)
	{
		std::vector<std::pair<uint32, uint8>> colourTable;
		Image image;
		bool valid = (IsValid(&data[0]) && !data.empty());
		
		if (isLoaded != nullptr){*isLoaded = valid;
		}
		
		if (valid)
		{
			uint16 colours = GetColourTableSize(data);
			uint16 width = GetWidth(data);
			uint16 height = GetHeight(data);
			uint32 dataSize = width * height;
			
			colourTable.reserve(256);
			for (uint16 i = 0; i < colours; i++){colourTable.push_back(ToCGPixel(*(uint32*)&data[HeaderOffset + (i * 4)]));
			}
			
			image = Image(width, height);
			
			for (uint32 y = 0; y < height; y++)
			{
				for (uint32 x = 0; x < width; x++)
				{
					uint32 i = (y * width) + x;
					void *ptr = &data[HeaderOffset + (colours * 4)];
					uint16 index = (colours <= 256 ? ((uint8*)ptr)[i] : ((uint16*)ptr)[i]);
					*image.accessPixel(x, y) = colourTable[index];
				}
			}
		} else image = Image(256, 256, 0x00FF0000);
		
		return image;
	}
	
	//Create SHTXPS file from CG Image
	//Expects a 256 colour image
	//Needs a pointer to the old image data to copy header
	std::vector<uint8> CreateFile(Image &image, uint16 colourTableSize = 256, uint8 *oldHeader = nullptr)
	{
		std::unordered_map<uint32, uint16> colourTable;
		std::vector<uint8> data;
		
		if (oldHeader != nullptr && IsValid(oldHeader))
		{
			for (uint32 i = 0; i < HeaderOffset; i++){data.push_back(oldHeader[i]);
			}
		} else {
			//20
			uint8 basicHeader[] = {'S', 'H', 'T', 'X', 
								   'P', 'S', 0x00, 0x00,
								   0x00, 0x00, 0x00, 0x00,
								   0x00, 0x00, 0x00, 0x00,
								   0x00, 0x00, 0x00, 0x00};
			
			data = std::vector<uint8>(basicHeader, basicHeader + sizeof(basicHeader));
		}
		
		*(uint16*)&data[0x0A] = image.getWidth();
		*(uint16*)&data[0x0C] = image.getHeight();
		uint32 imgSize = image.getWidth() * image.getHeight();
		
		//Create colour table
		for (uint32 i = 0, j = 0; i < imgSize; i++)
		{
			uint32 c = ToSHTXPSPixel(*image[i]);
			if (colourTable.count(c) == 0)
			{
				colourTable[c] = j;
				j++;
			}
		}
		
		*(uint16*)&data[0x06] = (uint16)colourTable.size();
		
		for (auto c : colourTable)
		{
			for (uint32 i = 0; i < 4; i++){data.push_back(((uint8*)&c.first)[i]);
			}
		}
		
		for (uint32 i = 0; i < imgSize; i++)
		{
			uint32 c = ToSHTXPSPixel(*image[i]);
			uint16 index = colourTable[c];
			
			data.push_back(index & 0x00FF); //Lowest byte first because little endian
			if (colourTableSize > 256){data.push_back((index & 0xFF00) >> 8);
			}
		}
		
		return data;
	}
};

#endif
