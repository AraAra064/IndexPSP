#include <vector>
#include <fstream>

#include "KadaTools.hpp"

#ifndef __ASSET_ARCHIVE__
	#define __ASSET_ARCHIVE__
#endif

#ifdef __ASSET_ARCHIVE__

//Name used to refer to the GRP, DAT, EVT... files
namespace AssetArchive
{
	const uint32_t HEADER_SIZE = 0x14;
	
	struct FileInfo
	{
		uint32_t size;
		uint32_t offset;
		
		FileInfo(uint32_t size = 0, uint32_t offset = 0)
		{
			this->size = size;
			this->offset = offset;
		}
	};
	
	uint32_t GetNumberOfFiles(std::vector<uint8_t> &data){return *(uint32_t*)&data[0x00];
	}
	uint32_t GetFileChunkSize(std::vector<uint8_t> &data){return *(uint32_t*)&data[0x04];
	}
	uint32_t GetFileSizeScale(std::vector<uint8_t> &data){return *(uint32_t*)&data[0x08];
	}
	uint32_t GetFileOffsetShift(std::vector<uint8_t> &data){return *(uint32_t*)&data[0x0C];
	}
	uint32_t GetMaskVal(std::vector<uint8_t> &data){return *(uint32_t*)&data[0x10];
	}
	
	std::vector<FileInfo> GetFileInfo(std::vector<uint8_t> &data)
	{
		std::vector<FileInfo> fileInfo;
		
		uint32_t files = GetNumberOfFiles(data);
		uint32_t chunkSize = GetFileChunkSize(data);
		uint32_t fileScale = GetFileSizeScale(data);
		uint32_t shiftVal = GetFileOffsetShift(data);
		uint32_t maskVal = GetMaskVal(data);
	
		for (uint32_t i = 0, pos = HEADER_SIZE; i < files; i++)
		{
			uint32_t fi = *(uint32_t*)&data[pos];
			
			fileInfo.push_back(FileInfo(((fi & maskVal) * fileScale) + 0x03FF, (fi >> shiftVal) * chunkSize));
			pos += sizeof(uint32_t);
		}
		
		return fileInfo;
	}
	
	std::vector<uint8_t> GetFileData(std::vector<uint8_t> &data, std::vector<FileInfo> &fileInfo, uint32_t index, bool decode = true)
	{
		std::vector<uint8_t> fileData;
		
		if (index < fileInfo.size())
		{
			uint32_t pos = fileInfo[index].offset;
			uint32_t size = fileInfo[index].size;
			
			fileData.resize(size);
			memcpy(&fileData[0], &data[pos], size);
			
			if (decode){fileData = KadaTools::DecodeRLE2(fileData);
			}
		}
		
		return fileData;
	}
	
//	bool SaveArchive(std::string fileName, )
};

#endif
