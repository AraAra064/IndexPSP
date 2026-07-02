#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <deque>

#include "KadaTools.hpp"
#include "Encoder.hpp"

#ifndef __INDEX_CRAF__
#define __INDEX_CRAF__

namespace IndexCRAF
{
	//Change function names later
	uint32_t GetNumberOfFiles(std::vector<uint8_t> &crafData){return *((uint32_t*)&crafData[0x08]);
	}
	uint32_t GetCRAFSize(std::vector<uint8_t> &crafData){return *((uint32_t*)&crafData[0x04]) + 0x10;
	}
	uint32_t GetCRAFSizeVal(std::vector<uint8_t> &crafData){return *((uint32_t*)&crafData[0x04]);
	}
	uint32_t SizeToCRAFVal(uint32_t size){return (size + (size % 0x10)) - 0x10;
	}
	
	std::vector<uint8_t> GetFileData(std::vector<uint8_t> &crafData, uint32_t index, bool decode = true)
	{
		uint32_t pos = 0x20, ro, fs;
		for (uint32_t i = 0; i <= index && pos < crafData.size(); i++)
		{
			ro = *((uint32_t*)&crafData[pos-0x08]);
			fs = *((uint32_t*)&crafData[pos-0x0C]);
			pos += ro;
		}
		pos -= ro;
		
		std::vector<uint8_t> fileData(crafData.begin() + pos, crafData.begin() + pos + fs);
		if (decode){fileData = KadaTools::DecodeRLE2(fileData);
		}
		
		return fileData;
	}
	std::deque<std::vector<uint8_t>> GetAllFiles(std::vector<uint8_t>& crafData, bool decode = true)
	{
		std::deque<std::vector<uint8_t>> files(GetNumberOfFiles(crafData));

		for (int i = 0; i < files.size(); i++)
		{
			files[i] = GetFileData(crafData, i, decode);
		}

		return files;
	}
	uint32_t GetFileType(std::vector<uint8_t> &crafData, uint32_t index)
	{
		uint32_t pos = 0x20, ro, ft;
		for (uint32_t i = 0; i <= index && pos < crafData.size(); i++)
		{
			ro = *((uint32_t*)&crafData[pos-0x08]);
			ft = *((uint32_t*)&crafData[pos-0x10]);
			pos += ro;
		}
		
		return ft;
	}
	std::string GetFileTypeStr(std::vector<uint8_t> &crafData, uint32_t index)
	{
		uint32_t fileType = GetFileType(crafData, index);
		
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
	
	bool CreateCRAF(std::string fileName, std::deque<std::vector<uint8_t>> &files)
	{
		bool retVal;
		const uint32_t zero = 0x00000000;
		
		std::string (*GetFileHeader)(std::vector<uint8_t>&) = [](std::vector<uint8_t> &data)->std::string
		{
			std::string header;
			for (uint32_t i = 0; i < data.size() && isalnum(data[i]); i++){header += data[i];
			}
			
			return header;
		};
		
		std::ofstream writeFile(fileName.c_str(), std::ios::binary);
		if ((retVal = writeFile.is_open()))
		{
			//CRAF Header
			writeFile.write("CRAF", 4); //MAGICNUM
			uint32_t crafSize = 0;
			writeFile.write((char*)&crafSize, sizeof(uint32_t)); //CRAFSIZE
			uint32_t v = files.size();
			writeFile.write((char*)&v, sizeof(uint32_t)); //NUMFILES
			writeFile.write((char*)&zero, sizeof(uint32_t)); //UNKNOWN
			
			for (uint32_t i = 0; i < files.size(); i++)
			{
				//File Header
				//std::cout<<GetFileHeader(files[i])<<std::endl;
				
				uint32_t fileType;
				switch (*(uint32_t*)&files[i][0])
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
				uint32_t fs = fileData.size() + (fileData.size() % 0x04); // + 1;
				uint32_t r = fileData.size() % 0x10;
				fileData.resize(fileData.size() + r, 0x00);
				uint32_t ro = fileData.size() + 0x10;
				
				writeFile.write((char*)&fileType, sizeof(uint32_t)); //FILETYPE
				writeFile.write((char*)&fs, sizeof(uint32_t)); //FILESIZE
				writeFile.write((char*)&ro, sizeof(uint32_t)); //ROFFSET
				writeFile.write((char*)&zero, sizeof(uint32_t)); //UNKNOWN
				
				writeFile.write((char*)&fileData[0], fileData.size() * sizeof(uint8_t)); //Encoded file data
				crafSize = (uint32_t)writeFile.tellp() - 0x10; //- r + 1;
			}
			
			writeFile.seekp(0x04);
			writeFile.write((char*)&crafSize, sizeof(uint32_t));
			
			writeFile.close();
		}
		
		return retVal;
	}
}

#endif
