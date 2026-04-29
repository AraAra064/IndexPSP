#include <vector>
#include <algorithm>

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
		ImageInfo ii;
		ii.numColours = *(uint16_t*)&shtxData[0x06];
		ii.width = *(uint16_t*)&shtxData[0x0A];
		ii.height = *(uint16_t*)&shtxData[0x0C];
		return ii;
	}

	std::vector<uint8_t> CreateHeader(ImageInfo info)
	{
		std::vector<uint8_t> header(C_HEADERSIZE, 0x00);
		memcpy(&header[0x00], "SHTXPS", 0x06);
		memcpy(&header[0x06], &info.numColours, 0x02);
		header[0x08] = 1; //"rendermode"
		memcpy(&header[0x0A], &info.width, 0x02);
		memcpy(&header[0x0C], &info.height, 0x02);
		header[0x0E] = std::log2f(info.width);
		header[0x0F] = std::log2f(info.height);
		uint16_t size = info.width * info.height;
		memcpy(&header[0x10], &size, 0x02);

		return header;
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

	//Expects a maximum of 256 different colours
	//No idea what happens when there is more than 256
	std::vector<uint8_t> CreateSHTX(std::vector<uint32_t>& pixels, uint32_t& outWidth, uint32_t& outHeight)
	{
		std::vector<uint8_t> shtxData;
		
		if (!pixels.empty())
		{
			std::vector<uint32_t> colourTable;
			for (uint32_t p : pixels){colourTable.push_back(BGRAToSHTX(p));
			}
			std::sort(colourTable.begin(), colourTable.end());
			colourTable.erase(std::unique(colourTable.begin(), colourTable.end()), colourTable.end());
			std::sort(colourTable.begin(), colourTable.end());
			uint32_t colourTableSize = colourTable.size() * 0x04;
			uint32_t colourIndexSize = (outWidth * outHeight);
			shtxData.resize(C_HEADERSIZE + colourTableSize + colourIndexSize);

			auto header = CreateHeader(ImageInfo(colourTable.size(), outWidth, outHeight));
			memcpy(&shtxData[0], &header[0], C_HEADERSIZE);
			memcpy(&shtxData[0], &colourTable[0], colourTableSize);

			for (uint32_t i = 0; i < colourIndexSize; i++)
			{
				uint32_t c = BGRAToSHTX(pixels[i]);
				auto t = std::lower_bound(colourTable.begin(), colourTable.end(), c);
				uint16_t index = std::distance(colourTable.begin(), t);

				shtxData[C_HEADERSIZE + colourTableSize + i] = index;
			}
		}

		return shtxData;
	}

};
#endif
