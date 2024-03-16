#include <vector>
#include <fstream>

#include "IndexPSP\KadaTools.hpp"

#ifndef int8
	#define int8 int8_t
	#define uint8 uint8_t
	#define int16 int16_t
	#define uint16 uint16_t
	#define int32 int32_t
	#define uint32 uint32_t
	#define int64 int64_t
	#define uint64 uint64_t
#endif

#ifndef __ASSET_ARCHIVE__
	#define __ASSET_ARCHIVE__
#endif

#ifdef __ASSET_ARCHIVE__

//Name used to refer to the GRP, DAT, EVT... files
namespace AssetArchive
{
	const uint32 HEADER_SIZE = 0x14;
	
	struct FileInfo
	{
		uint32 size;
		uint32 offset;
		
		FileInfo(uint32 size = 0, uint32 offset = 0)
		{
			this->size = size;
			this->offset = offset;
		}
	};
	
	uint32 GetNumberOfFiles(std::vector<uint8> &data){return *(uint32*)&data[0x00];
	}
	uint32 GetFileChunkSize(std::vector<uint8> &data){return *(uint32*)&data[0x04];
	}
	uint32 GetFileSizeScale(std::vector<uint8> &data){return *(uint32*)&data[0x08];
	}
	uint32 GetFileOffsetShift(std::vector<uint8> &data){return *(uint32*)&data[0x0C];
	}
	uint32 GetMaskVal(std::vector<uint8> &data){return *(uint32*)&data[0x10];
	}
	
	std::vector<FileInfo> GetFileInfo(std::vector<uint8> &data)
	{
		std::vector<FileInfo> fileInfo;
		
		uint32 files = GetNumberOfFiles(data);
		uint32 chunkSize = GetFileChunkSize(data);
		uint32 fileScale = GetFileSizeScale(data);
		uint32 shiftVal = GetFileOffsetShift(data);
		uint32 maskVal = GetMaskVal(data);
	
		for (uint32 i = 0, pos = HEADER_SIZE; i < files; i++)
		{
			uint32 fi = *(uint32*)&data[pos];
			//memcpy(&fi, &data[pos], sizeof(uint32));

			fileInfo.push_back(FileInfo(((fi & maskVal) * fileScale) + 0x03FF, ((fi >> shiftVal) & ~maskVal) * chunkSize));
			pos += sizeof(uint32);
		}
		
		return fileInfo;
	}
	
	std::vector<uint8> GetFileData(std::vector<uint8> &data, std::vector<FileInfo> &fileInfo, uint32 index, bool decode = true)
	{
		std::vector<uint8> fileData;
		
		if (index < fileInfo.size())
		{
			uint32 pos = fileInfo[index].offset;
			uint32 size = fileInfo[index].size;
			
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
