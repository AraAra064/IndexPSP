#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

int cli(int nArgs, char** arg)
{
	std::vector<std::string> archiveNames = { "GRP", "DAT", "EVT", "ADV", "FARC", "PSPSND", "PSPBGM" };
	std::vector<bool> encodedArchives = { true, true, true, false, false, false };

	//bool b = pipe.initalise("\\\\.\\pipe\\ArchiveEditor", pmsg::PipeMode::SND);
	//if (!b)
	//{
	//	std::cout << "Failed to connect to " << pipe.getPipeName() << ", no output other than prompts." << std::endl;
	//}
	if (nArgs > 1)
	{
		//Create AA
		std::string argString; // = "Available commands:\n\n";
		argString += "ipae.exe AA_IN LISTNAME (AA_OUT)";
		argString += "\nAA_IN = Asset Archive to be edited";
		argString += "\nLISTNAME = List to edit AA from";
		//argString += "\nAA_OUT (not functional/optional) = File to save custom AA as";

		////Extract one
		//argString += "ipae.exe EXTRACTO AA_IN FILE_INDEX (FILE_OUT)";
		//argString += "\nAA_IN = Asset Archive to be extracted\FILE_INDEX = File to extract";
		//argString += "\nFILE_OUT (optional) = File to save as";
		//
		////Extract all
		//argString += "ipae.exe EXTRACTA AA_IN (FILE_OUT)";
		//argString += "\nAA_IN = Asset Archive to be extracted";
		//argString += "\nFILE_OUT (optional) = File to save as";

		if (nArgs < 3)
		{
			std::cout << "Not enough arguments." << std::endl;
			std::cout << argString << std::endl;
			std::cout << "Available AAs for AA_IN:" << std::endl;
			for (auto s : archiveNames)
			{
				std::cout << s << std::endl;
			}

			return 1;
		}

		std::string aaIn = arg[1], listName = arg[2];
		listName = "Lists\\" + listName;
		std::transform(aaIn.begin(), aaIn.end(), aaIn.begin(), toupper);
		std::string endName = aaIn;
		if (aaIn.find('_') != std::string::npos){endName.erase(aaIn.find('_'));
		} else endName = aaIn;
		endName = "CustomArchives\\" + endName + ".BIN";

		//check if correct
		uint32 i = 0;
		for (auto s : archiveNames)
		{
			if (s == aaIn) {
				break;
			}
			i++;
		}
		auto f = LoadFile("OriginalArchives\\" + aaIn + ".BIN");
		if (f.empty())
		{
			std::cout << "e" << std::endl;
			return 1;
		}

		auto fi = AssetArchive::GetFileInfo(f);
		std::deque<std::vector<uint8>> nF(fi.size());
		std::vector<std::string> lC(fi.size());
		LoadList(f, fi, encodedArchives[i], nF, lC, listName, 1);
		AssetArchive::SaveArchive(f, fi, nF, endName, 3 | (encodedArchives[i] ? AssetArchive::IsEncoded : 0));
		std::cout << "Saved" << std::endl;

		return 0;
	}
	//else pipe.send("Starting GUI.");
}