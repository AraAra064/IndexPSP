#include "ConsoleGraphics.hpp"

bool ReplaceFile(std::vector<uint8> &newFileData, std::string filePath, std::vector<uint8> origData, bool isImage, ColourReductionSettings *crs = nullptr, uint16 scale = 1, bool fastMessage = false)
{
//	chdir(".NewFiles");
	filePath = ".NewFiles\\"+filePath;
	bool fileExists = doesFileExist(filePath);
	
	if (fileExists)
	{
		//Accepts a 24-bit BMP or a PNG
		if (isImage) //fileType == FileType::SHTXPS)
		{
			cg::Image img;
			LoadImage(img, filePath);
			
			cg::Image origImage = !origData.empty() ? IndexSHTXPS::LoadImage(origData) : cg::Image();
			origImage.scale(scale);
			float f = (float)origImage.getWidth()/(float)img.getWidth();
			cg::InterpolationMethod im = (f < 1.f ? cg::InterpolationMethod::AreaAveraging : cg::InterpolationMethod::NearestNeighbor);
			img.resize(origImage.getWidth(), origImage.getHeight(), im);
			
			uint16 colourTableSize;
			float percentileStep;
			uint8 ditherMode;
			
			if (crs == nullptr)
			{
				ColourReductionSettings c;
				colourTableSize = c.colourTableSize;
				percentileStep = c.percentileStep;
				ditherMode = c.ditherMode;
			} else {
				colourTableSize = crs->colourTableSize;
				percentileStep = crs->percentileStep;
				ditherMode = crs->ditherMode;
			}
			
			ReduceImageColours(img, img, colourTableSize, 0, percentileStep, ditherMode, 1);//(std::max(img.getWidth(), img.getHeight()) <= 256) ? 2 : 1); //Colour reduction settings?
			newFileData = IndexSHTXPS::CreateFile(img, 256, origData.empty() ? nullptr : &origData[0]);
			
			//pipe.send("[LOG:REPLACE_FILE] Loaded image file \""+filePath+"\".");
		} else {
			//Any other file
			newFileData = LoadFile(filePath);
			
			//pipe.send("[LOG:REPLACE_FILE] Loaded binary file \""+filePath+"\".");
		}
	} else {
		//pipe.send("[ERROR:REPLACE_FILE] \""+filePath+"\" does not exist.");
	}
	
	return fileExists;
}

void ListHandler(std::vector<uint8> *archiveData, std::vector<AssetArchive::FileInfo> *fileInfo, bool isEncoded, std::deque<std::vector<uint8>> *newFiles, std::vector<std::string> *listCommands, std::vector<std::string> list, std::string dir)
{
	std::deque<std::vector<uint8>> &nF = *newFiles;
	std::string path = dir;
	
	for (std::string line : list)
	{
		auto l = StringParser::Split(line, ':');
		
		uint32 fileIndex;
		std::string fileName, info;
		
		switch (line[0])
		{
			case '!': //Directory
				path = StringParser::Strip(line, '!');
			case '#': //Comment
			case '=': //List Version (not used)
			case '*': //List type
				continue;
			
			case '+': //Add file
				line = StringParser::Strip(line, '+');
				nF.push_back(std::vector<uint8>());
				fileIndex = nF.size()-1;
				break;
			
			case '-': //Remove file
				nF.pop_back();
				continue;
			
			default:
				if (l.size() < 2)
				{
					//pipe.send("[ERROR:LISTCMD] Invalid command, \""+line+"\". Not enough parameters ("+std::to_string(l.size())+")");
					continue;
				}
				
				fileIndex = strtoul(l[0].c_str(), nullptr, 0);
				fileName = l[1];
				break;
		}
		
		uint16 scale = 1;
		if (l.size() == 3){scale = strtoul(l[2].c_str(), nullptr, 0);
		}
		
		ColourReductionSettings crs;
		if (l.size() == 4)
		{
			std::string crsData = l[3];
			crsData = StringParser::Strip(crsData, '[');
			crsData = StringParser::Strip(crsData, ']');
			
			auto crsList = StringParser::Split(crsData, ',');
			for (auto &s : crsList){s = StringParser::Strip(s);
			}
			
			if (crsList.size() >= 3)
			{
				crs.colourTableSize = strtoul(crsList[0].c_str(), nullptr, 0);
				crs.percentileStep = strtof(crsList[1].c_str(), nullptr);
				crs.ditherMode = strtoul(crsList[2].c_str(), nullptr, 0);
			}// else pipe.send("[ERROR:LOADFROMLIST] Not enough ColourReduction parameters, \""+l[3]+"\". Using default.");
		}
		
		auto fileData = AssetArchive::GetFileData(*archiveData, *fileInfo, fileIndex, isEncoded);
		std::string s = StringParser::ToUpper(fileName);
		bool isImage = (s.find(".PNG") != std::string::npos || s.find(".BMP") != std::string::npos);
		bool success = ReplaceFile(nF[fileIndex], path+'\\'+fileName, fileData, isImage, &crs, scale);
		
		if (!success){//pipe.send("[ERROR:LOADFROMLIST] Error replacing file \'"+std::to_string(fileIndex)+"\'.");
		} else listCommands->at(fileIndex) = ArchiveEditor::ReplaceCommand(fileIndex, fileName, scale);
	}
	
	return;
}

void LoadList(std::vector<uint8> &archiveData, std::vector<AssetArchive::FileInfo> &fileInfo, bool isEncoded, std::deque<std::vector<uint8>> &newFiles, std::vector<std::string> &listCommands, std::string filePath, uint32 reqThreads = 4)
{
	std::vector<std::string> list;
	std::ifstream readFile(filePath.c_str());
	if (readFile.is_open())
	{
		std::string str;
		while (std::getline(readFile, str)){list.push_back(str);
		}
		readFile.close();
		
		uint32 numThreads = reqThreads;
		std::string dir = ".";
		
		std::string line = list[0];
		if (line[0] == '!' && line.size() > 1)
		{
			dir = std::string(&line.c_str()[1]);
		}
		line = list[1];
		if (line[0] == '=' && line.size() > 1)
		{
			uint32 numFiles = strtoul(line.c_str() + 1, nullptr, 0);
			newFiles.resize(numFiles);
		}
		
		ListHandler(&archiveData, &fileInfo, isEncoded, &newFiles, &listCommands, list, dir);
	}
	
	return;
}

void CreateList(std::vector<std::string> &list, std::string fileName, std::string directory = ".")
{
	std::ofstream writeFile(fileName.c_str());
	if (writeFile.is_open())
	{
		writeFile << "!" << directory << "\n";
		writeFile << ArchiveEditor::ResizeCommand(list.size());
		
		uint32 fileIndex = 0;
		for (std::string &s : list)
		{
			if (s.empty()){continue;
			}
			
			writeFile << s;
			fileIndex++;
		}
		
		writeFile.close();
	}
	
	return;
}
