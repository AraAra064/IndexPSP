#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#ifndef __INDEX_SSAD
#define __INDEX_SSAD

namespace IndexSSAD
{
	template <class type>
	struct UIProperty
	{
		std::string name;
		uint32_t size;
		std::vector<type> data;

		//aniCounter, duration
		//repeat

		UIProperty()
		{
			name = "NULL";
			size = 0;
		}
		UIProperty(std::string name, uint32_t size, std::vector<type> data)
		{
			this->name = name;
			this->size = size;
			this->data = data;
		}
	};

	struct UIElement
	{
		uint32_t part;
		std::string name;
		//int32_t posX, posY;
		UIProperty<uint32_t> posX;
		UIProperty<uint32_t> posY;
		UIProperty<uint32_t> scaX;
		UIProperty<uint32_t> scaY;
		//int32_t scaX, scaY;
		bool flpX, flpY;
		uint32_t imgX, imgY, imgW, imgH;
		uint32_t myID;
		uint32_t paID;
		UIProperty<uint32_t> area;
		uint32_t chID;
		uint32_t pcID;
		uint32_t prio;
		uint32_t orgX, orgY;
		UIProperty<uint32_t> tran;
		UIProperty<uint32_t> hide;
		UIProperty<uint8_t> udat;
		uint32_t tbdt, sucd;
		UIProperty<uint32_t> pcol;

		uint32_t aniCounter;

		UIElement()
		{
			part = 0;
			name = "...";
			flpX = false;
			flpY = false;
			myID = 0;
			paID = 0xFFFFFFFF;
			chID = 0;
			pcID = 0;
			prio = 0;
			orgX = 0;
			orgY = 0;
			imgX = 0;
			imgY = 0;
			imgW = 0;
			imgH = 0;
			tbdt = 0;
			sucd = 0;

			aniCounter = 0;
		}
	};

	//When subSSA is 0, function will get all elements for 1st sub SSA in SSAD file
	//If subSSA > subSSAs within the file then it will return the last file
	std::deque<IndexSSAD::UIElement> GetElements(std::vector<uint8>& fileData, uint32_t reqSubSSA = 0, bool clear = true)
	{
		uint32_t subSSA = 0;
		uint8_t header[0x20];

		uint32_t index = 0x20;
		std::deque<UIElement> elements;
		while (index < fileData.size())
		{
			UIElement e;
			static uint32_t eName = 0;
			uint32_t eSize;
			std::vector<uint32_t> eData;
			//Read all UIElement Attributes (first should be PART)
			eName = *(uint32_t*)&fileData[index];
			eSize = *(uint32_t*)&fileData[index + 4];
			index += 8;
			//Get Data
			uint32_t eD, c = 0;
			const uint32_t RESET_COUNTER = 0x00002710;

			const uint32_t SSAD = 0x44415353;
			const uint32_t PARTATTRIB = 0x54524150;
			const uint32_t NAMEATTRIB = 0x454D414E;
			const uint32_t AREAATTRIB = 0x41455241;
			const uint32_t ORGXATTRIB = 0x5847524F;
			const uint32_t ORGYATTRIB = 0x5947524F;
			const uint32_t TBDTATTRIB = 0x54424454;
			const uint32_t MYIDATTRIB = 0x4449594D;
			const uint32_t PAIDATTRIB = 0x44494150;
			const uint32_t CHIDATTRIB = 0x44494843;
			const uint32_t PCIDATTRIB = 0x44494350;
			const uint32_t SUCDATTRIB = 0x44435553;
			const uint32_t POSXATTRIB = 0x58534F50;
			const uint32_t POSYATTRIB = 0x59534F50;
			const uint32_t SCAXATTRIB = 0x58414353;
			const uint32_t SCAYATTRIB = 0x59414353;
			const uint32_t TRANATTRIB = 0x4E415254;
			const uint32_t HIDEATTRIB = 0x45444948;
			const uint32_t FLPHATTRIB = 0x48504C46;
			const uint32_t FLPVATTRIB = 0x56504C46;
			const uint32_t PCOLATTRIB = 0x4C4F4350;
			//"1P??MAIN"???
			//UDAT, UKEY, KEYT, KEYS, KEYN
			//ANGL, PRIO, PALT, VERT
			//IMGX, IMGY, IMGW, IMGH
			//ORFX, ORFY
			bool good = true;

			while (good)
			{
				//Get UIElement data
				do
				{
					eD = *(uint32_t*)&fileData[index];
					eData.push_back(eD);
					c += 4;

					if (index + 4 < fileData.size() && *(uint32_t*)&fileData[index + 4] == RESET_COUNTER)
					{
						eData.push_back(RESET_COUNTER); //??
						index += 4;
						c = 0;
					}
					index += 4;
				} while (c < eSize && index < fileData.size());
				c = 0;

				//Apply data to structure
				switch (eName)
				{
				case PARTATTRIB:
					e.part = eData[0];
					break;

				case NAMEATTRIB:
					e.name = std::string((char*)&eData[0]);
					//std::cout << e.name << std::endl;
					break;

				case AREAATTRIB:
					e.area = UIProperty<uint32_t>("AREA", eData.size(), eData);
					break;

				case POSXATTRIB:
					e.posX = UIProperty<uint32_t>("POSX", eData.size(), eData);
					break;

				case POSYATTRIB:
					e.posY = UIProperty<uint32_t>("POSY", eData.size(), eData);
					break;

				case MYIDATTRIB:
					e.myID = eData[0];
					break;

				case PAIDATTRIB:
					e.paID = eData[0];
					break;

				case CHIDATTRIB:
					e.chID = eData[0];
					if (e.chID != 0)
					{
						std::cout << e.name << " - " << e.chID << std::endl;
					}
					break;

				case PCIDATTRIB: //Might change to UIProperty
					e.pcID = eData[0];
					break;

				case SCAXATTRIB: //off3,+7
					e.scaX = UIProperty<uint32_t>("SCAX", eData.size(), eData);
					break;

				case SCAYATTRIB:
					e.scaY = UIProperty<uint32_t>("SCAY", eData.size(), eData);
					break;

				case TRANATTRIB:
					e.tran = UIProperty<uint32_t>("TRAN", eData.size(), eData);
					break;

				case ORGXATTRIB:
					e.orgX = eData[0];
					break;

				case ORGYATTRIB:
					e.orgY = eData[0];
					break;

				case HIDEATTRIB:
					e.hide = UIProperty<uint32_t>("HIDE", eData.size(), eData);
					break;

				case FLPHATTRIB:
					e.flpX = (eData[2] == 1);
					break;

				case FLPVATTRIB:
					e.flpY = (eData[2] == 1);
					break;

				case TBDTATTRIB:
					e.tbdt = eData[0];
					break;

				case SUCDATTRIB:
					e.sucd = eData[0];
					break;

				case PCOLATTRIB:
					e.pcol = UIProperty<uint32_t>("PCOL", eData.size(), eData);
					break;

				default:
					break;
				}
				eData.clear();

				if (index < fileData.size())
				{
					eName = *(uint32_t*)&fileData[index];
					eSize = *(uint32_t*)&fileData[index + 4];
				}
				index += 8;
				good = (eName != SSAD && eName != PARTATTRIB && index < fileData.size());
			}

			index -= 8;

			elements.push_back(e);

			if (eName == SSAD)
			{
				//Skip header
				index += 0x20;

				//std::cout << subSSA << ' ' << reqSubSSA << std::endl;
				if (subSSA != reqSubSSA)
				{
					if (clear) {
						elements.clear();
					}
					subSSA++;
				}
				else break;
			}
		}

		return elements;
	}

	//Returns all subSSAs NAME attributes that contain the a string (matchStr)
	std::vector<std::string> GetSubSSANames(std::deque<UIElement> elements, std::string matchStr = "")
	{
		std::vector<std::string> names;

		for (auto& e : elements)
		{
			bool nameMatch = (matchStr.empty() || e.name.find(matchStr) != std::string::npos);

			//all sub ssas have a part value of 1 (i think)
			if (e.part == 0x01 && nameMatch)
			{
				names.push_back(e.name);
			}
		}

		return names;
	}

	//Returns all elements contained within a subSSA
	//subSSA must equal to the NAME attribute exactly
	std::deque<UIElement> GetElementsByName(std::deque<UIElement>& elements, std::string subSSAName)
	{
		std::deque<UIElement> subSSA;

		bool isSubSSA = false;
		bool copyStart = false;

		for (size_t i = 0; i < elements.size(); i++)
		{
			auto& e = elements[i];
			isSubSSA = e.part == 0x01;
			if (!isSubSSA && !copyStart)
			{
				continue;
			}

			if (copyStart && isSubSSA)
			{
				break;
			}
			copyStart = !copyStart ? (isSubSSA && e.name == subSSAName) : copyStart;


			if (copyStart)
			{
				subSSA.push_back(e);
			}
			else continue;
		}

		return subSSA;
	}

	//Returns the result of merging elements from secondary to primary by NAME attribute
	std::deque<UIElement> MergeElements(std::deque<UIElement>& primary, std::deque<UIElement>& secondary)
	{
		std::deque<UIElement> elements = primary;
		std::map<std::string, uint32_t> eleMap;

		for (size_t i = 0; i < primary.size(); i++)
		{
			UIElement& e = primary[i];

			eleMap[e.name] = i;
		}

		for (UIElement& e : secondary)
		{
			if (eleMap.count(e.name) != 0)
			{
				//overwrite primary with secondary
				auto i = eleMap[e.name];
				elements[i] = e;
			}
			else elements.push_back(e);
		}

		return elements;
	}
}

#endif