#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <deque>

#include "IndexPSP\KadaTools.hpp"
#include "IndexPSP\Encoder.hpp"

#ifndef __CRAF_EDITOR__
#define __CRAF_EDITOR__

namespace IndexCRAF
{
	//Change function names later
	uint32 GetNumberOfFiles(std::vector<uint8> &crafData){return *((uint32*)&crafData[0x08]);
	}
	uint32 GetCRAFSize(std::vector<uint8> &crafData){return *((uint32*)&crafData[0x04]) + 0x10;
	}
	uint32 GetCRAFSizeVal(std::vector<uint8> &crafData){return *((uint32*)&crafData[0x04]);
	}
	uint32 SizeToCRAFVal(uint32 size){return (size + (size % 0x10)) - 0x10;
	}
	
	std::vector<uint8> GetFileData(std::vector<uint8> &crafData, uint32 index, bool decode = true)
	{
		uint32 pos = 0x20, ro, fs;
		for (uint32 i = 0; i <= index && pos < crafData.size(); i++)
		{
			ro = *((uint32*)&crafData[pos-0x08]);
			fs = *((uint32*)&crafData[pos-0x0C]);
			pos += ro;
		}
		pos -= ro;
		
		std::vector<uint8> fileData(crafData.begin() + pos, crafData.begin() + pos + fs);
		if (decode){fileData = KadaTools::DecodeRLE2(fileData);
		}
		
		return fileData;
	}
	uint32 GetFileType(std::vector<uint8> &crafData, uint32 index)
	{
		uint32 pos = 0x20, ro, ft;
		for (uint32 i = 0; i <= index && pos < crafData.size(); i++)
		{
			ro = *((uint32*)&crafData[pos-0x08]);
			ft = *((uint32*)&crafData[pos-0x10]);
			pos += ro;
		}
		
		return ft;
	}
	std::string GetFileTypeStr(std::vector<uint8> &crafData, uint32 index)
	{
		uint32 fileType = GetFileType(crafData, index);
		
		std::string str;
		switch (fileType)
		{
			case 0x01:
				str = "PSC6";
				break;
			case 0x02:
				str = "PSM0";
				break;
			case 0x03:
				str = "SHTXPS";
				break;
			case 0x04:
				str = "PTC";
				break;
			case 0x05:
				str = "UNKNOWN BINARY";
				break;
			case 0x06:
				str = "SSAD";
				break;
			
			default:
				str = "NOT KNOWN";
				break;
		}
		
		return str;
	}
	
	bool CreateFile(std::string fileName, std::deque<std::vector<uint8>> &files)
	{
		bool retVal;
		const uint32 zero = 0x00000000;
		
		std::string (*GetFileHeader)(std::vector<uint8>&) = [](std::vector<uint8> &data)->std::string
		{
			std::string header;
			for (uint32 i = 0; i < data.size() && isalnum(data[i]); i++){header += data[i];
			}
			
			return header;
		};
		
		std::ofstream writeFile(fileName.c_str(), std::ios::binary);
		if ((retVal = writeFile.is_open()))
		{
			//CRAF Header
			writeFile.write("CRAF", 4); //MAGICNUM
			uint32 crafSize = 0;
			writeFile.write((char*)&crafSize, sizeof(uint32)); //CRAFSIZE
			uint32 v = files.size();
			writeFile.write((char*)&v, sizeof(uint32)); //NUMFILES
			writeFile.write((char*)&zero, sizeof(uint32)); //UNKNOWN
			
			for (uint32 i = 0; i < files.size(); i++)
			{
				//File Header
				//std::cout<<GetFileHeader(files[i])<<std::endl;
				
				uint32 fileType;
				switch (*(uint32*)&files[i][0])
				{
					case 0x36435350: //PSC6
						fileType = 0x01;
						break;
					case 0x304D5350: //PSM0
						fileType = 0x02;
						break;
					case 0x58544853: //SHTX
						fileType = 0x03;
						break;
					case 0x00435450: //PTC
						fileType = 0x04;
						break;
					default: //UNKNOWN BINARY
						fileType = 0x05;
						break;
					case 0x44415353: //SSAD
						fileType = 0x06;
						break;
				}
				
				auto fileData = IndexEncoder::EncodeRLE3(files[i]);
				uint32 fs = fileData.size() + (fileData.size() % 0x04); // + 1;
				uint32 r = fileData.size() % 0x10;
				fileData.resize(fileData.size() + r, 0x00);
				uint32 ro = fileData.size() + 0x10;
				
				writeFile.write((char*)&fileType, sizeof(uint32)); //FILETYPE
				writeFile.write((char*)&fs, sizeof(uint32)); //FILESIZE
				writeFile.write((char*)&ro, sizeof(uint32)); //ROFFSET
				writeFile.write((char*)&zero, sizeof(uint32)); //UNKNOWN
				
				writeFile.write((char*)&fileData[0], fileData.size() * sizeof(uint8)); //Encoded file data
				crafSize = (uint32)writeFile.tellp() - 0x10; //- r + 1;
			}
			
			writeFile.seekp(0x04);
			writeFile.write((char*)&crafSize, sizeof(uint32));
			
			writeFile.close();
		}
		
		return retVal;
	}
}

#endif
