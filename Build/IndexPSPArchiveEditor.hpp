#include <vector>
#include <fstream>
#include <map>
#include <unordered_map>
#include <thread>

#include "ConsoleGraphics.hpp"
#include "ConsoleUtilities.h"
#include "lodepng/lodepng.cpp"

#ifndef __ARCHIVE_EDITOR__
	#define __ARCHIVE_EDITOR__
#endif

#ifdef __ARCHIVE_EDITOR__

std::string IntToHexStr(uint32 n)
{
	std::string str;
	std::stringstream ss;
	ss << std::hex << n;
	str = ss.str();
	std::transform(str.begin(), str.end(), str.begin(), [](char c)->char {return toupper(c);});
	std::string s = std::string(str);
	return "0x" + ((s.size() % 2) != 0 ? std::string("0") : std::string()) + s;
}

std::string GetFileHeader(std::vector<uint8>& fileData)
{
	std::string header;
	for (uint32 i = 0; i < fileData.size() && isalnum(fileData[i]); i++) {
		header += fileData[i];
	}

	if (header.empty()) {
		header = "[UNKNOWN]";
	}
	return header;
}

std::string GetFileInfo(std::vector<uint8>& fileData)
{
	std::string fileInfo;

	uint32 v = 0x00000000;
	if (fileData.size() >= 4) {
		v = *(uint32*)&fileData[0];
	}

	switch (v)
	{
	case C_SHTXPS: //SHTX(PS)
		fileInfo = '[' + std::to_string(*(uint16*)&fileData[0x0A]) + ", " + std::to_string(*(uint16*)&fileData[0x0C]) + ']';
		break;

	case C_SSAD:
		for (uint32 i = 0x34; fileData[i] != '\0'; i++) {
			fileInfo += fileData[i];
		}
		break;

	case C_PSC6:
	{
		uint32* fileNameOffset = (uint32*)&fileData[0x44];
		for (uint32 i = *fileNameOffset; fileData[i] != '\0'; i++) {
			fileInfo += fileData[i];
		}
	}
	break;

	case C_CRAF:
	{
		fileInfo = std::to_string(IndexCRAF::GetNumberOfFiles(fileData)) + " files contained in CRAF \n";
		fileInfo += IndexCRAF::GetFileTypeStr(fileData, 0);
	}

	default:
		break;
	}

	return fileInfo;
}

struct ColourReductionSettings
{
	uint16 colourTableSize;
	uint8 mergeTolerance;
	float percentileStep;
	uint8 ditherMode;

	ColourReductionSettings()
	{
		colourTableSize = 256;
		mergeTolerance = 0;
		percentileStep = 0.78125f;
		ditherMode = 1;
	}
};

struct Settings
{
	uint32 windowWidth;
	uint32 windowHeight;
	uint8 pixelSize;

	Settings()
	{
		windowWidth = 1920 / 2;
		windowHeight = 1080 / 2;
		pixelSize = 2;
	}
};

struct GlobalData
{
	std::string archiveName;
	std::string archiveFileName;
	bool archiveEncoded;
	uint32 archiveIndex;

	//ColourReductionSettings crs
	Settings settings;
	Keyboard keyboard;
	//pmsg::PipeMsg pipe;

	GlobalData()
	{
		archiveEncoded = false;
		archiveIndex = 0;
	}
};

std::vector<uint8> LoadFile(std::string fileName)
{
	std::vector<uint8> retVal;
	std::ifstream readFile(fileName.c_str(), std::ios::binary);
	if (readFile.is_open())
	{
		retVal.assign(std::istreambuf_iterator<char>(readFile), std::istreambuf_iterator<char>());
		readFile.close();
	}
	
	return retVal;
}

uint32 matDist(std::pair<uint32, uint8> &colourA, std::pair<uint32, uint8> &colourB)
{
	uint32 dist = 0;
	dist += abs((int)cg::GetR(colourA.first)-(int)cg::GetR(colourB.first));
	dist += abs((int)cg::GetG(colourA.first)-(int)cg::GetG(colourB.first));
	dist += abs((int)cg::GetB(colourA.first)-(int)cg::GetB(colourB.first));
	dist += abs((int)colourA.second-(int)colourB.second);
	return dist;
}

uint32 _matDist(uint32 &colourA, std::pair<uint32, uint8> &colourB)
{
	uint32 dist = 0;
	dist += abs((int)cg::GetR(colourA)-(int)cg::GetR(colourB.first));
	dist += abs((int)cg::GetG(colourA)-(int)cg::GetG(colourB.first));
	dist += abs((int)cg::GetB(colourA)-(int)cg::GetB(colourB.first));
	dist += abs((int)cg::GetA(colourA)-(int)colourB.second);
	return dist;
}

uint32 __matDist(uint32 &colourA, uint32 &colourB)
{
	uint32 dist = 0;
	dist += abs((int)cg::GetR(colourA)-(int)cg::GetR(colourB));
	dist += abs((int)cg::GetG(colourA)-(int)cg::GetG(colourB));
	dist += abs((int)cg::GetB(colourA)-(int)cg::GetB(colourB));
	dist += abs((int)cg::GetA(colourA)-(int)cg::GetA(colourB));
	return dist;
}

struct ColourData
{
	uint32 c;
	uint32 n;
	
	ColourData()
	{
		c = 0;
		n = 0;
	}
	ColourData(uint32 c, uint32 n)
	{
		this->c = c;
		this->n = n;
	}
};

/*const*/	int	BAYER_PATTERN_16X16[16][16]	=	{	//	16x16 Bayer Dithering Matrix.  Color levels: 256
												{	  0, 191,  48, 239,  12, 203,  60, 251,   3, 194,  51, 242,  15, 206,  63, 254	}, 
												{	127,  64, 175, 112, 139,  76, 187, 124, 130,  67, 178, 115, 142,  79, 190, 127	},
												{	 32, 223,  16, 207,  44, 235,  28, 219,  35, 226,  19, 210,  47, 238,  31, 222	},
												{	159,  96, 143,  80, 171, 108, 155,  92, 162,  99, 146,  83, 174, 111, 158,  95	},
												{	  8, 199,  56, 247,   4, 195,  52, 243,  11, 202,  59, 250,   7, 198,  55, 246	},
												{	135,  72, 183, 120, 131,  68, 179, 116, 138,  75, 186, 123, 134,  71, 182, 119	},
												{	 40, 231,  24, 215,  36, 227,  20, 211,  43, 234,  27, 218,  39, 230,  23, 214	},
												{	167, 104, 151,  88, 163, 100, 147,  84, 170, 107, 154,  91, 166, 103, 150,  87	},
												{	  2, 193,  50, 241,  14, 205,  62, 253,   1, 192,  49, 240,  13, 204,  61, 252	},
												{	129,  66, 177, 114, 141,  78, 189, 126, 128,  65, 176, 113, 140,  77, 188, 125	},
												{	 34, 225,  18, 209,  46, 237,  30, 221,  33, 224,  17, 208,  45, 236,  29, 220	},
												{	161,  98, 145,  82, 173, 110, 157,  94, 160,  97, 144,  81, 172, 109, 156,  93	},
												{	 10, 201,  58, 249,   6, 197,  54, 245,   9, 200,  57, 248,   5, 196,  53, 244	},
												{	137,  74, 185, 122, 133,  70, 181, 118, 136,  73, 184, 121, 132,  69, 180, 117	},
												{	 42, 233,  26, 217,  38, 229,  22, 213,  41, 232,  25, 216,  37, 228,  21, 212	},
												{	169, 106, 153,  90, 165, 102, 149,  86, 168, 105, 152,  89, 164, 101, 148,  85	}
											};

void Process(cg::Image *image, uint32 posY, uint32 height, std::vector<ColourData> ct, bool floydDither)
{
	for (uint32 y = posY; y < posY+height && y < image->getHeight(); y++)
	{
		for (uint32 x = 0; x < image->getWidth(); x++)
		{
			uint32 index = 0;
			auto colour = *image->accessPixel(x, y);
			std::pair<uint32, uint8> _c;
			bool findClosest = false;
			
			for (uint32 i = 0, dLast = 0xFFFFFFFF; i < ct.size(); ++i)
			{
				auto &c = ct[i];
				uint32 d = _matDist(c.c, colour);
				if (d > dLast){continue;
				}
				if ((findClosest = (d != 0)))
				{
					dLast = d;
					index = i;
				} else break;
			}
			
			if (findClosest){_c = std::make_pair(ct[index].c & 0x00FFFFFF, (ct[index].c & 0xFF000000) >> 24);
			} else _c = colour;
			
			*image->accessPixel(x, y) = _c;
			
			if (!floydDither){continue;
			}
			uint8 newR, newG, newB, newA;
			newR = cg::GetR(_c.first);
			newG = cg::GetG(_c.first);
			newB = cg::GetB(_c.first);
			newA = _c.second;
			int16 errR, errG, errB, errA;
			errR = cg::GetR(colour.first) - newR;
			errG = cg::GetG(colour.first) - newG;
			errB = cg::GetB(colour.first) - newB;
			errA = colour.second - newA;
			const float c1 = 7.f/16.f, c2 = 3.f/16.f, c3 = 5.f/16.f, c4 = 1.f/16.f;
			
			int16 r, g, b, a;
			if (x != image->getWidth()-1)
			{
				auto p = image->accessPixel(x+1, y);
				if (p->second != 0)
				{
					r = std::min<int16>(cg::GetR(p->first) + c1 * errR, 255);
					g = std::min<int16>(cg::GetG(p->first) + c1 * errG, 255);
					b = std::min<int16>(cg::GetB(p->first) + c1 * errB, 255);
					a = std::min<int16>(p->second + c1 * errA, 255);
					r = r < 0 ? 0 : r;
					g = g < 0 ? 0 : g;
					b = b < 0 ? 0 : b;
					a = a < 0 ? 0 : a;
					*p = std::make_pair(cg::BGR(r, g, b), a);
				} else *p = std::make_pair(0, 0);
			}
			if (x != 0 && y != image->getHeight()-1)
			{
				auto p = image->accessPixel(x-1, y+1);
				if (p->second != 0)
				{
					r = std::min<int16>(cg::GetR(p->first) + c2 * errR, 255);
					g = std::min<int16>(cg::GetG(p->first) + c2 * errG, 255);
					b = std::min<int16>(cg::GetB(p->first) + c2 * errB, 255);
					a = std::min<int16>(p->second + c2 * errA, 255);
					r = r < 0 ? 0 : r;
					g = g < 0 ? 0 : g;
					b = b < 0 ? 0 : b;
					a = a < 0 ? 0 : a;
					*p = std::make_pair(cg::BGR(r, g, b), a);
				} else *p = std::make_pair(0, 0);
			}
			if (y != image->getHeight()-1)
			{
				auto p = image->accessPixel(x, y+1);
				if (p->second != 0)
				{
					r = std::min<int16>(cg::GetR(p->first) + c3 * errR, 255);
					g = std::min<int16>(cg::GetG(p->first) + c3 * errG, 255);
					b = std::min<int16>(cg::GetB(p->first) + c3 * errB, 255);
					a = std::min<int16>(p->second + c3 * errA, 255);
					r = r < 0 ? 0 : r;
					g = g < 0 ? 0 : g;
					b = b < 0 ? 0 : b;
					a = a < 0 ? 0 : a;
					*p = std::make_pair(cg::BGR(r, g, b), a);
				} else *p = std::make_pair(0, 0);
			}
			if (x != image->getWidth()-1 && y != image->getHeight()-1)
			{
				auto p = image->accessPixel(x+1, y+1);
				if (p->second != 0)
				{
					r = std::min<int16>(cg::GetR(p->first) + c4 * errR, 255);
					g = std::min<int16>(cg::GetG(p->first) + c4 * errG, 255);
					b = std::min<int16>(cg::GetB(p->first) + c4 * errB, 255);
					a = std::min<int16>(p->second + c4 * errA, 255);
					r = r < 0 ? 0 : r;
					g = g < 0 ? 0 : g;
					b = b < 0 ? 0 : b;
					a = a < 0 ? 0 : a;
					*p = std::make_pair(cg::BGR(r, g, b), a);
				} else *p = std::make_pair(0, 0);
			}
		}
	}
	
	return;
}

//ditherMode = 0, no dithering
//ditherMode = 1, bayer dither
//ditherMode = 2, floyd dither
//-----
//reqThreads = if possible, use "this" many threads
void ReduceImageColours(cg::Image &inImage, cg::Image &outImage, uint16 colourTableSize = 256, uint8 mergeTolerance = 0, float percentile = 1.25f, uint8 ditherMode = 1, uint32 reqThreads = 2)
{
	std::unordered_map<uint32, uint32> colourTable;
	std::vector<ColourData> ct;
	colourTable[0x00000000] = 0xFFFFFFFF;
	
	static bool div = false;
	if (!div)
	{
		for (uint32 i = 0; i < 16; i++)
		{
			for (uint32 j = 0; j < 16; j++){BAYER_PATTERN_16X16[i][j] /= (4*4);
			}
		}
		div = true;
	}
	
	outImage = inImage;
	for (uint32 y = 0; y < outImage.getHeight(); y++)
	{
		for (uint32 x = 0; x < outImage.getWidth(); x++)
		{
			auto colour = outImage.accessPixel(x, y);
			
			if (colour->second == 0){*colour = std::make_pair(0, 0);
			}
			
			uint32 c = cg::BGRA(colour->first, colour->second);
			if (colourTable[c] != -1)
			{
				colourTable[c]++;
			}
			
			if (ditherMode == 1)
			{
				int16 r = std::max<int16>(std::min<int16>(cg::GetR(colour->first) + (BAYER_PATTERN_16X16[y % 16][x % 16] - 0.5f), 255), 0);
				int16 g = std::max<int16>(std::min<int16>(cg::GetG(colour->first) + (BAYER_PATTERN_16X16[y % 16][x % 16] - 0.5f), 255), 0);
				int16 b = std::max<int16>(std::min<int16>(cg::GetB(colour->first) + (BAYER_PATTERN_16X16[y % 16][x % 16] - 0.5f), 255), 0);
				int16 a = std::max<int16>(std::min<int16>(colour->second + (BAYER_PATTERN_16X16[y % 16][x % 16] - 0.5f), 255), 0);
				
				colour->first = cg::BGR(r, g, b);
				colour->second = a;
			}
		}
	}
	
	if (colourTable.size() <= colourTableSize){return;
	}
	
	ct.reserve(colourTable.size());
	for (auto c = colourTable.begin(); c != colourTable.end(); c++){ct.push_back(ColourData(c->first, c->second));
	}
	std::sort(ct.begin(), ct.end(), [](ColourData&a, ColourData&b)->bool{return a.n < b.n;});
	
	uint32 iterations = (uint32)(100.f / percentile);
	for (uint32 i = 1, s = (uint32)round(colourTableSize * (percentile / 100.f)), pos = s+(colourTableSize % s); i < iterations && pos < ct.size(); i++)
	{
		memcpy(&ct[pos], &ct[(int)(ct.size() * ((float)i/(float)iterations))-(s/2)], s * sizeof(ColourData));
		pos += s;
	}
	
	ct.resize(colourTableSize);
	
	//Find similar colours and remove them from the colour table
	if (mergeTolerance != 0)
	{
		uint32 c = 0;
		for (uint32 i = 0; i < ct.size()-c; i++)
		{
			for (uint32 j = i; j < ct.size()-c; j++)
			{
				if (__matDist(ct[i].c, ct[j].c) < mergeTolerance)
				{
					if (ct[i].n < ct[j].n){std::swap(ct[i], ct[j]);
					}
					std::swap(ct[j], ct[ct.size()-c]);
					c++;
					break;
				}
			}
		}
		
		ct.erase(ct.end()-c, ct.end());
		std::sort(ct.begin(), ct.end(), [](ColourData&a, ColourData&b)->bool{return a.n < b.n;});
	}
	
	bool floydDither = (ditherMode == 2);
	uint32 threadCount = (floydDither ? 1 : reqThreads);
	uint32 startY = 0, yStep;
	uint32 height = outImage.getHeight() / threadCount;
	yStep = height / threadCount;
	std::vector<std::thread> threads;
	for (uint32 i = 0; i < threadCount; i++)
	{
		if (i == threadCount-1){height = outImage.getHeight()-startY;
		}
		threads.push_back(std::thread(Process, &outImage, startY, height, ct, floydDither));
		startY += yStep;
	}
	
	for (std::thread &t : threads){t.join();
	}
	
	return;
}

void drawImage(cg::ConsoleGraphics &graphics, cg::Image &image, float imgScale = 1.f, cg::DrawType drawMode = cg::DrawType::Repeat, void (*drawFunc)(std::pair<uint32, uint8>*, void*) = nullptr, void *data = nullptr)
{
	int32 srcX = image.getPosX(), srcY = image.getPosY();
	uint32 dstX = srcX < 0 ? 0 : srcX, dstY = srcY < 0 ? 0 : srcY;
	uint32 w = srcX < 0 ? image.getWidth()-abs(srcX) : image.getWidth(), h = srcY < 0 ? image.getHeight()-abs(srcY) : image.getHeight();
	graphics.drawEX(image, srcX < 0 ? abs(srcX) : 0, srcY < 0 ? abs(srcY) : 0, dstX, dstY, w*imgScale, h*imgScale, drawMode, drawFunc, data);
	return;
}

bool LoadPNG(cg::Image &image, std::string fileName)
{
	std::vector<uint8> pixelData;
	unsigned int width, height;
	bool v = (lodepng::decode(pixelData, width, height, fileName) == 0);
	if (v)
	{
		for (uint32 i = 0; i < pixelData.size()-3; i += 4){std::swap(pixelData[i], pixelData[i+2]); //RGB to BGR
		}
		image.loadImageFromArray((uint32*)pixelData.data(), width, height, true);
	}
	return v;
}

bool LoadImage(cg::Image &image, std::string fileName)
{
	std::string str = fileName;
	std::transform(str.begin(), str.end(), str.begin(), [](char c)->char{return toupper(c);});
	
	bool b;
	if (str.find(".PNG") != std::string::npos){b = LoadPNG(image, fileName);
	} else b = (image.loadImage(fileName) != 0);
	return b;
}

namespace ArchiveEditor
{
	std::string ReplaceCommand(uint32 fileIndex, std::string fileName, uint16 scale){return std::to_string(fileIndex)+':'+fileName+':'+std::to_string(scale)+'\n';
	}
	std::string ResizeCommand(uint32 size){return '='+std::to_string(size)+'\n';
	}
	std::string DirectoryCommand(std::string dir){return '!'+dir+'\n';
	}
	std::string ListVersionCommand(uint32 version){return '*'+std::to_string(version)+'\n';
	}
};

#endif
