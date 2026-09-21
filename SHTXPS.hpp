#include <vector>
#include <algorithm>
#include <unordered_map>

#ifndef __SHTXPS_PARSER__
#define __SHTXPS_PARSER__

namespace IndexSHTXPS
{
	struct Header
	{
		const char magic[6] = {'S', 'H', 'T', 'X', 'P', 'S'};
		uint16_t numColours;
		uint16_t renderMode = 0x01;
		uint16_t width;
		uint16_t height;
		uint8_t l2Width;
		uint8_t l2Height;
		uint32_t size;
	};

	struct Colour
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;

		Colour(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0, uint8_t a = 0)
		{
			this->r = r;
			this->g = g;
			this->b = b;
			this->a = a;
		}
	};

	struct ImageInfo
	{
		uint16_t numColours;
		uint16_t width;
		uint16_t height;

		ImageInfo(uint16_t numColours = 0, uint16_t width = 0, uint16_t height = 0)
		{
			this->numColours = numColours;
			this->width = width;
			this->height = height;
		}
	};

	const uint32_t C_HEADERSIZE = sizeof(Header); //20
	
	//RGBA -> BGRA
	uint32_t SHTXToBGRA(uint32_t p){return (_byteswap_ulong(p) >> 8 | (p & 0xFF000000));
	}
	//BGRA -> RGBA
	uint32_t BGRAToSHTX(uint32_t p){return (_byteswap_ulong(p << 8)) | (p & 0xFF000000);
	}

	//Game only checks first 4 bytes but doing first 6 anyway
	bool IsValid(std::vector<uint8_t>& data)
	{
		if (!data.empty())
		{
			std::string magic((char*)&data[0], 6);
			return (magic == "SHTXPS");
		}

		return false;
	}

	ImageInfo GetImageInfo(std::vector<uint8_t>& shtxData)
	{
		Header* h = (Header*)shtxData.data();
		ImageInfo ii;
		ii.numColours = h->numColours;
		ii.width = h->width;
		ii.height = h->height;
		return ii;
	}

	std::vector<uint8_t> CreateHeader(ImageInfo info)
	{
		Header h;
		h.numColours = info.numColours;
		h.width = info.width;
		h.height = info.height;
		h.l2Width = std::log2f(info.width);
		h.l2Height = std::log2f(info.height);
		h.size = info.width * info.height;

		uint8_t* it = (uint8_t*)&h;
		uint8_t* it2 = (uint8_t*)&h + sizeof(Header);

		return std::vector<uint8_t>(it, it2);
	}

	//Returns an array of pixel data, defaults to (0xAARRGGBB)
	//(outWidth, outHeight) = size of image
	//Returns [](shtxps, 0, 0) on failure
	std::vector<uint32_t> GetPixelData(std::vector<uint8_t>& shtxData, uint32_t& outWidth, uint32_t& outHeight, bool isRGBA = false)
	{
		outWidth = 0;
		outHeight = 0;
		std::vector<uint32_t> pixels;

		if (IsValid(shtxData))
		{
			auto info = GetImageInfo(shtxData);
			outWidth = info.width;
			outHeight = info.height;

			std::vector<uint32_t> colourTable(info.numColours);
			memcpy(&colourTable[0], &shtxData[C_HEADERSIZE], info.numColours * 4);

			uint32_t size = info.width * info.height;
			uint32_t index = C_HEADERSIZE + (info.numColours * 4);
			pixels.resize(size);
			for (uint32_t i = 0; i < size; i++)
			{
				uint32_t pixel = colourTable[shtxData[index + i]];
				pixels[i] = isRGBA ? pixel : SHTXToBGRA(pixel);
			}
		}

		return pixels;
	}

	//Expects a maximum of 256 different colours, in 0xAABBGGRR
	//Undefined behaviour when the total unique colours is greater than 256
	//Returns a valid SHTXPS file to be added to the game
	std::vector<uint8_t> CreateSHTX(std::vector<uint32_t>& pixels, uint32_t outWidth, uint32_t outHeight, std::vector<uint32_t> colourTable)
	{
		std::vector<uint8_t> shtxData;
		
		if (!pixels.empty())
		{
			if (colourTable.empty())
			{
				//colourTable = pixels;
				//std::sort(colourTable.begin(), colourTable.end());
				//colourTable.erase(std::unique(colourTable.begin(), colourTable.end()), colourTable.end());
				
				return shtxData;
			}

			uint32_t colourTableSize = colourTable.size() * 0x04;
			uint32_t colourIndexSize = (outWidth * outHeight);
			shtxData.resize(C_HEADERSIZE + colourTableSize + colourIndexSize);

			auto header = CreateHeader(ImageInfo(colourTable.size(), outWidth, outHeight));
			memcpy(&shtxData[0], &header[0], C_HEADERSIZE);
			memcpy(&shtxData[C_HEADERSIZE], &colourTable[0], colourTableSize);

			std::unordered_map<uint32_t, uint16_t> cIndexTable;
			for (uint16_t index = 0; index < colourTable.size(); index++)
			{
				cIndexTable[colourTable[index]] = index;
			}

			for (uint32_t i = 0; i < colourIndexSize; i++)
			{
				uint32_t c = pixels[i];
				uint16_t index = cIndexTable[c];
			
				shtxData[C_HEADERSIZE + colourTableSize + i] = index;
			}
		}

		return shtxData;
	}

};
#endif
