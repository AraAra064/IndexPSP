#include <vector>

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

#ifndef __SHADE_DECODER__
#define __SHADE_DECODER__

//My attempt at re-creating the decoding function (FUN_08908000) in C++ (it doesn't work kek)
//Maybe modify the function so it doesn't crash on bad data
std::vector<uint8> ShadeDecodeRLE(std::vector<uint8> &encoded, uint32 exSize = 0)
{
	const uint8 BACK_COPY = 0x80;
	const uint8 REPEAT = 0x40;
	
	std::vector<uint8> decoded;
	uint32 index = 0;
	
	if (exSize != 0){decoded.reserve(exSize);
	}
	
	while (encoded[index] != 0)
	{
		if (encoded[index] & BACK_COPY) // > 7F, true if non-zero
		{
			uint32 backCopySize = ((uint32)encoded[index] >> 0x05) + 4;
			uint32 backCopyDist = (encoded[index+1] | (encoded[index] & 0x1F) << 8);
			index++;
			
			if (backCopySize != 0)
			{
				for (uint32 i = 0, s = decoded.size(); i < backCopySize; i++){decoded.push_back(decoded[s - backCopyDist - 1]);
				}
			}
			
			//Don't know how to improve this
			while (true)
			{
				if ((encoded[index] & 0xE0) != 0x60){break;
				}
				index++;
				
				if (encoded[index] & 0x1F)
				{
					uint32 n = encoded[index] & 0x1F;
					
					for (uint32 i = 0, s = decoded.size(); i < n; i++){decoded.push_back(decoded[s - backCopyDist - 1]);
					}
				}
				index++;
			}
		} else if (encoded[index] & REPEAT)
		{
			uint32 n = encoded[index] & 0x0F;
			
			if (encoded[index] & 0x10) //REPEAT_EX
			{
				n = (encoded[index+1] | (encoded[index] & 0x0F) << 8);
				index++;
			}
			n += 4;
			index++;
			
			if (n != 0)
			{
				for (uint32 i = 0; i < n; i++){decoded.push_back(encoded[index]);
				}
			}
		} else 
		{
			uint32 size = (encoded[index] & 0x1F);
			
			if (encoded[index] & 0x20) //DirectCopyEX
			{
				size = (encoded[index+1] | (encoded[index] & 0x1F) << 8);
				index++;
			}
			index++;
			
			for (uint32 i = 0; i < size; i++){decoded.push_back(encoded[index++]);
			}
			index--;
		}
		
		index++;
	}
	
	return decoded;
}

#endif
