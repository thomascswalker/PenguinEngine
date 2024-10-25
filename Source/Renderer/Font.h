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
inline char							 g_alphabet[91] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghikjklmnoprstuvwxyz1234567890-=[]{};':\",./<>?!@#$%^&*()_+";

namespace TTF
{
	enum class ScalarType : uint8
	{
		True = 0x74727565,
		OpenType = 0x4F54544F,
		Typ1 = 0X74797031
	};

	enum class TableType : uint8
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

	enum class CMAPPlatform : uint8
	{
		Unicode = 0,
		Macintosh = 1,
		Reserved = 2,
		Microsoft = 3
	};

	enum class WindowsEncoding : uint8
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

	struct Table
	{
		std::string type;
		uint32		checkSum;
		uint32		offset;
		uint32		length;
	};

	struct CMAPEncodingSubTable
	{
		uint16 platformId;
		uint16 platformSpecificId;
		uint32 offset;
	};

	struct CMAPFormat
	{
		uint16 format;
		uint16 length;
		uint16 language;
	};

	struct CMAPFormat4 : CMAPFormat
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

	struct CMAP
	{
		uint16							  version;
		uint16							  subTableCount;
		std::vector<CMAPEncodingSubTable> subTables;
	};

	struct FontDirectory
	{
		OffsetSubtable				 offsetSubtable;
		std::vector<Table>			 tables;
		std::unique_ptr<CMAPFormat4> format = nullptr;
		std::map<char, int32>		 glyphIndexes;
	};

	inline void readOffsetSubtable(ByteReader& reader, OffsetSubtable* offsetSubtable)
	{
		offsetSubtable->scalarType = reader.readUInt32();
		offsetSubtable->tableCount = reader.readUInt16();
		offsetSubtable->searchRange = reader.readUInt16();
		offsetSubtable->entrySelector = reader.readUInt16();
		offsetSubtable->rangeShift = reader.readUInt16();
	}

	inline void readCMAP(ByteReader& reader, int32 offset, CMAP* cmap)
	{
		reader.seek(offset, ESeekDir::Beginning);

		cmap->version = reader.readUInt16();
		cmap->subTableCount = reader.readUInt16();

		for (int32 i = 0; i < cmap->subTableCount; i++)
		{
			CMAPEncodingSubTable encodingSubTable;
			encodingSubTable.platformId = reader.readUInt16();
			encodingSubTable.platformSpecificId = reader.readUInt16();
			encodingSubTable.offset = reader.readUInt32();
			cmap->subTables.emplace_back(encodingSubTable);
		}
	}

	inline void readTableDirectory(ByteReader& reader, std::vector<Table>& tables, int32 tableSize)
	{
		for (int32 i = 0; i < tableSize; i++)
		{
			Table t;

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

	inline void readFormat4(ByteReader& reader, CMAPFormat4* f)
	{
		int32 start = reader.getPos() - 6; // Start position is -3x u16

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
		int32 remaining = f->length - (end - start);
		for (int32 i = 0; i < remaining / 2; i++)
		{
			f->glyphIndexArray.emplace_back(reader.readUInt16());
		}
	}

	inline bool readTables(ByteReader& reader, FontDirectory* fontDirectory)
	{
		for (const auto& t : fontDirectory->tables)
		{
			if (t.type == "cmap")
			{
				CMAP cmap;
				readCMAP(reader, t.offset, &cmap);

				// Go to the offset of this CMAP subtable
				reader.seek(t.offset + cmap.subTables[0].offset, ESeekDir::Beginning);

				auto fmt = reader.readUInt16();
				auto length = reader.readUInt16();
				auto language = reader.readUInt16();

				switch (fmt)
				{
					case 4:
					{
						fontDirectory->format = std::make_unique<CMAPFormat4>();
						
						auto f = fontDirectory->format.get();
						f->format = fmt;
						f->length = length;
						f->language = language;
						readFormat4(reader, f);
						break;
					}
					default:
					{
						LOG_ERROR("Format {} not implemented.", fmt)
					}
				}
			}

		}
		return true;
	}

	inline int32 getGlyphIndex(uint16 codePoint, CMAPFormat4* f)
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
					uint16 result = f->glyphIndexArray[glyphIndex];

					return result;
				}
				else
				{
					return codePoint;
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
		for (const auto& t : fontDirectory->tables)
		{
			LOG_INFO("{}\t\t{}\t\t{}", t.type, t.length, t.offset);
		}
#endif
		readTables(reader, fontDirectory);

		if (!fontDirectory->format)
		{
			LOG_ERROR("Failed to read tables on font.")
			return;
		}

		for (auto c : g_alphabet)
		{
			int32 r = getGlyphIndex(c, fontDirectory->format.get());
			fontDirectory->glyphIndexes[c] = r;
#ifdef _DEBUG
			LOG_INFO("{} = {}", c, r)
#endif
		}
	}
} // namespace TTF

using namespace TTF;

class FontDatabase
{
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
		readFontDirectory(buffer, &fontDirectory);
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
			if (path.find("arial") == std::string::npos)
			{
				return;
			}
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