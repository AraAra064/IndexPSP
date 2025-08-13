#include "ConsoleGraphics.hpp"
#include "StateManager.hpp"

class ModifyState : public StateBase
{
	const uint32 backgroundColour = 0x005F5FAF;
	const uint32 rowColour = 0x00AFAFAF;
	const uint8 rowAlpha = 0x3F;
	cg::Image preview;
	cg::Image font;
	cg::Text text;

	std::vector<uint8> archiveData;
	std::vector<AssetArchive::FileInfo> fileInfo;

	std::deque<std::vector<uint8>> newFiles;
	std::vector<std::string> fileList;
	std::vector<uint8> currentFile;
	uint32 fileIndex;

	GlobalData* gd;
	Keyboard* keyboard;

	protected:
		void handleInputs(void);

	public:
		ModifyState()
		{
			gd = nullptr;
			keyboard = nullptr;

			font.loadImage("Fonts\\DefaultFontx2.bmp");
			font.setColourToAlpha(cg::BGR(255, 0, 0));
			font.scale(0.5f, cg::InterpolationMethod::AreaAveraging);
			text.setFont(&font, 8, 13);
		}

		void onStartup(void)
		{
			gd = (GlobalData*)globalStateData;
			keyboard = &gd->keyboard;
			fileIndex = 0;
			keyboard->setMaxDownTime(75);

			archiveData = LoadFile(gd->archiveFileName);
			if (archiveData.empty())
			{
				*stateManagerState = -1;
				return;
			}

			fileInfo = AssetArchive::GetFileInfo(archiveData);
			newFiles.clear();
			newFiles.resize(AssetArchive::GetNumberOfFiles(archiveData));
			fileList.clear();
			fileList.resize(AssetArchive::GetNumberOfFiles(archiveData));

			return;
		}
		void update(float dT)
		{
			//Reload
			if (currentFile.empty())
			{
				//If no new file has been loaded, load a file like normal
				if (newFiles[fileIndex].empty() && fileIndex < AssetArchive::GetNumberOfFiles(archiveData))
				{
					currentFile = AssetArchive::GetFileData(archiveData, fileInfo, fileIndex, gd->archiveEncoded);
				} else currentFile = newFiles[fileIndex];
				
				//Load preview image
				uint32 fileType = *(uint32*)&currentFile[0];
				std::string fileTypeStr = "unknown";
				uint32 s = 4;
				switch (fileType)
				{
					case C_SHTXPS: //SHTX(PS)
						fileTypeStr = "SHTXPS";
						preview = IndexSHTXPS::LoadImage(currentFile);
						break;
					
					case C_PTC:
						s = 3;
					case C_PSC6:
					case C_PSM0:
					case C_SSAD:
					case C_CRAF:
					case C_RIFF:
						fileTypeStr = std::string((char*)&fileType, s);
					default: //UNKNOWN BINARY
						preview = cg::Image("Images\\"+fileTypeStr+".bmp");
						break;
				}

				if (fileType == C_CRAF)
				{
					bool success = false;
					auto fd = IndexCRAF::GetFileData(currentFile, 0);
					cg::Image subImage(256, 256, 0, 0);
					if (fd.data() != nullptr && *(uint32*)fd.data() == C_SHTXPS){subImage = IndexSHTXPS::LoadImage(fd, &success);
					}
				
					if (success)
					{
						subImage.resize(preview.getWidth() / 2, 0, cg::InterpolationMethod::AreaAveraging);
						uint32 dstX = preview.getWidth() / 2;
						uint32 dstY = preview.getHeight() / 2;
						uint32 w = subImage.getWidth();
						uint32 h = subImage.getHeight();
						preview.blendImage(subImage, dstX, dstY, 0, 0, w, h);
					}
				}
			}
			
			handleInputs();

			keyboard->update(dT);
			
			return;
		}
		void render(void)
		{
			graphics->drawRect(0, 0, graphics->getWidth(), graphics->getHeight(), backgroundColour);

			uint32 w = preview.getWidth(), h = preview.getHeight();
			float f = 256.f / (float)std::max(w, h);
			preview.setPos((graphics->getWidth() / 2) - (w * f / 2), (graphics->getHeight() / 2) - (h * f / 2));
			drawImage(*graphics, preview, f, cg::DrawType::Resize);

			uint32 rows = 3, rowHeight = rows * text.getCharHeight(), rowY = graphics->getHeight() - rowHeight;
			graphics->drawRectA(0, 0, graphics->getWidth(), rowHeight, rowColour, rowAlpha);
			//Left
			text.setPos(0, 0);
			text.setText(gd->archiveName + "\nFileIndex=" + std::to_string(fileIndex) + " [" + IntToHexStr(fileIndex) + "] " + "\nHeader=" + GetFileHeader(currentFile));
			graphics->draw(text);
			//Mid
			//...
			//Right
			text.setText("NumFiles=" + std::to_string(newFiles.size()) + '\n' + GetFileInfo(currentFile));
			text.setPos(graphics->getWidth() - text.getWidth(), 0);
			graphics->draw(text);

			rows = 3;
			graphics->drawRectA(0, rowY, graphics->getWidth(), rowHeight, rowColour, rowAlpha);

			text.setText("A=Add new file\nR=Replace current file \nX=Remove last file");
			text.setPos(0, rowY);
			graphics->draw(text);

			text.setText("J=Jump to file index\nK=Create list\nL=Load list");
			text.setPos((graphics->getWidth() / 2) - (text.getWidth() / 2), rowY);
			graphics->draw(text);

			text.setText("C=Create new AA\nE=Extract options");
			text.setPos(graphics->getWidth() - text.getWidth(), rowY);
			graphics->draw(text);

			return;
		}
};

void ModifyState::handleInputs(void)
{
	if (keyboard->isKeyDown(VK_ESCAPE))
	{
		*stateManagerState = 0xFF;
	}

	if (keyboard->isKeyDown(VK_RIGHT))
	{
		++fileIndex %= newFiles.size();
		currentFile.clear();
	}
	else if (keyboard->isKeyDown(VK_LEFT))
	{
		fileIndex = std::min<uint32>(--fileIndex, newFiles.size() - 1);
		currentFile.clear();
	}

	if (keyboard->isKeyDown('J'))
	{
		clearInput();
		std::cout << "Enter the index of the file to jump to:" << std::flush;
		std::string indexStr;
		std::getline(std::cin, indexStr);
		clearInput();
		fileIndex = strtoul(indexStr.c_str(), nullptr, 0) % newFiles.size();
		//pipe.send("[LOG:JUMP_TO_INDEX] Jumping to index \'" + std::to_string(fileIndex) + "\'");
		currentFile.clear();
	}

	if (keyboard->isKeyDown('C'))
	{
		std::string fileName = "CustomArchives\\";
		//replace with string.find char and cut string at '_'
		std::string ender = gd->archiveName;
		ender.erase(ender.find('_'));
		ender += ".BIN";
		fileName += ender;

		bool retVal = AssetArchive::SaveArchive(archiveData, fileInfo, newFiles, fileName, 0x03 | AssetArchive::IsEncoded);

		//if (retVal)
		//{
		//	pipe.send("[CREATE_ASSET_ARCHIVE]: Created new AA (" + fileName + ")");
		//}

		currentFile.clear(); //Reload
	}

	if (keyboard->isKeyDown('K'))
	{
		std::string dir = "Lists";
		clearInput();
		std::cout << "In current folder:\"" << dir << "\"" << std::endl;
		std::cout << "Enter a name of list to create:" << std::flush;
		std::string fileName;
		std::getline(std::cin, fileName);
		CreateList(fileList, dir + '\\' + fileName);
		clearInput();
	}
	if (keyboard->isKeyDown('L'))
	{
		std::string dir = "Lists";
		clearInput();
		std::cout << "In current folder:\"" << dir << "\"" << std::endl;
		std::cout << "Enter a name of list to load:" << std::flush;
		std::string fileName;
		std::getline(std::cin, fileName);
		LoadList(archiveData, fileInfo, gd->archiveEncoded, newFiles, fileList, dir + '\\' + fileName);
		clearInput();
		currentFile.clear();
	}

	if (keyboard->isKeyDown('R'))
	{
		std::string dir = ".NewFiles";
		clearInput();
		std::cout << "In current folder:\"" << dir << "\"" << std::endl;
		std::cout << "Enter a file to replace:" << std::flush;
		std::string fileName;
		std::getline(std::cin, fileName);
		std::string filePath = fileName, s = StringParser::ToUpper(fileName);
		bool isImage = (s.find(".PNG") != std::string::npos || s.find(".BMP") != std::string::npos);
		uint16 scale = 1;

		std::vector<uint8> origData = AssetArchive::GetFileData(archiveData, fileInfo, fileIndex, gd->archiveEncoded);

		if (isImage && doesFileExist(filePath))
		{
			std::cout << "0 = Original PSP Texture Resolution" << std::endl;
			std::cout << "1 = Original PSP Texture Resolution x2" << std::endl;
			std::cout << "2 = Original PSP Texture Resolution x4" << std::endl;
			std::cout << "3 = Original PSP Texture Resolution x8" << std::endl;
			std::cout << "Enter an option:" << std::flush;
			std::string os;
			std::getline(std::cin, os);
			uint32 o = strtoul(os.c_str(), nullptr, 0);

			switch (o)
			{
				default:
				case 0:
					break;

				case 1:
				case 2:
				case 3:
					scale = powf(2.f, o);
					break;
			}
		}
		clearInput();

		bool success = ReplaceFile(newFiles[fileIndex], filePath, origData, isImage, nullptr, scale);
		fileList[fileIndex] = ArchiveEditor::ReplaceCommand(fileIndex, fileName, scale);

		currentFile.clear();
	}

	if (keyboard->isKeyDown('A'))
	{
		std::string dir = ".NewFiles";
		clearInput();
		std::cout << "In current folder:\"" << dir << "\"" << std::endl;
		std::cout << "Enter a file to add:" << std::flush;
		std::string fileName;
		std::getline(std::cin, fileName);
		std::string filePath = fileName, s = StringParser::ToUpper(fileName);
		bool isImage = (s.find(".PNG") != std::string::npos || s.find(".BMP") != std::string::npos);
		uint16 scale = 1;

		if (isImage && doesFileExist(filePath))
		{
			std::cout << "0 = 256x256" << std::endl;
			std::cout << "1 = 512x512" << std::endl;
			std::cout << "2 = 1024x1024" << std::endl;
			std::cout << "3 = 2048x2048" << std::endl;
			std::cout << "Enter an option:" << std::flush;
			std::string os;
			std::getline(std::cin, os);
			uint32 o = strtoul(os.c_str(), nullptr, 0);

			switch (o)
			{
			default:
			case 0:
				break;

			case 1:
			case 2:
			case 3:
				scale = powf(2.f, o);
				break;
			}
		}

		clearInput();

		newFiles.push_back(std::vector<uint8>());
		bool success = ReplaceFile(newFiles.back(), filePath, std::vector<uint8>(), isImage, nullptr, scale);
		if (!success){newFiles.pop_back();
		}
		fileList.push_back(ArchiveEditor::ReplaceCommand(newFiles.size() - 1, fileName, scale));

		currentFile.clear();
	}
	if (keyboard->isKeyDown('X'))
	{
		newFiles.pop_back();
		fileIndex %= newFiles.size();
		currentFile.clear();
		fileList.pop_back();
	}

	if (keyboard->isKeyDown('E'))
	{
		clearInput();
		std::cout << "0 = Extract original files from AA" << std::endl;
		std::cout << "1 = Extract new files from AA" << std::endl;
		std::cout << "Enter an option:" << std::flush;
		std::string option;
		std::getline(std::cin, option);

		uint32 val = strtoul(option.c_str(), nullptr, 0);
		if (val < 2)
		{
			std::cout << "Enter name of folder to extract to:" << std::flush;
			std::string folderName;
			std::getline(std::cin, folderName);
			folderName = ".SavedFiles\\" + folderName;

			//Just going to assume that the folder has been made
			_mkdir(folderName.c_str());

			std::vector<uint8> fd;

			for (uint32 i = 0; i < newFiles.size(); i++)
			{
				if (val == 0){fd = AssetArchive::GetFileData(archiveData, fileInfo, i, gd->archiveEncoded);
				} else fd = newFiles[i];

				if (fd.empty()){continue;
				}

				std::string header = std::string((char*)&fd[0]);
				if (header.size() > 4) {
					header.resize(4);
				}
				for (char c : header)
				{
					if (!isalnum(c))
					{
						header = "UNKNOWN";
						break;
					}
				}
				if (header == "SHTX"){header += "PS";
				}

				std::string pre;
				switch (gd->archiveIndex)
				{
					default:
						pre = "File";
						break;

					case 6:
						pre = "Sound";
						header = "bin";
						break;

					case 7:
						pre = "BGM";
						header = "AT3";
						break;
				}
				std::string fileName = pre + std::to_string(i);
				if (val != 0){fileName = "New" + fileName;
				}
				std::string filePath = folderName + '\\' + fileName + '.' + header;

				std::ofstream writeFile;
				writeFile.open(filePath.c_str(), std::ios::binary);
				if (writeFile.is_open())
				{
					writeFile.write((char*)&fd[0], fd.size() * sizeof(char));
					writeFile.close();
				}

				//If image, also save as PNG
				if (header == "SHTXPS")
				{
					cg::Image image = IndexSHTXPS::LoadImage(fd);
					std::vector<uint8> p;
					//BGR-A -> RGBA
					for (uint32 i = 0; i < image.getWidth() * image.getHeight(); ++i)
					{
						p.push_back(cg::GetR(image[i]->first));
						p.push_back(cg::GetG(image[i]->first));
						p.push_back(cg::GetB(image[i]->first));
						p.push_back(image[i]->second);
					}

					filePath = folderName + '\\' + fileName + ".png";
					lodepng::encode(filePath, p.data(), image.getWidth(), image.getHeight());
				}

				//If encoded version available, save it too
				if (val == 0 && gd->archiveEncoded)
				{
					fd = AssetArchive::GetFileData(archiveData, fileInfo, i, false);
					fileName += "Encoded";
					filePath = folderName + '\\' + fileName + '.' + header;

					writeFile.open(filePath.c_str(), std::ios::binary);
					if (writeFile.is_open())
					{
						writeFile.write((char*)&fd[0], fd.size() * sizeof(char));
						writeFile.close();
					}
				}
			}
		}
		//else pipe.send("[ERROR:EXTRACTOP] Invalid option entered. [" + std::to_string(val) + "]");
	}

	return;
}