#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <deque>

#ifndef __INDEXPSP_PSM__
	#define __INDEXPSP_PSM

namespace IndexPSM
{
	//maybe change this
	enum ModelID {WindmillNight0 = 0x00, WindmillNight = 0x10, WindmillNight2 = 0x20, WindmillDay = 0x30};
	enum FuncObjID {Player1 = 0x01, Player2, Genbu, Byakko, Suzaku, Seiryu};
	enum class DrawMode {Quads, Tri};

	struct Header
	{
		uint32_t magic;
		uint32_t drawSectionOffset;
		uint32_t verticiesOffset;
		uint32_t faceOffset;

		uint32_t colourOffset;
		uint32_t uvOffset;
		uint32_t hitDataOffset;
		uint32_t zeroOffset1;

		uint32_t zero1 = 0;
		uint32_t zeroOffset2;
		uint32_t zero2 = 0;
		uint32_t unknownOffset1;

		uint32_t unknownOffset2;
		uint32_t objectOffset;
		uint32_t zeroOffset3;
		uint32_t zeroOffset4;
	};

	struct DrawData
	{
		uint16_t unknown1;
		uint16_t unknown2;
		uint16_t startFace;
		uint16_t numFaces;
		float unknown3[4];

		DrawData(uint16_t startFace = 0, uint16_t numFaces = 0)
		{
			unknown1 = 0;
			unknown2 = 0;
			this->startFace = startFace;
			this->numFaces = numFaces;
			memset(unknown3, 0, sizeof(unknown3));
		}
	};

	struct VertexEntry
	{
		float z;
		float x;
		float y;
		uint32_t unknown;

		VertexEntry(float x = 0.f, float y = 0.f, float z = 0.f)
		{
			this->x = x;
			this->y = y;
			this->z = z;
			unknown = 0;
		}
	};

	//Add rest of data to struct
	struct FaceEntry
	{
		uint16_t index1;
		uint16_t index2;
		uint16_t index3;
		uint16_t index4;
		uint16_t unknown1; //0x??04
		uint16_t colourID; //temp as 0x00
		uint16_t textureID; //temp as 0x00
		uint8_t unknown2[0x06]; //0xFFFFFF
		uint8_t unknown3[0x1C]; //??

		FaceEntry()
		{
			index1 = 0;
			index2 = 0;
			index3 = 0;
			index4 = 0;
			unknown1 = 4;
			colourID = 0;
			textureID = 0;
			memset(unknown2, 0xFF, 0x06);
			memset(unknown3, 0x00, 0x1C);
		}
		FaceEntry(uint16_t i1, uint16_t i2, uint16_t i3, uint16_t i4, uint16_t colID = 0, uint16_t texID = 0)
		{
			index1 = i1;
			index2 = i2;
			index3 = i3;
			index4 = i4;
			unknown1 = 4;
			colourID = colID;
			textureID = texID;
			memset(unknown2, 0xFF, 0x06);
			memset(unknown3, 0x00, 0x1C);
		}
	};

	struct ColourEntry
	{
		uint8_t b;
		uint8_t g;
		uint8_t r;

		//0x00RRGGBB
		ColourEntry(uint32_t c = 0)
		{
			b = c & 0x000000FF;
			g = (c & 0x0000FF00) >> 8;
			r = (c & 0x00FF0000) >> 16;
		}
		ColourEntry(uint8_t r, uint8_t g, uint8_t b)
		{
			this->b = b;
			this->g = g;
			this->r = r;
		}

	};

	struct UVEntry
	{
		uint8_t u1;
		uint8_t v1;
		uint8_t u2;
		uint8_t v2;
		uint8_t u3;
		uint8_t v3;
		uint8_t u4;
		uint8_t v4;
		uint8_t textureID;
		uint8_t unknown1[0x03];

		//un = uvn & 0xFF
		//vn = (uvn >> 0x08) & 0xFF
		//Unscuff this
		UVEntry()
		{
			u1 = 0;
			v1 = 0;

			u2 = 0;
			v2 = 0;

			u3 = 0;
			v3 = 0;

			u4 = 0;
			v4 = 0;

			textureID = 0;
			memset(unknown1, 0, 3);
		}
		UVEntry(uint16_t uv1, uint16_t uv2, uint16_t uv3, uint16_t uv4, uint8_t texID)
		{
			u1 = uv1 & 0x00FF;
			v1 = (uv1 >> 8) & 0x00FF;

			u2 = uv2 & 0x00FF;
			v2 = (uv2 >> 8) & 0x00FF;

			u3 = uv3 & 0x00FF;
			v3 = (uv3 >> 8) & 0x00FF;

			u4 = uv4 & 0x00FF;
			v4 = (uv4 >> 8) & 0x00FF;

			textureID = texID;
			memset(unknown1, 0x00, 0x03);
		}
	};

	//type 0 (functional) objects
	//prop1 = id
	//prop2 = rotation
	//...

	//non zero type objects (visual)
	//type = modelid (?)
	//prop1 = rotation
	//prop2 = scale
	//...
	//Shadow OP :rose:
	struct ObjectEntry
	{
		float z;
		float x;
		float y;

		int16_t type;
		int16_t prop1;
		int16_t prop2;
		int16_t prop3;
		int16_t prop4;
		int16_t prop5;
		int16_t prop6;
		int16_t prop7;

		ObjectEntry()
		{
			z = 0.f;
			x = 0.f;
			y = 0.f;

			type = 0;
			prop1 = 0;
			prop2 = 0;
			prop3 = 0;
			prop4 = 0;
			prop5 = 0;
			prop6 = 0;
			prop7 = 0;
		}
		//unk3-unk6 is usually 0 so not adding it here yet
		ObjectEntry(float x, float y, float z, int16_t type, int16_t prop1, int16_t prop2, int16_t prop3)
		{
			this->z = z;
			this->x = x;
			this->y = y;
			
			this->type = type;
			this->prop1 = prop1;
			this->prop2 = prop2;
			this->prop3 = prop3;
			prop4 = 0;
			prop5 = 0;
			prop6 = 0;
			prop7 = 0;
		}

		//Check if object isn't null
		bool isNull(void)
		{
			uint32_t v = 0;
			v |= *(uint32_t*)&z;
			v |= *(uint32_t*)&x;
			v |= *(uint32_t*)&y;

			v |= type;
			v |= prop1;
			v |= prop2;
			v |= prop3;
			v |= prop4;
			v |= prop5;
			v |= prop6;
			v |= prop7;

			return v == 0; //if all is equal to zero
		}
		bool isFunctionalType(void)
		{
			return type == 0 && prop1 != 0;
		}
		bool isVisualType(void)
		{
			return type != 0;
		}

		//??
		//setVisualObject
		//setFunctionalObject

		void setPosition(float x, float y, float z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
			return;
		}
		//degrees
		void setRotation(int16_t rotation)
		{
			if (isVisualType())
			{
				prop1 = rotation;
			}
			else prop2 = rotation;

			return;
		}
		int16_t getRotation(void)
		{
			return (isVisualType() ? prop1 : prop2);
		}

		void setScale(int16_t scale)
		{
			if (isVisualType())
			{
				prop2 = scale;
			}

			return;
		}
		int16_t getScale(void)
		{
			return (isVisualType() ? prop2 : 0);
		}
	};

	const uint32_t C_PSM_HEADERSZ = 0x40;
	const uint32_t C_PSM_MAGIC = 0x304D5350; //PSM0
	const uint32_t C_PSM_MAGICOFF = 0x00; //Offset to get magic number value
	const uint32_t C_DRAW_ENTRYSZ = 0x18;
	const uint32_t C_DRAW_ENTRY_SECTOFF = 0x04; //Offset to get section offset
	const uint32_t C_VERTEX_ENTRYSZ = 0x10;
	const uint32_t C_VERTEX_ENTRY_SECTOFF = 0x08; //Offset to get section offset
	const uint32_t C_FACE_ENTRYSZ = 0x30;
	const uint32_t C_FACE_ENTRY_SECTOFF = 0x0C; //Offset to get section offset
	const uint32_t C_COLOUR_ENTRYSZ = 0x03;
	const uint32_t C_COLOUR_ENTRY_SECTOFF = 0x10; //Offset to get section offset
	const uint32_t C_UV_ENTRYSZ = 0x0C;
	const uint32_t C_UV_ENTRY_SECTOFF = 0x14; //Offset to get section offset
	const uint32_t C_HITDATA_SECTOFF = 0x18; //Offset to get section offset
	const uint32_t C_HITDATA_DATAOFF = 0x14; //Start of hitdata within its section
	const uint32_t C_SECT11_ENTRYSZ = 0; //UNKNOWN
	const uint32_t C_SECT11_ENTRY_SECTOFF = 0x2C;
	const uint32_t C_SECT12_ENTRYSZ = 0x0C; //UNKNOWN - 3 FLOATS?
	const uint32_t C_SECT12_ENTRY_SECTOFF = 0x30;
	const uint32_t C_OBJ_ENTRYSZ = 0x0C;
	const uint32_t C_OBJ_ENTRY_SECTOFF = 0x34; //Offset to get section offset

	//Returns true if first 4 bytes equals "PSM0"
	bool IsValid(std::vector<uint8_t> &psmData)
	{
		if (!psmData.empty() && psmData.size() >= 4)
		{
			return (*reinterpret_cast<uint32_t*>(&psmData[0]) == C_PSM_MAGIC);
		} else return false;
	}

	inline uint32_t GetVertexOffset(std::vector<uint8_t> &psmData){return *(uint32_t*)&psmData[0x08];
	}
	inline uint32_t GetFaceOffset(std::vector<uint8_t> &psmData){return *(uint32_t*)&psmData[0x0C];
	}
	inline uint32_t GetColourOffset(std::vector<uint8_t> &psmData){return *(uint32_t*)&psmData[0x10];
	}
	inline uint32_t GetUVOffset(std::vector<uint8_t> &psmData){return *(uint32_t*)&psmData[0x14];
	}
	inline uint32_t GetHitDataOffset(std::vector<uint8_t> &psmData){return *(uint32_t*)&psmData[0x18];
	}
	//Returns the n'th section's offset from the header of the PSM file
	uint32_t GetSectionOffset(std::vector<uint8_t> &psmData, uint32_t n){return *(uint32_t*)&psmData[n * 0x04];
	}
	////Returns the n'th section's size from the header of the PSM file
	//uint32_t GetSectionSize(std::vector<uint8_t> &psmData, uint32_t n)

	std::vector<DrawData> GetDrawOrder(std::vector<uint8_t>& psmData)
	{
		std::vector<DrawData> drawOrder;
		auto off = GetSectionOffset(psmData, 1);

		uint16_t nDraws = *(uint16_t*)&psmData[off]; //maybe just @ 0x40
		off += 0x04;

		for (uint16_t i = 0; i < nDraws; i++)
		{
			DrawData dd;
			memcpy(&dd, &psmData[off], sizeof(DrawData));
			drawOrder.push_back(dd);
			off += C_DRAW_ENTRYSZ;
		}

		return drawOrder;
	}

	std::vector<ObjectEntry> GetObjectEntries(std::vector<uint8_t>& psmData)
	{
		std::vector<ObjectEntry> entries;
		auto off = GetSectionOffset(psmData, 13);

		uint32_t nEntries = *(uint32_t*)&psmData[off];
		off += 0x04;

		for (uint16_t i = 0; i < nEntries; i++)
		{
			ObjectEntry e;
			memcpy(&e, &psmData[off], sizeof(ObjectEntry));
			entries.push_back(e);
			off += sizeof(ObjectEntry);
		}

		return entries;
	}

	//Returns the number of vertices within the PSM file
	uint16_t GetNumVertices(std::vector<uint8_t> &psmData)
	{
		auto off = GetVertexOffset(psmData);
		return *(uint16_t*)&psmData[off];
	}

	//Returns the number of faces within the PSM file
	uint32_t GetNumFaces(std::vector<uint8_t> &psmData)
	{
		auto off = GetFaceOffset(psmData);
		return *(uint32_t*)&psmData[off];
	}

	//Returns the number of colours within the PSM file
	uint16_t GetNumColours(std::vector<uint8_t> &psmData)
	{
		auto off = GetColourOffset(psmData);
		return *(uint16_t*)&psmData[off];
	}

	//Returns the number of UVs within the PSM file
	uint16_t GetNumUVs(std::vector<uint8_t> &psmData)
	{
		auto off = GetUVOffset(psmData);
		return *(uint16_t*)&psmData[off];
	}

	//Returns vertices in the format (x1, y1, z1), (x2, y2, z2), etc
	std::vector<float> GetVertices(std::vector<uint8_t> &psmData)
	{
		std::vector<float> vertices;

		auto off = GetVertexOffset(psmData) + 0x02; //size of nVertices
		auto numVertices = GetNumVertices(psmData);

		for (uint32_t i = 0; i < numVertices; i++)
		{
			vertices.push_back(*(float*)&psmData[off + 4]); //z-psm
			vertices.push_back(*(float*)&psmData[off + 8]); //y-psm
			vertices.push_back(*(float*)&psmData[off]); //x-psm
			off += C_VERTEX_ENTRYSZ;
		}

		return vertices;
	}
	//Returns vertices using VertexEntry structure
	std::vector<VertexEntry> GetVertexEntries(std::vector<uint8_t>& psmData)
	{
		std::vector<VertexEntry> entries(GetNumVertices(psmData));
		memcpy(entries.data(), &psmData[GetVertexOffset(psmData)], entries.size() * sizeof(VertexEntry));
		return entries;
	}

	//Returns indices (QUADS/TRIS)
	std::vector<uint16_t> GetIndices(std::vector<uint8_t> &psmData, DrawMode m = DrawMode::Quads)
	{
		std::vector<uint16_t> indices;

		auto off = GetFaceOffset(psmData) + 0x04; //size of nFaces
		auto numFaces = GetNumFaces(psmData);

		for (uint32_t i = 0; i < numFaces; i++)
		{
			uint16_t i1 = *(uint16_t*)&psmData[off + 0];
			uint16_t i2 = *(uint16_t*)&psmData[off + 2];
			uint16_t i3 = *(uint16_t*)&psmData[off + 4];
			uint16_t i4 = *(uint16_t*)&psmData[off + 6];

			if (m == DrawMode::Quads)
			{
				indices.push_back(i1);
				indices.push_back(i2);
				indices.push_back(i3);
				indices.push_back(i4);
			}
			else
			{
				indices.push_back(i1);
				indices.push_back(i2);
				indices.push_back(i3);

				indices.push_back(i3);
				indices.push_back(i4);
				indices.push_back(i1);
			}
			
			off += C_FACE_ENTRYSZ;
		}

		return indices;
	}
	//Returns face table using FaceEntry structure
	std::vector<FaceEntry> GetFaceEntries(std::vector<uint8_t>& psmData)
	{
		std::vector<FaceEntry> entries(GetNumFaces(psmData));
		memcpy(entries.data(), &psmData[GetFaceOffset(psmData)], entries.size() * sizeof(FaceEntry));
		return entries;
	}

	//Returns a table of colours in the format 0x00RRGGBB
	//Not to be indexed directly, use with GetColourIDList or use GetColours instead
	std::vector<uint32_t> GetColourTable(std::vector<uint8_t>& psmData)
	{
		std::vector<uint32_t> colourTable;
		auto nColours = GetNumColours(psmData);

		if (nColours != 0)
		{
			colourTable.reserve(nColours);

			for (uint32_t i = 0, off = GetColourOffset(psmData) + 0x02; i < nColours; i++)
			{
				uint32_t c = 0x00000000; //*(uint32_t*)&psmData[off] & 0x00FFFFFF;
				memcpy(&c, &psmData[off], 0x03);
				colourTable.push_back(c);
				off += 0x03;
			}
		}

		return colourTable;
	}
	//Returns colour table using ColourEntry structure
	std::vector<ColourEntry> GetColourEntries(std::vector<uint8_t>& psmData)
	{
		std::vector<ColourEntry> entries(GetNumColours(psmData));
		memcpy(entries.data(), &psmData[GetColourOffset(psmData)], entries.size() * sizeof(ColourEntry));
		return entries;
	}

	//Returns list of base colour IDs
	//Uses baseID -> baseID + 4 from the colour table
	std::vector<uint16_t> GetColourIDList(std::vector<uint8_t>& psmData)
	{
		std::vector<uint16_t> ids;

		auto off = GetFaceOffset(psmData) + 0x04; //size of nFaces
		auto numFaces = GetNumFaces(psmData);

		for (uint32_t i = 0; i < numFaces; i++)
		{
			uint16_t cID = *(uint16_t*)&psmData[off + 0x0A];
			ids.push_back(cID);
			off += C_FACE_ENTRYSZ;
		}

		return ids;
	}
	//Returns colours in the format 0x00RRGGBB
	//Size of returned value is equal to the number of vertices
	//std::vector<uint32_t> GetColours(std::vector<uint8_t>& psmData)
	//{
	//	std::vector<uint32_t> colours;
	//	auto cTable = GetColourTable(psmData);
	//	auto cIDs = GetColourIDList(psmData);
	//	colours.reserve(cIDs.size() * 4); //4 because quads
	//
	//	for (uint32_t i = 0; i < cIDs.size(); i++)
	//	{
	//		uint32_t colourIndex = cIDs[i];
	//		colours.push_back(cTable[colourIndex + 0]);
	//		colours.push_back(cTable[colourIndex + 1]);
	//		colours.push_back(cTable[colourIndex + 2]);
	//		colours.push_back(cTable[colourIndex + 3]);
	//	}
	//
	//	return colours;
	//}

	//Returns UVs in the format (u1, v1), ..., (u4, v4), etc
	//4 sets of UVs per face
	std::vector<uint8_t> GetUVTable(std::vector<uint8_t>& psmData)
	{
		std::vector<uint8_t> uvs;

		auto off = GetUVOffset(psmData) + 0x02; //size of nUVs
		auto numFaces = GetNumUVs(psmData);

		for (uint32_t i = 0; i < numFaces; i++)
		{
			uvs.push_back(*(uint32_t*)&psmData[off + 0]); //u
			uvs.push_back(*(uint32_t*)&psmData[off + 1]); //v

			uvs.push_back(*(uint32_t*)&psmData[off + 2]);
			uvs.push_back(*(uint32_t*)&psmData[off + 3]);

			uvs.push_back(*(uint32_t*)&psmData[off + 4]);
			uvs.push_back(*(uint32_t*)&psmData[off + 5]);

			uvs.push_back(*(uint32_t*)&psmData[off + 6]);
			uvs.push_back(*(uint32_t*)&psmData[off + 7]);
			off += C_UV_ENTRYSZ;
		}

		return uvs;
	}
	//Returns UV Table using UVEntry structure
	std::vector<UVEntry> GetUVEntries(std::vector<uint8_t>& psmData)
	{
		std::vector<UVEntry> entries(GetNumUVs(psmData));
		memcpy(entries.data(), &psmData[GetUVOffset(psmData)], entries.size() * sizeof(UVEntry));
		return entries;
	}
	//Returns lists of UV IDs
	//1 ID per 4 sets of UVs
	std::vector<uint16_t> GetUVIDList(std::vector<uint8_t>& psmData)
	{
		std::vector<uint16_t> ids;

		auto off = GetFaceOffset(psmData) + 0x04; //size of nFaces
		auto numFaces = GetNumFaces(psmData);

		for (uint32_t i = 0; i < numFaces; i++)
		{
			uint16_t cID = *(uint16_t*)&psmData[off + 0x0C];
			ids.push_back(cID);
			off += C_FACE_ENTRYSZ;
		}

		return ids;
	}
	//Returns UVs in the format (u1, v1), ..., (u4, v4), etc
	//4 sets of UVs per face
	//std::vector<uint8_t> GetUVs(std::vector<uint8_t>& psmData)
	//{
	//	std::vector<uint8_t> uvs;
	//
	//	auto uvTable = GetUVTable(psmData);
	//	auto uvIDs = GetUVIDList(psmData);
	//	uvs.reserve(uvIDs.size() * 8); //8=uvsz
	//
	//	for (uint32_t i = 0; i < uvIDs.size(); i++)
	//	{
	//		uint32_t uvIndex = uvIDs[i];
	//		uvs.push_back(uvTable[uvIndex + 0]);
	//		uvs.push_back(uvTable[uvIndex + 1]);
	//
	//		uvs.push_back(uvTable[uvIndex + 2]);
	//		uvs.push_back(uvTable[uvIndex + 3]);
	//
	//		uvs.push_back(uvTable[uvIndex + 4]);
	//		uvs.push_back(uvTable[uvIndex + 5]);
	//
	//		uvs.push_back(uvTable[uvIndex + 6]);
	//		uvs.push_back(uvTable[uvIndex + 7]);
	//	}
	//
	//	return uvs;
	//}

	//Returns ID-1 of texture in CRAF file 
	std::vector<uint8_t> GetTexIDList(std::vector<uint8_t>& psmData)
	{
		std::vector<uint8_t> ids;
		auto uvIDs = GetUVIDList(psmData);

		auto baseOff = GetUVOffset(psmData) + 0x02; //size of nUVs
		auto numFaces = GetNumFaces(psmData);
		auto nUVs = GetNumUVs(psmData);

		for (uint32_t i = 0; i < numFaces; i++)
		{
			if (uvIDs[i] != 0xFFFF)
			{
				uint32_t off = uvIDs[i] * C_UV_ENTRYSZ;
				ids.push_back(*(uint8_t*)&psmData[baseOff + off + 0x08]);
			} else ids.push_back(0xFF);
		}

		return ids;
	}

	//Assuming first 3 floats from offset is origin
	//Returns last two floats, first float usually == 4.f
	std::pair<float, float> GetHitDataOrigin(std::vector<uint8_t>& psmData)
	{
		std::pair<float, float> origin;

		auto off = GetHitDataOffset(psmData);
		origin.first = *(float*)&psmData[off + 0x04];
		origin.second = *(float*)&psmData[off + 0x08];

		return origin;
	}

	//No idea what hit data is exactly other than the game doesn't let you move without it
	uint32_t GetHitDataSZ(std::vector<uint8_t> &psmData)
	{
		auto off = GetHitDataOffset(psmData) + 0x0C;
		uint16_t s1 = *(uint16_t*)&psmData[off]; //s1 may be 2nd float / 2
		uint16_t s2 = *(uint16_t*)&psmData[off + 2];
		uint16_t s3 = *(uint16_t*)&psmData[off + 4];
		return (s1 * s2 * 2) + (s3 * 2) + 8;
	}
	//Returns how seperate parts of HitDataSZ, 0xFFFF if i >= 3
	uint16_t GetHitDataSX(std::vector<uint8_t>& psmData, unsigned int i)
	{
		auto off = GetHitDataOffset(psmData) + 0x0C;
		return (i < 3 ? *(uint16_t*)&psmData[off + (i * 2)] : -1);
	}

	//Returns "HitData"
	std::vector<uint8_t> GetHitData(std::vector<uint8_t>& psmData)
	{
		auto off = GetHitDataOffset(psmData) + C_HITDATA_DATAOFF;
		std::vector<uint8_t> data(GetHitDataSZ(psmData));
		memcpy(&data[0], &psmData[off], data.size());
		return data;
	}
	//Returns whole hitdata section
	std::vector<uint8_t> GetHitData2(std::vector<uint8_t>& psmData)
	{
		auto off = GetHitDataOffset(psmData);
		std::vector<uint8_t> data(GetHitDataSZ(psmData) + C_HITDATA_DATAOFF);
		memcpy(&data[0], &psmData[off], data.size());
		return data;
	}

	std::vector<uint8_t> CreateHitData(std::pair<float, float> origin, uint16_t s1, uint16_t s2, uint16_t s3, std::vector<uint8_t> data)
	{
		const float z = 4.f;
		std::vector<uint8_t> hitData(C_HITDATA_DATAOFF + data.size(), 0x00);
	
		memcpy(&hitData[0x00], &z, sizeof(float));
		memcpy(&hitData[0x04], &origin.first, sizeof(float));
		memcpy(&hitData[0x08], &origin.second, sizeof(float));
	
		memcpy(&hitData[0x0C], &s1, sizeof(uint16_t));
		memcpy(&hitData[0x0E], &s2, sizeof(uint16_t));
		memcpy(&hitData[0x10], &s3, sizeof(uint16_t));
	
		memcpy(&hitData[C_HITDATA_DATAOFF], &data[0], data.size());
	
		return hitData;
	}

	//scuffed
	//Returns a PSM file for the game to use
	std::vector<uint8_t> CreatePSM(std::vector<DrawData>& drawData, std::vector<VertexEntry>& vertices, std::vector<FaceEntry>& faces, std::vector<ColourEntry>& colourTable, std::vector<UVEntry>& uvTable, std::vector<uint8_t>& hitData, std::vector<ObjectEntry>& objects)
	{
		uint32_t offset = 0x00, sectionSize = C_PSM_HEADERSZ;
		uint32_t entryCount = 0;
		std::vector<uint8_t> psmData(C_PSM_HEADERSZ, 0x00);
		memcpy(&psmData[0x00], &C_PSM_MAGIC, 0x04);
		offset += sectionSize;
	
		//First section (DrawOrder)
		memcpy(&psmData[C_DRAW_ENTRY_SECTOFF], &offset, sizeof(uint32_t)); //change header
		sectionSize = drawData.size() * sizeof(DrawData) + sizeof(uint32_t);
		psmData.resize(psmData.size() + sectionSize, 0x00); //increase size
		entryCount = drawData.size() & (drawData.size() << 0x10);
		memcpy(&psmData[offset], &entryCount, sizeof(uint32_t));
		memcpy(&psmData[offset + sizeof(uint32_t)], &drawData[0], sectionSize - sizeof(uint32_t));
		offset += sectionSize;
	
		//Second section (Vertices)
		entryCount = vertices.size();
		memcpy(&psmData[C_VERTEX_ENTRY_SECTOFF], &offset, sizeof(uint32_t)); //change header
		sectionSize = entryCount * sizeof(VertexEntry) + sizeof(uint32_t);
		psmData.resize(psmData.size() + sectionSize, 0x00); //increase size
		memcpy(&psmData[offset], &entryCount, sizeof(uint32_t));
		memcpy(&psmData[offset + 4], &vertices[0], vertices.size() * sizeof(VertexEntry));
		offset += sectionSize;
	
		//Third section (Face data)
		entryCount = faces.size();
		memcpy(&psmData[C_FACE_ENTRY_SECTOFF], &offset, sizeof(uint32_t)); //change header
		sectionSize = entryCount * sizeof(FaceEntry) + sizeof(uint32_t);
		psmData.resize(psmData.size() + sectionSize, 0x00); //increase size
		memcpy(&psmData[offset], &entryCount, sizeof(uint32_t));
		memcpy(&psmData[offset + 4], &faces[0], faces.size() * sizeof(FaceEntry));
		offset += sectionSize;
	
		//Forth section (Colour table data)
		entryCount = colourTable.size();
		memcpy(&psmData[C_COLOUR_ENTRY_SECTOFF], &offset, sizeof(uint16_t)); //change header
		sectionSize = entryCount * C_COLOUR_ENTRYSZ + sizeof(uint16_t);
		psmData.resize(psmData.size() + sectionSize, 0x00); //increase size
		memcpy(&psmData[offset], &entryCount, sizeof(uint16_t));
		memcpy(&psmData[offset + 2], &colourTable[0], colourTable.size() * C_COLOUR_ENTRYSZ);
		offset += sectionSize;
	
		//Fifth section (UV table data)
		entryCount = uvTable.size();
		memcpy(&psmData[C_UV_ENTRY_SECTOFF], &offset, sizeof(uint32_t)); //change header
		sectionSize = entryCount * sizeof(UVEntry) + sizeof(uint32_t);
		psmData.resize(psmData.size() + sectionSize, 0x00); //increase size
		memcpy(&psmData[offset], &entryCount, sizeof(uint32_t));
		memcpy(&psmData[offset + 4], &uvTable[0], uvTable.size() * sizeof(UVEntry));
		offset += sectionSize;
	
		//Sixth section ("Hit data")
		sectionSize = hitData.size();
		memcpy(&psmData[C_HITDATA_SECTOFF], &offset, sizeof(uint32_t));
		psmData.resize(psmData.size() + sectionSize, 0x00);
		memcpy(&psmData[offset], &hitData[0], hitData.size());
		offset += sectionSize;
	
		//Seventh section (UNKNOWN)
		//...
		sectionSize = 4;
		memcpy(&psmData[0x1C], &offset, sizeof(uint32_t));
		psmData.resize(psmData.size() + sectionSize, 0x00);
		offset += sectionSize;
	
		//Zero sections 1 and 2
		//... ...
		sectionSize = 4;
		memcpy(&psmData[0x24], &offset, sizeof(uint32_t));
		psmData.resize(psmData.size() + sectionSize, 0x00);
		offset += sectionSize;
	
	
		//12th section (UNKNOWN)
		//...
		sectionSize = 4;
		memcpy(&psmData[0x2C], &offset, sizeof(uint32_t));
		psmData.resize(psmData.size() + sectionSize, 0x00);
		offset += sectionSize;
	
		//13th section (Object section)
		entryCount = objects.size();
		memcpy(&psmData[C_OBJ_ENTRY_SECTOFF], &offset, sizeof(uint32_t)); //change header
		sectionSize = entryCount * sizeof(ObjectEntry) + sizeof(uint32_t);
		psmData.resize(psmData.size() + sectionSize, 0x00); //increase size
		memcpy(&psmData[offset], &entryCount, sizeof(uint32_t));
		memcpy(&psmData[offset + 4], &objects[0], objects.size() * sizeof(ObjectEntry));
		offset += sectionSize;
	
		//Zero sections 3 and 4
		//... ...
		sectionSize = 8;
		psmData.resize(psmData.size() + sectionSize, 0x00);
		offset += sectionSize;
	
		return psmData;
	}
}

#endif
