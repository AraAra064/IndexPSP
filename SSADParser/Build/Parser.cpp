#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <map>
#include <vector>

#include "ConsoleGraphics.hpp"
#include "ConsoleUtilities.h"
#include "StringParser.hpp"
#include "IndexPSP/CRAF.hpp"
#include "IndexPSP/KadaTools.hpp"
#include "IndexPSP/SHTXPS.hpp"

std::string IntToHexStr(uint32 n)
{
	std::string str;
	std::stringstream ss;
	ss<<std::hex<<n;
	str = ss.str();
	std::transform(str.begin(), str.end(), str.begin(), [](char c)->char{return toupper(c);});
	std::string s = std::string(str);
	return "0x"+(s.size() == 1 ? std::string("0") : std::string())+s;
}

std::string PrintHexArr(std::vector<uint32> &a)
{
	std::string str;
	for (uint32 n : a){str += IntToHexStr(n)+' ';
	}
	return str;
}

template <class type>
struct UIProperty
{
	std::string name;
	uint32 size;
	std::vector<type> data;

	UIProperty()
	{
		name = "NULL";
		size = 0;
	}
	UIProperty(std::string name, uint32 size, std::vector<type> data)
	{
		this->name = name;
		this->size = size;
		this->data = data;
	}
};

struct UIElement
{
	uint32 part;
	std::string name;
	int32 posX, posY;
	int32 scaX, scaY;
	bool flpX, flpY;
	uint32 texX, texY, texW, texH;
	uint32 myID;
	uint32 paID;
	UIProperty<uint32> area;
	//uint32 chID;
	uint32 pcID;
	//uint32 prio;
	UIProperty<uint8> udat;

	UIElement()
	{
		name = "....";
		posX = 0;
		posY = 0;
		scaX = 1;
		scaY = 1;
		flpX = false;
		flpY = false;
		myID = 0;
		paID = 0xFFFFFFFF;
		//chID = 0;
		pcID = 0;
		//prio = 
	}
};

//scuffed
std::vector<uint8> GetFileData(std::string fileName)
{
	std::vector<uint8> data;
	std::ifstream r(fileName.c_str(), std::ios::binary);
	
	if (r.is_open())
	{
		while (!r.eof())
		{
			uint8 c;
			r.read((char*)&c, sizeof(char));
			data.push_back(c);
		}

		r.close();
	}

	return data;
}

int main()
{
	cg::ConsoleGraphics graphics(480, 272, true, 1);
	uint8 header[0x20];
	//std::map<std::string, std::vector<uint32>> ssadData;
	
	auto fileData = GetFileData("File1026.SSAD");
	//std::ifstream readFile("ExactFile1008SSAD.bin", std::ios::binary);
	//readFile.read((char*)&header[0], 0x20 * sizeof(uint8));
	
	uint32 index = 0x20;
	std::deque<UIElement> elements;
	while (index < fileData.size())
	{
		UIElement e;
		static uint32 eName = 0;
		uint32 eSize;
		std::vector<uint32> eData;
		//Read all UIElement Attributes (first should be PART)
		eName = *(uint32*)&fileData[index];
		eSize = *(uint32*)&fileData[index+4];
		index += 8;
		//readFile.read((char*)&eName, sizeof(uint32));
		//readFile.read((char*)&eSize, sizeof(uint32));
		//Get Data
		uint32 eD, c = 0;
		const uint32 RESET_COUNTER = 0x00002710;

		const uint32 SSAD = 0x44415353;
		const uint32 PARTATTRIB = 0x54524150;
		const uint32 NAMEATTRIB = 0x454D414E;
		const uint32 POSXATTRIB = 0x58534F50;
		const uint32 POSYATTRIB = 0x59534F50;
		const uint32 AREAATTRIB = 0x41455241;
		const uint32 MYIDATTRIB = 0x4449594D;
		const uint32 PAIDATTRIB = 0x44494150;
		const uint32 PCIDATTRIB = 0x44494350;
		bool good = true;

		while (good)
		{
			std::cout << "INDEX=" << IntToHexStr(index) << '\n' << std::endl;
			std::cout << "eName=" << IntToHexStr(eName) << '('+std::string((char*)&eName, 4)+')' << std::endl;
			std::cout << "eSize=" << IntToHexStr(eSize) << '\n' << std::endl;
			//if (eSize < 4){eSize = 4;
			//}
			do
			{
				eD = *(uint32*)&fileData[index];
				//readFile.read((char*)&eD, sizeof(uint32));
				eData.push_back(eD);
				c += 4;

				if (*(uint32*)&fileData[index+4] == RESET_COUNTER)
				{
					std::cout << "RESET!" << std::endl;
					eData.push_back(RESET_COUNTER);
					index += 4;
					c = 0;
				}
				index += 4;
			} while (c < eSize);
			c = 0;
			std::cout << eData.size() * 4 << std::endl;

			//Apply data to structure
			switch (eName)
			{
				case PARTATTRIB:
					memcpy(&e.part, &eData[0], sizeof(uint32));
					break;

				case NAMEATTRIB:
					e.name = std::string((char*)&eData[0], eData.size() * sizeof(uint32));
					std::cout << e.name << std::endl;
					break;

				case AREAATTRIB:
					e.area = UIProperty<uint32>("AREA", eData.size(), eData);
					break;

				//For now, posX and posY is just position on screen
				case POSXATTRIB:
					e.posX = 0.00101f * ((float)eData[0x10 / sizeof(uint32)] - 130749.83f);
					break;

				case POSYATTRIB:
					e.posY = 0.00101f * ((float)eData[0x10 / sizeof(uint32)] - 130749.83f);
					break;

				case MYIDATTRIB:
					e.myID = eData[0];
					break;

				case PAIDATTRIB:
					e.paID = eData[0];
					break;

				case PCIDATTRIB:
					e.pcID = eData[0];

				default:
					break;
			}
			eData.clear();

			eName = *(uint32*)&fileData[index];
			eSize = *(uint32*)&fileData[index+4];
			index += 8;
			//readFile.read((char*)&eName, sizeof(uint32));
			//readFile.read((char*)&eSize, sizeof(uint32));
			good = (eName != SSAD && eName != PARTATTRIB);
		}

		index -= 8;

		elements.push_back(e);

		//...............
		if (eName == SSAD) //SSAD
		{
			break;
		}
	}

	for (auto& e : elements)
	{
		std::cout << "Name=" << e.name << '\t' << "X=" << e.posX << '\t' << "Y=" << e.posY << std::endl;
		std::cout << "MYID=" << e.myID << '\t' << "PCID=" << e.pcID << '\t' << "PAID=" << IntToHexStr(e.paID) << std::endl;
	}
	std::cout << "There are \"" << elements.size() << "\" elements in first SSAD." << std::endl;
}
