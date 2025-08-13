#include <vector>

#include "ConsoleGraphics.hpp"

#ifndef __SHTXPS_PARSER__
#define __SHTXPS_PARSER__

namespace IndexSHTXPS
{
	const uint32 HeaderOffset = 0x14; //20
	
	//BGRA
	std::pair<uint32, uint8> ToCGPixel(uint32 p){return std::make_pair(_byteswap_ulong(p) >> 8, (p & 0xFF000000) >> 24);
	}
	uint32 ToSHTXPSPixel(std::pair<uint32, uint8> p){return (_byteswap_ulong(p.first << 8)) | (p.second << 24);
	}
	
	//Create CG Image from SHTXPS file
	cg::Image LoadImage(std::vector<uint8> &data, bool *isLoaded = nullptr)
	{
		std::vector<std::pair<uint32, uint8>> colourTable;
		cg::Image image;
		bool valid = false;
		
		if (!data.empty())
		{
			std::string magic((char*)&data[0], 6);
			valid = (magic == "SHTXPS");
		}
		if (isLoaded != nullptr){*isLoaded = valid;
		}
		
		if (valid)
		{
			uint8 *header = &data[0];
			uint16 colours = *(uint16*)&header[0x06];
			uint16 width = *(uint16*)&header[0x0A];
			uint16 height = *(uint16*)&header[0x0C];
			uint32 dataSize = width * height;
			
			colourTable.reserve(256);
			for (uint16 i = 0; i < colours; i++){colourTable.push_back(ToCGPixel(*(uint32*)&data[HeaderOffset + (i * 4)]));
			}
			
			image = cg::Image(width, height);
			
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
		} else image = cg::Image(256, 256, 0x00FF0000);
		
		return image;
	}
	
	//Create SHTXPS file from CG Image
	//Expects a 256 colour image
	//Needs a pointer to the old image data to copy header
	std::vector<uint8> CreateFile(cg::Image &image, uint16 colourTableSize = 256, uint8 *oldHeader = nullptr)
	{
		std::vector<uint32> colourTable;
		std::vector<uint8> data(IndexSHTXPS::HeaderOffset, 0x00);

		uint8 header[] = {'S', 'H', 'T', 'X', 
						  'P', 'S', 0x00, 0x00,
						  0x00, 0x00, 0x00, 0x00,
						  0x00, 0x00, 0x00, 0x00,
						  0x00, 0x00, 0x00, 0x00};

		//Add renderMode to parameters?
		uint16 renderMode = (oldHeader == nullptr ? 0x0001 : *(uint16*)&oldHeader[0x08]);
		uint32 imgSize = image.getWidth() * image.getHeight();
		*(uint16*)&header[0x08] = renderMode;
		*(uint32*)&header[0x10] = imgSize;
		*(uint16*)&header[0x0A] = image.getWidth();
		*(uint16*)&header[0x0C] = image.getHeight();
		header[0x0E] = std::log2f(image.getWidth());
		header[0x0F] = std::log2f(image.getHeight());
		
		for (uint32 i = 0; i < imgSize; i++){colourTable.push_back(ToSHTXPSPixel(*image[i]));
		}
		std::sort(colourTable.begin(), colourTable.end());
		colourTable.erase(std::unique(colourTable.begin(), colourTable.end()), colourTable.end());
		colourTable.resize(colourTableSize, 0x00);
		std::sort(colourTable.begin(), colourTable.end());
		*(uint16*)&header[0x06] = (uint16)colourTable.size();

		memcpy(&data[0], &header[0], sizeof(header));
		
		for (uint32 c : colourTable)
		{
			for (uint32 i = 0; i < 4; i++){data.push_back(((uint8*)&c)[i]);
			}
		}
		
		for (uint32 i = 0; i < imgSize; i++)
		{
			uint32 c = ToSHTXPSPixel(*image[i]);
			auto t = std::lower_bound(colourTable.begin(), colourTable.end(), c);
			uint16 index = std::distance(colourTable.begin(), t);
			
			data.push_back(index & 0x00FF); //Lowest byte first because little endian
			if (colourTableSize > 256){data.push_back((index & 0xFF00) >> 8);
			}
		}
		
		return data;
	}
};
#endif
