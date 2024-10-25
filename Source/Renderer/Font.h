#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "Core/Buffer.h"
#include "Core/Types.h"
#include "Core/IO.h"

/** https://handmade.network/forums/articles/t/7330-implementing_a_font_reader_and_rasterizer_from_scratch%252C_part_1__ttf_font_reader. **/

struct Font;
class FontDatabase;

inline std::unique_ptr<FontDatabase> g_fontDatabase = std::make_unique<FontDatabase>();
inline char							 g_alphabet[89] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghikjklmnoprstuvwxyz1234567890-=[]{};':\",./<>?!@#$%^&*()";
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

enum class TTFTableType : uint8
{
	CMAP,
	GLYF,
	HEAD,
	HHEA,
	HMTX,
	LOCA,
	MAXP,
	NAME,
	POST,

	// Optional
	CVT,
	FPGM,
	HDMX,
	KERN,
	OS2,
	PREP
};

enum class TTFCMAPPlatform : uint8
{
	Unicode = 0,
	Macintosh = 1,
	Reserved = 2,
	Microsoft = 3
};

enum class TTFWindowsEncoding : uint8
{
	Symbol,
	UnicodeBMP,
	ShiftJIS,
	PRC,
	BigFive,
	Johab,
	UnicodeUCS4
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

struct TTFCMAPFormat
{
	uint16 format;
	uint16 length;
	uint16 language;
};

struct TTFCMAPFormat4 : TTFCMAPFormat
{
	uint16				format = 4;
	uint16				length;
	uint16				language;
	uint16				segCountX2;
	uint16				searchRange;
	uint16				entrySelector;
	uint16				rangeShift;
	uint16				reservedPad;
	std::vector<uint16> endCode;
	std::vector<uint16> startCode;
	std::vector<uint16> idDelta;
	std::vector<uint16> idRangeOffset;
	std::vector<uint16> glyphIndexArray;
};

struct TTFCMAP
{
	uint16								 version;
	uint16								 subTableCount;
	std::vector<TTFCMAPEncodingSubTable> subTables;
};

struct FontDirectory
{
	OffsetSubtable					offsetSubtable;
	std::vector<TTFTable>			tables;
	std::unique_ptr<TTFCMAPFormat4> format = nullptr;
	std::map<char, int32>			glyphIndexes;
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
		}
	}

	inline void readTables(ByteReader& reader, FontDirectory* fontDirectory)
	{
		for (const auto& t : fontDirectory->tables)
		{
			if (t.type == "cmap")
			{
				TTFCMAP cmap;
				readCMAP(reader, t.offset, &cmap);

				// Go to the offset of this CMAP subtable
				reader.seek(t.offset + cmap.subTables[0].offset, ESeekDir::Beginning);
				int32 start = reader.getPos();

				auto fmt = reader.readUInt16();
				auto length = reader.readUInt16();
				auto language = reader.readUInt16();

				switch (fmt)
				{
					case 4:
					{
						fontDirectory->format = std::make_unique<TTFCMAPFormat4>();
						auto f = fontDirectory->format.get();
						f->format = fmt;
						f->length = length;
						f->language = language;
						f->segCountX2 = reader.readUInt16();
						f->searchRange = reader.readUInt16();
						f->entrySelector = reader.readUInt16();
						f->rangeShift = reader.readUInt16();

						int32 segmentCount = f->segCountX2 / 2;
						for (int32 i = 0; i < segmentCount; i++)
						{
							f->endCode.emplace_back(reader.readUInt16());
						}
						reader.seek(1);
						for (int32 i = 0; i < segmentCount; i++)
						{
							f->startCode.emplace_back(reader.readUInt16());
						}
						for (int32 i = 0; i < segmentCount; i++)
						{
							f->idDelta.emplace_back(reader.readUInt16());
						}
						for (int32 i = 0; i < segmentCount; i++)
						{
							f->idRangeOffset.emplace_back(reader.readUInt16());
						}
						int32 end = reader.getPos();
						int32 remaining = length - (end - start);
						for (int32 i = 0; i < remaining / 2; i++)
						{
							f->glyphIndexArray.emplace_back(reader.readUInt16());
						}
					}
				}

				break;
			}
		}
	}

	inline int32 getGlyphIndex(uint16 codePoint, TTFCMAPFormat4* f)
	{
		int32 index = -1;
		switch (f->format)
		{
			case 4:
			{
				uint16* ptr = nullptr;

				for (int32 i = 0; i < f->segCountX2 / 2; i++)
				{
					if (f->endCode[i] > codePoint)
					{
						index = i;
						break;
					}
				}

				if (index == -1)
				{
					return 0;
				}

				if (f->startCode[index] >= codePoint)
				{
					return 0;
					;
				}

				if (f->idRangeOffset[index] != 0)
				{
					uint16 idDelta = f->idDelta[index];
					uint16 glyphIndex = codePoint - f->startCode[index];
					uint16 result = f->glyphIndexArray[glyphIndex] + f->idDelta[index];

					return result;
				}
				else
				{
					return codePoint + f->idDelta[index];
				}
			}
				return 0;
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
		readTables(reader, fontDirectory);

		for (auto c : g_alphabet)
		{
			int32 r = getGlyphIndex(c, fontDirectory->format.get());
			fontDirectory->glyphIndexes[c] = r;
		}
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
#ifdef _DEBUG
		for (const auto& [k, v] : fontDirectory.glyphIndexes)
		{
			LOG_INFO("{} = {}", k, v)
		}
#endif
	}

	void loadFonts()
	{
		std::filesystem::path fontDir = getFontDirectory();
		if (!std::filesystem::exists(fontDir))
		{
			LOG_ERROR("OS not implemented.");
			return;
		}

		for (const auto& entry : std::filesystem::directory_iterator(fontDir))
		{
			int8*		data = nullptr;
			std::string path = entry.path().string();

			// Only read .ttf files
			if (!path.ends_with(".ttf"))
			{
				continue;
			}

			// TODO: Remove this hardcoded path
			path = "C:\\Users\\thoma\\Desktop\\Envy Code R.ttf";
			std::string strBuffer;
			if (!IO::readFile(path, strBuffer))
			{
				LOG_ERROR("Failed to load font {}.", path)
				return;
			}
			registerFont(strBuffer, path);

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