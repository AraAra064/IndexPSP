#include <vector>

#include "Consts.hpp"

#ifndef __TOARU_ENCODER__
#define __TOARU_ENCODER__

namespace IndexEncoder
{

//Direct copy without RLE (Version 1)
std::vector<uint8> EncodeHackyRLE(std::vector<uint8> &data)
{
	std::vector<uint8> retVal;
	uint32 i = 0;
	uint32 maxN = 0x1F, n; //31
	
	while (i < data.size())
	{
		n = std::min<uint32>(data.size()-i, maxN);
		
		retVal.push_back(n);
		for (uint32 j = 0; j < n; ++j){retVal.push_back(data[i+j]);
		}
		
		i += n;
	}
	retVal.push_back(0x00);
	
	return retVal;
}

struct EncodedData
{
	uint8 val;
	uint32 n;
	
	EncodedData()
	{
		val = 0;
		n = 1;
	}
};

//Basic version only includes RLE (Version 2)
//+Encoded version is now smaller than the original
std::vector<uint8> EncodeRLE(std::vector<uint8> &data)
{
	std::vector<EncodedData> basicRLE;
	std::vector<uint8> retVal;
	
	//Encode data into RLE
	for (uint32 i = 0; i < data.size(); ++i)
	{
		EncodedData d;
		d.val = data[i];
		while (data[i] == data[i+1] && i < data.size() && d.n < 0x00FF)
		{
			++d.n;
			++i;
		}
		
		basicRLE.push_back(d);
	}
	
	//Reformat data
	for (uint32 i = 0; i < basicRLE.size(); ++i)
	{
		EncodedData val = basicRLE[i];
		bool validRLE;
		
		if ((validRLE = (val.n > 4))) //REPEAT_FLAG
		{
			uint8 c0 = 0x40, c1;
			c0 |= (0x10 | ((val.n & 0x0F00) >> 8));
			c1 = (val.n & 0x00FF) - 4;
			
			retVal.push_back(c0);
			retVal.push_back(c1);
			retVal.push_back(val.val);
		} else { //Direct copy 
			std::vector<uint8> directData;
			
			if (!validRLE && val.n != 1)
			{
				for (uint32 j = 0; j < val.n; ++j){directData.push_back(val.val);
				}
			} else {
				uint32 n = 0;
				while (val.n == 1 && i < basicRLE.size() && n < 31)
				{
					directData.push_back(val.val);
					++i;	
					++n;
					if (i < basicRLE.size()){val = basicRLE[i];
					}
				}
				i -= 1;
			}
			
			retVal.push_back(directData.size());
			for (auto c : directData){retVal.push_back(c);
			}
		}
	}
	retVal.push_back(0x00);
	
	return retVal;
}

//Version 2.1 (Minor improvements to V2)
//+Can now repeat a character only using one character
std::vector<uint8> EncodeRLE2(std::vector<uint8> &data)
{
	std::vector<EncodedData> basicRLE;
	std::vector<uint8> retVal;
	
	//Encode data into RLE
	for (uint32 i = 0; i < data.size(); i++)
	{
		EncodedData d;
		d.val = data[i];
		while (data[i] == data[i+1] && i < data.size() && d.n < 0x00FF)
		{
			d.n++;
			i++;
		}
		
		basicRLE.push_back(d);
	}
	
	//Reformat data
	for (uint32 i = 0; i < basicRLE.size(); ++i)
	{
		EncodedData val = basicRLE[i];
		bool validRLE;
		
		if ((validRLE = (val.n >= 4))) //REPEAT_FLAG
		{
			bool multi = false;
			uint8 c0 = 0x40, c1;
			
			if (val.n < 0x0F) //Same char version (16 max)
			{
				c0 |= (val.n - 4);
			} else { //Multi char version (4095 max)
				multi = true;
				c0 |= (0x10 | ((val.n & 0x0F00) >> 8));
				c1 = (val.n & 0x00FF) - 4;
			}
			
			retVal.push_back(c0);
			if (multi){retVal.push_back(c1);
			}
			retVal.push_back(val.val);
		} else { //Direct copy 
			std::vector<uint8> buffer;
			
			while (val.n < 4 && buffer.size()+val.n < 0x1F && i < basicRLE.size())
			{
				for (uint32 j = 0; j < val.n; j++){buffer.push_back(val.val);
				}
				i++;	
				if (i < basicRLE.size()){val = basicRLE[i];
				}
			}
			i -= 1;
			
			retVal.push_back(buffer.size());
			for (auto c : buffer){retVal.push_back(c);
			}
		}
	}
	retVal.push_back(0x00);
	
	return retVal;
}


//Version 2.2
//+Added DirectCopyEX (can copy one to one with longer range)
//+Fixed bug which limited the number of times a character could be repeated to 0xFF (255)
//^The limit is now 0x0FFF (4095)
//+Added mode which makes function do a one to one copy using DirectCopyEX
//.Added null character to the end of output of all other versions
std::vector<uint8> EncodeRLE3(std::vector<uint8>& data, bool directMode = false)
{
	std::vector<EncodedData> basicRLE;
	std::vector<uint8> retVal;

	//Encode data into RLE
	for (uint32 i = 0; i < data.size(); i++)
	{
		EncodedData d;
		d.val = data[i];
		while (data[i] == data[i + 1] && i < data.size() && d.n < 0x0FFF)
		{
			d.n++;
			i++;
		}

		basicRLE.push_back(d);
	}

	//Reformat data
	for (uint32 i = 0; i < basicRLE.size(); ++i)
	{
		EncodedData val = basicRLE[i];

		if (val.n >= 4 && !directMode) //Repeat
		{
			bool multi = false;
			uint8 c0 = 0x40, c1;

			if (val.n < 0x0F) //Single char version (16 max)
			{
				c0 |= (val.n - 4);
			} else
			{
				//Multi-char version (4095 max)
				multi = true;
				uint16 n = val.n - 4;
				c0 |= (0x10 | ((n & 0x0F00) >> 8));
				c1 = (n & 0x00FF);
			}

			retVal.push_back(c0);
			if (multi){retVal.push_back(c1);
			}
			retVal.push_back(val.val);
		} else 
		{
			std::vector<uint8> buffer;

			while ((val.n < 4 || directMode) && buffer.size() + val.n < 0x1FFF && i < basicRLE.size()) //DirectCopyEX MAX
			{
				for (uint32 j = 0; j < val.n; j++){buffer.push_back(val.val);
				}

				i++;
				val = (i < basicRLE.size() ? basicRLE[i] : val);
			}
			i--;

			if (buffer.size() >= 0x0F) //DirectCopyEX 
			{
				uint8 c0 = 0x20, c1;
				uint16 size = buffer.size();
				c0 |= ((size & 0x1F00) >> 8);
				c1 = (size & 0x00FF);

				retVal.push_back(c0);
				retVal.push_back(c1);
			} else retVal.push_back(buffer.size()); //DirectCopy

			for (auto c : buffer){retVal.push_back(c);
			}
		}
	}

	retVal.push_back(0x00);

	return retVal;
}

//struct Entry
//{
//	uint32 pos;
//	std::string str;
//
//	Entry(uint32 id = 0, std::string str = "")
//	{
//		this->pos = pos;
//		this->str = str;
//	}
//};

//Version 3.0
//Now uses a dictionary when RLE won't work
//Removed directMode
//std::vector<uint8> EncodeRLE4(std::vector<uint8>& data, uint32 dictSize = 0xFFFF)
//{
//	std::vector<EncodedData> basicRLE;
//	std::vector<uint8> retVal;
//
//	//Encode data into RLE
//	for (uint32 i = 0; i < data.size(); i++)
//	{
//		EncodedData d;
//		d.val = data[i];
//		while (data[i] == data[i + 1] && i < data.size() && d.n < 0x0FFF)
//		{
//			d.n++;
//			i++;
//		}
//
//		basicRLE.push_back(d);
//	}
//
//	//[entry, pos], string
//	std::unordered_map<uint32, Entry> dict;
//
//	//Reformat data
//	for (uint32 i = 0; i < basicRLE.size(); ++i)
//	{
//		EncodedData val = basicRLE[i];
//
//		if (val.n >= 4) //Repeat
//		{
//			bool multi = false;
//			uint8 c0 = 0x40, c1;
//
//			if (val.n < 0x0F) //Single char version (16 max)
//			{
//				c0 |= (val.n - 4);
//			} else
//			{
//				//Multi-char version (4095 max)
//				multi = true;
//				uint16 n = val.n - 4;
//				c0 |= (0x10 | ((n & 0x0F00) >> 8));
//				c1 = (n & 0x00FF);
//			}
//
//			retVal.push_back(c0);
//			if (multi){retVal.push_back(c1);
//			}
//			retVal.push_back(val.val);
//		} else 
//		{
//			std::vector<uint8> buffer;
//
//			while ((val.n < 4) && buffer.size() + val.n < 0x1FFF && i < basicRLE.size()) //DirectCopyEX MAX
//			{
//				for (uint32 j = 0; j < val.n; j++){buffer.push_back(val.val);
//				}
//
//				i++;
//				val = (i < basicRLE.size() ? basicRLE[i] : val);
//			}
//			i--;
//
//
//
//			if (buffer.size() >= 0x0F) //DirectCopyEX 
//			{
//				uint8 c0 = 0x20, c1;
//				uint16 size = buffer.size();
//				c0 |= ((size & 0x1F00) >> 8);
//				c1 = (size & 0x00FF);
//
//				retVal.push_back(c0);
//				retVal.push_back(c1);
//			} else retVal.push_back(buffer.size()); //DirectCopy
//
//			for (auto c : buffer){retVal.push_back(c);
//			}
//		}
//	}
//
//	retVal.push_back(0x00);
//
//	return retVal;
//}

}

#endif
