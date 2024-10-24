#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "Core/Buffer.h"
#include "Core/Types.h"
#include "Core/IO.h"

/** https://handmade.network/forums/articles/t/7330-implementing_a_font_reader_and_rasterizer_from_scratch%252C_part_1__ttf_font_reader. **/

struct Font;
class FontDatabase;

inline std::unique_ptr<FontDatabase> g_fontDatabase = std::make_unique<FontDatabase>();

struct Font
{
	std::string family;
	std::string name;
};

enum class TTFScalarType : uint8
{
	True = 0x74727565,
	OpenType = 0x4F54544F,
	Typ1 = 0X74797031
};

struct OffsetSubtable
{
	uint32 scalarType{};
	uint16 tableCount{};
	uint16 searchRange{};
	uint16 entrySelector{};
	uint16 rangeShift{};
};

struct TTFTable
{
	std::string type;
	uint32		checkSum;
	uint32		offset;
	uint32		length;
};

struct TTFCMAPEncodingSubTable
{
	uint16 platformId;
	uint16 platformSpecificId;
	uint32 offset;
};

struct TTFCMAP
{
	uint16								 version;
	uint16								 subTableCount;
	std::vector<TTFCMAPEncodingSubTable> subTables;
};

struct FontDirectory
{
	OffsetSubtable		  offsetSubtable;
	std::vector<TTFTable> tables;
};

namespace TTF
{
	inline void printTableDirectory(std::vector<TTFTable>& tables, int32 tableSize)
	{
		for (const auto& t : tables)
		{
			LOG_INFO("{}\t\t{}\t\t{}", t.type, t.length, t.offset);
		}
	}

	inline void readOffsetSubtable(ByteReader& reader, OffsetSubtable* offsetSubtable)
	{
		offsetSubtable->scalarType = reader.readUInt32();
		offsetSubtable->tableCount = reader.readUInt16();
		offsetSubtable->searchRange = reader.readUInt16();
		offsetSubtable->entrySelector = reader.readUInt16();
		offsetSubtable->rangeShift = reader.readUInt16();
	}

	
	inline void readCMAP(ByteReader& reader, int32 offset, TTFCMAP* cmap)
	{
		reader.seek(offset, ESeekDir::Beginning);

		cmap->version = reader.readUInt16();
		cmap->subTableCount = reader.readUInt16();

		for (int32 i = 0; i < cmap->subTableCount; i++)
		{
			TTFCMAPEncodingSubTable encodingSubTable;
			encodingSubTable.platformId = reader.readUInt16();
			encodingSubTable.platformSpecificId = reader.readUInt16();
			encodingSubTable.offset = reader.readUInt32();
			cmap->subTables.emplace_back(encodingSubTable);
		}

		int a = 5;
	}


	inline void readTableDirectory(ByteReader& reader, std::vector<TTFTable>& tables, int32 tableSize)
	{
		for (int32 i = 0; i < tableSize; i++)
		{
			TTFTable t;

			// Read the tag into a string.
			uint8 tag[4]{};
			t.type.push_back(reader.readUInt8());
			t.type.push_back(reader.readUInt8());
			t.type.push_back(reader.readUInt8());
			t.type.push_back(reader.readUInt8());

			t.checkSum = reader.readUInt32();
			t.offset = reader.readUInt32();
			t.length = reader.readUInt32();

			tables.emplace_back(t);

			if (t.type == "cmap")
			{
				TTFCMAP cmap;
				readCMAP(reader, t.offset, &cmap);
				int a = 5;
			}

		}
	}

	inline void readFontDirectory(ByteReader& reader, FontDirectory* fontDirectory)
	{
		readOffsetSubtable(reader, &fontDirectory->offsetSubtable);

		int32 tableSize = fontDirectory->offsetSubtable.tableCount;
		readTableDirectory(reader, fontDirectory->tables, tableSize);
#ifdef _DEBUG
		printTableDirectory(fontDirectory->tables, tableSize);
#endif
	}
} // namespace TTF

class FontDatabase
{
	std::vector<Font> m_fonts;

	std::string getFontDirectory()
	{
#if defined(_WIN32) || defined(_WIN64)
		return "C:\\Windows\\Fonts";
#else
		return "";
#endif
	}

	void registerFont(std::string& data, const std::string& fileName)
	{
		LOG_INFO("Registering {}", fileName)

		ByteReader	  buffer(data, data.size(), std::endian::big);
		FontDirectory fontDirectory;
		TTF::readFontDirectory(buffer, &fontDirectory);
	}

	void loadFonts()
	{
		std::filesystem::path fontDir = getFontDirectory();
		if (!std::filesystem::exists(fontDir))
		{
			LOG_ERROR("OS not implemented.");
			return;
		}

		auto envy = "C:\\Users\\thoma\\Desktop\\Envy Code R.ttf";
		for (const auto& entry : std::filesystem::directory_iterator(fontDir))
		{
			int8*		data = nullptr;
			std::string path = entry.path().string();

			// Only read .ttf files
			if (!path.ends_with(".ttf"))
			{
				continue;
			}

			std::string strBuffer;
			if (!IO::readFile(envy, strBuffer))
			{
				LOG_ERROR("Failed to load font {}.", envy)
				return;
			}
			registerFont(strBuffer, envy);

			// TODO: Remove this `break`
			break;
		}
	}

public:
	void init()
	{
		loadFonts();
	}
};