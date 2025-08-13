#include <vector>
#include <fstream>

#include "IndexPSP\KadaTools.hpp"
#include "IndexPSP\Encoder.hpp"

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
			uint32 fi;// = *(uint32*)&data[pos];
			memcpy(&fi, &data[pos], sizeof(uint32));

			//((fi >> shiftVal) & ~maskVal)
			FileInfo x;
			fileInfo.push_back(FileInfo(((fi & maskVal) * fileScale) + 0x03FF, (fi >> shiftVal) * chunkSize));
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
			
			if ((pos + size) < data.size()) //ADV silly
			{
				fileData.resize(size);
				memcpy(&fileData[0], &data[pos], size);
				fileData = decode ? KadaTools::DecodeRLE2(fileData) : fileData;
			} else fileData = std::vector<uint8>(1, 0xFF);
		}
		
		return fileData;
	}
	
	//Used for SaveArchive function
	const uint32 IsEncoded = 0x10;

	//Encoder
	//LS 4 bits = encoder version (encoderVer = encoder & 0x0F)
	//next bit = is encoded (is encoded, bool = (encoder & 0x10)
	//SaveArchive(archiveData, fileInfo, newFiles, "GRP.bin", 4 | AssetArchive::IsEncoded)
	bool SaveArchive(std::vector<uint8> &archiveData, std::vector<FileInfo> &fileInfo, std::deque<std::vector<uint8>> &files, std::string fileName, uint32 encoder)
	{
		bool success = true;
		uint32 encoderVer = encoder & 0x0F;
		bool isEncoded = ((encoder & 0x10) == 0x10);

		std::ofstream writeFile(fileName.c_str(), std::ios::binary);
		success = writeFile.is_open();

		uint32 chunkSize;
		uint32 fileScale;
		uint32 fileOffsetShift;
		uint32 maskVal;
		std::vector<uint32> fileTable;
		uint32 paddingSize;
		uint32 offset;

		if (success)
		{
			uint32 n = files.size();
			writeFile.write((char*)&n, sizeof(uint32));
			chunkSize = AssetArchive::GetFileChunkSize(archiveData);
			writeFile.write((char*)&chunkSize, sizeof(uint32));
			fileScale = AssetArchive::GetFileSizeScale(archiveData);
			writeFile.write((char*)&fileScale, sizeof(uint32));
			fileOffsetShift = AssetArchive::GetFileOffsetShift(archiveData);
			writeFile.write((char*)&fileOffsetShift, sizeof(uint32));
			maskVal = AssetArchive::GetMaskVal(archiveData);
			writeFile.write((char*)&maskVal, sizeof(uint32));

			//pos = AssestArchive::HEADER_SIZE = 0x14
			fileTable.resize(files.size(), 0x00000000);
			writeFile.write((char*)&fileTable[0], files.size() * sizeof(uint32));
			uint32 s = AssetArchive::HEADER_SIZE + (files.size() * sizeof(uint32));
			paddingSize = (s % chunkSize != 0 ? (chunkSize - (s % chunkSize)) : 0);
			n = 0x00;
			for (uint32 j = 0; j < paddingSize; j++){writeFile.write((char*)&n, sizeof(uint8));
			}

			offset = AssetArchive::HEADER_SIZE + (files.size() * sizeof(uint32)) + paddingSize;

			if ((offset % chunkSize) != 0)
			{
				//Something is wrong with allignment (not a multiple of "chunkSize")
				success = false;
			}
		}
		if (success)
		{
			std::vector<uint8> fileData;

			for (uint32 index = 0; index < files.size(); index++)
			{
				if (files[index].empty())
				{
					fileData = AssetArchive::GetFileData(archiveData, fileInfo, index, false);
				} else fileData = (isEncoded ? IndexEncoder::EncodeRLE3(files[index]) : files[index]); //Add other encoder versions

				uint32 paddingSize = (fileData.size() % chunkSize != 0 ? (chunkSize - (fileData.size() % chunkSize)) : 0);

				fileData.resize(fileData.size() + paddingSize, 0x00);
				writeFile.write((char*)&fileData[0], fileData.size() * sizeof(uint8));

				//Maybe change file size depending on if the file data is from a new file
				fileTable[index] = ((offset / chunkSize) << fileOffsetShift) | (uint32)std::ceil((float)(fileData.size() - 0x07FE) / (float)fileScale);
				offset += fileData.size();

				if ((offset % chunkSize) != 0)
				{
					//Something is wrong with allignment (not a multiple of "chunkSize")
					success = false;
				}
			}
		}
		if (success)
		{
			writeFile.seekp(AssetArchive::HEADER_SIZE);
			writeFile.write((char*)&fileTable[0], fileTable.size() * sizeof(uint32));
			writeFile.close();
		}

		if (writeFile.is_open()){writeFile.close();
		}
		if (!success)
		{
			//Delete file...?
		}

		return success;
	}
};

#endif
