#include <vector>
#include <fstream>

#include "KadaTools.hpp"
#include "Encoder.hpp"

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
			uint32_t fi;// = *(uint32_t*)&data[pos];
			memcpy(&fi, &data[pos], sizeof(uint32_t));

			//((fi >> shiftVal) & ~maskVal)
			FileInfo x;
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
			
			if ((pos + size) < data.size()) //ADV silly
			{
				fileData.resize(size);
				memcpy(&fileData[0], &data[pos], size);
				fileData = decode ? KadaTools::DecodeRLE2(fileData) : fileData;
			} else fileData = std::vector<uint8_t>(1, 0xFF);
		}
		
		return fileData;
	}
	
	//Used for SaveArchive function
	const uint32_t IsEncoded = 0x10;

	//Encoder
	//LS 4 bits = encoder version (encoderVer = encoder & 0x0F)
	//next bit = is encoded (is encoded, bool = (encoder & 0x10)
	//SaveArchive(archiveData, fileInfo, newFiles, "GRP.bin", 4 | AssetArchive::IsEncoded)
	bool SaveArchive(std::vector<uint8_t> &archiveData, std::vector<FileInfo> &fileInfo, std::deque<std::vector<uint8_t>> &files, std::string fileName, uint32_t encoder)
	{
		bool success = true;
		uint32_t encoderVer = encoder & 0x0F;
		bool isEncoded = ((encoder & 0x10) == 0x10);

		std::ofstream writeFile(fileName.c_str(), std::ios::binary);
		success = writeFile.is_open();

		uint32_t chunkSize;
		uint32_t fileScale;
		uint32_t fileOffsetShift;
		uint32_t maskVal;
		std::vector<uint32_t> fileTable;
		uint32_t paddingSize;
		uint32_t offset;

		if (success)
		{
			uint32_t n = files.size();
			writeFile.write((char*)&n, sizeof(uint32_t));
			chunkSize = AssetArchive::GetFileChunkSize(archiveData);
			writeFile.write((char*)&chunkSize, sizeof(uint32_t));
			fileScale = AssetArchive::GetFileSizeScale(archiveData);
			writeFile.write((char*)&fileScale, sizeof(uint32_t));
			fileOffsetShift = AssetArchive::GetFileOffsetShift(archiveData);
			writeFile.write((char*)&fileOffsetShift, sizeof(uint32_t));
			maskVal = AssetArchive::GetMaskVal(archiveData);
			writeFile.write((char*)&maskVal, sizeof(uint32_t));

			//pos = AssestArchive::HEADER_SIZE = 0x14
			fileTable.resize(files.size(), 0x00000000);
			writeFile.write((char*)&fileTable[0], files.size() * sizeof(uint32_t));
			uint32_t s = AssetArchive::HEADER_SIZE + (files.size() * sizeof(uint32_t));
			paddingSize = (s % chunkSize != 0 ? (chunkSize - (s % chunkSize)) : 0);
			n = 0x00;
			for (uint32_t j = 0; j < paddingSize; j++){writeFile.write((char*)&n, sizeof(uint8_t));
			}

			offset = AssetArchive::HEADER_SIZE + (files.size() * sizeof(uint32_t)) + paddingSize;

			if ((offset % chunkSize) != 0)
			{
				//Something is wrong with allignment (not a multiple of "chunkSize")
				success = false;
			}
		}
		if (success)
		{
			std::vector<uint8_t> fileData;

			for (uint32_t index = 0; index < files.size(); index++)
			{
				if (files[index].empty())
				{
					fileData = AssetArchive::GetFileData(archiveData, fileInfo, index, false);
				} else fileData = (isEncoded ? IndexEncoder::EncodeRLE3(files[index]) : files[index]); //Add other encoder versions

				uint32_t paddingSize = (fileData.size() % chunkSize != 0 ? (chunkSize - (fileData.size() % chunkSize)) : 0);

				fileData.resize(fileData.size() + paddingSize, 0x00);
				writeFile.write((char*)&fileData[0], fileData.size() * sizeof(uint8_t));

				//Maybe change file size depending on if the file data is from a new file
				fileTable[index] = ((offset / chunkSize) << fileOffsetShift) | (uint32_t)std::ceil((float)(fileData.size() - 0x07FE) / (float)fileScale);
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
			writeFile.write((char*)&fileTable[0], fileTable.size() * sizeof(uint32_t));
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
