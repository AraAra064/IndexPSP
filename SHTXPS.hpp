#include <vector>

#include "ConsoleGraphics1.hpp"

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
	Image LoadImage(std::vector<uint8> &data, bool *isLoaded = nullptr)
	{
		std::vector<std::pair<uint32, uint8>> colourTable;
		Image image;
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
		std::vector<uint32> colourTable;
		std::vector<uint8> data;
		
		if (oldHeader != nullptr && *(uint32*)oldHeader == 0x58544853)
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
		
		for (uint32 i = 0; i < imgSize; i++){colourTable.push_back(ToSHTXPSPixel(*image[i]));
		}
		std::sort(colourTable.begin(), colourTable.end());
		colourTable.erase(std::unique(colourTable.begin(), colourTable.end()), colourTable.end());
		colourTable.resize(colourTableSize, 0x00);
		std::sort(colourTable.begin(), colourTable.end());
		*(uint16*)&data[0x06] = (uint16)colourTable.size();
		
		for (uint32 c : colourTable)
		{
			for (uint32 i = 0; i < 4; i++){data.push_back(((uint8*)&c)[i]);
			}
		}
		
//		std::cout<<"MAKE"<<std::endl;
		for (uint32 i = 0; i < imgSize; i++)
		{
			uint32 c = ToSHTXPSPixel(*image[i]);
			auto t = std::lower_bound(colourTable.begin(), colourTable.end(), c);
			uint16 index = std::distance(colourTable.begin(), t);
//			std::cout<<index<<std::endl;
			
			data.push_back(index & 0x00FF); //Lowest byte first because little endian
			if (colourTableSize > 256){data.push_back((index & 0xFF00) >> 8);
			}
		}
		
		return data;
	}
};

//if (image != nullptr && oldHeader != nullptr)
//		{
//			for (uint32 i = 0; i < HeaderOffset; i++){data.push_back(oldHeader[i]);
//			}
//			
////			data[0x08] = 0x00;
//			*(uint16*)&data[0x0A] = image->getWidth();
//			*(uint16*)&data[0x0C] = image->getHeight();
//			uint32 imgSize = image->getWidth() * image->getHeight();
////			*(uint16*)&data[0x0E] = imgSize; //Encoded data size?
////			*(uint16*)&data[0x10] = imgSize;
//			
//			for (uint32 i = 0, j = 0; i < imgSize; i++){colourTable.push_back(ToSHTXPSPixel(*(*image)[i]));
//			}
//			std::sort(colourTable.begin(), colourTable.end());
//			colourTable.erase(std::unique(colourTable.begin(), colourTable.end()), colourTable.end());
//////			colourTable.resize(256, 0x00);
//			std::sort(colourTable.begin(), colourTable.end());
//			*(uint16*)&data[0x06] = (uint16)colourTable.size();
////			std::cout<<colourTable.size()<<std::endl;
////			std::cout<<image->getWidth()<<char(158)<<image->getHeight()<<std::endl;
//			for (auto &c : colourTable)
//			{
//				for (uint32 i = 0; i < 4; i++){data.push_back(((uint8*)&c)[i]);
//				}
//			}
//			
//			for (uint32 i = 0; i < imgSize; i++)
//			{
//				uint32 c = ToSHTXPSPixel(*(*image)[i]);
//				auto t = std::lower_bound(colourTable.begin(), colourTable.end(), c);
//				uint16 j = std::distance(colourTable.begin(), t);
//				
//				if (colourTable.size() > 256)
//				{
//					uint8 j0 = j & 0x00FF;
//					uint8 j1 = (j & 0xFF00) >> 8;
//					data.push_back(j0);
//					data.push_back(j1);
//				} else data.push_back(j);
//			}
//		}

#endif
