#include <string>
#include <vector>
#include <deque>
#include <map>
#include <algorithm>
#include <sstream>

#include "ConsoleUtilities.h"

#ifndef STR_PARSER
#define STR_PARSER

namespace StringParser
{
	std::deque<std::string> Split(std::string str, char splitChar = ' ')
	{
		std::deque<std::string> parsedStr;
		
		std::string s;
		for (uint32 i = 0; i < str.size(); ++i)
		{
			if (str[i] == splitChar)
			{
				parsedStr.push_back(s);
				if (s.empty()){parsedStr.pop_back();
				}
				s.clear();
			} else s += str[i];
		}
		parsedStr.push_back(s);
		
		return parsedStr;
	}
//	std::deque<std::string> Split(std::string str, std::string splitStr = " ")
//	{
//		std::deque<std::string> parsedStr;
//		
//		std::string s;
//		for (uint32 i = 0; i < str.size(); ++i)
//		{
//			if (str[i] == splitChar)
//			{
//				parsedStr.push_back(s);
//				if (s.empty()){parsedStr.pop_back();
//				}
//				s.clear();
//			} else s += str[i];
//		}
//		parsedStr.push_back(s);
//		
//		return parsedStr;
//	}
	std::string Strip(std::string str, char stripChar = ' ')
	{
		uint32 index0 = str.find_first_not_of(stripChar);
		index0 = (index0 == std::string::npos ? 0 : index0);
		uint32 index1 = str.find_last_not_of(stripChar);
		index1 = (index1 == std::string::npos ? str.size()-1 : index1);
		return std::string(&str[index0], index1-index0+1);
	}
	
	std::string ToLower(std::string str)
	{
		std::string str0 = str;
		std::transform(str0.begin(), str0.end(), str0.begin(), [](char c)->char{return tolower(c);});
		return str0;
	}
	std::string ToUpper(std::string str)
	{
		std::string str0 = str;
		std::transform(str0.begin(), str0.end(), str0.begin(), [](char c)->char{return toupper(c);});
		return str0;
	}
	
	std::map<std::string, std::string> GetAllDataByTags(std::string data, const char *tagEncaser = "[]", char tagTerminator = '/')
	{
		std::map<std::string, std::string> output;
		
		for (uint32 i = 0; i < data.size(); ++i)
		{
			if (data[i] == tagEncaser[0])
			{
				std::string openingTag, terminatingTag, tagName;
				
				//Get opening tag and terminating tags ([DIRECTORY] and [/DIRECTORY])
				for (uint32 j = i; j < data.size(); ++j)
				{
					openingTag += data[j];
					terminatingTag += data[j];
					if (j == i){terminatingTag += tagTerminator;
					}
					
					if (data[j] == tagEncaser[1])
					{
						tagName = std::string(&openingTag[1], openingTag.size()-2);
						//tagName = DIRECTORY
						i = j+1;
						break;
					}
				}
				
				std::string str, potentialTag;
				str.reserve(256);
				
				for (;i < data.size()-1; ++i)
				{
					str += data[i];
					
					if (data[i+1] == tagEncaser[0])
					{
						uint32 index = data.find_first_of(tagEncaser[1], i);
						if (index == std::string::npos){continue;
						}
						potentialTag = std::string(&data[i+1], index-i);
						if (potentialTag == terminatingTag)
						{
							i += terminatingTag.size();
							break;
						}
					}
				}
				output[tagName] = str;
			}
		}
		
		return output;
	}
	
	std::string TagData(std::string tagName, std::string data, const char *tagEncaser = "[]", char tagTerminator = '/')
	{
		std::string tagOpen = std::string(tagEncaser, 2), tagClose = std::string(tagEncaser, 2);
		tagOpen.insert(1, tagName);
		tagClose.insert(1, 1, tagTerminator);
		tagClose.insert(2, tagName);
		return tagOpen+data+tagClose;
	}
	
	std::string GetDataByTag(std::string data, std::string tagName, const char *tagEncaser = "[]", char tagTerminator = '/')
	{
		std::string output;
		std::string openingTag = std::string(tagEncaser, 2), terminatingTag;
		openingTag.insert(1, tagName); //[tagName]
		terminatingTag = openingTag;
		terminatingTag.insert(1, 1, tagTerminator); //[/tagName]
		
		uint32 index0 = data.find(openingTag);
		uint32 index1 = data.find(terminatingTag);
		if (index0 != std::string::npos && index1 != std::string::npos && index0 < index1){output = std::string(&data[index0+openingTag.size()], index1-index0-(terminatingTag.size()-1));
		}
		
		return output;
	}
	
	std::string GetLine(std::string &data, uint32 line)
	{
		std::string str;
		std::stringstream s(data);
		for (uint32 i = 0; i < line; i++){std::getline(s, str);
		}
		
		return str;
	}
	uint32 GetLines(std::string &data)
	{
		uint32 lines = 0;
		std::string str;
		std::stringstream s(data);
		while (std::getline(s, str)){lines++;
		}
		
		return lines;
	}
};

#endif
