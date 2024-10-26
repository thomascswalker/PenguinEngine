#pragma once

#include <cassert>
#include <algorithm>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

#include "Core/Bitmask.h"
#include "Core/Buffer.h"
#include "Core/Types.h"
#include "Core/IO.h"
#include "Math/Vector.h"

/** https://handmade.network/forums/articles/t/7330-implementing_a_font_reader_and_rasterizer_from_scratch%252C_part_1__ttf_font_reader. **/
/** https://handmade.network/forums/wip/t/7610-reading_ttf_files_and_rasterizing_them_using_a_handmade_approach%252C_part_2__rasterization **/

class FontDatabase;
inline std::shared_ptr<FontDatabase> g_fontDatabase = std::make_shared<FontDatabase>();
inline char							 g_alphabet[91] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghikjklmnoprstuvwxyz1234567890-=[]{};':\",./<>?!@#$%^&*()_+";

typedef union
{
	uint16 s[2];
	uint32 i;
} Fixed;
using FWord = int16;
using UFWord = uint16;
using LongDateTime = int64;

namespace TTF
{
	enum class EScalarType : uint8
	{
		True = 0x74727565,
		OpenType = 0x4F54544F,
		Typ1 = 0X74797031
	};

	enum class ETableType : uint8
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

#define SET_TABLE_TYPE(name)       \
	if (tag == #name)              \
	{                              \
		t.type = ETableType::name; \
	}

	enum class ECMAPPlatform : uint8
	{
		Unicode = 0,
		Macintosh = 1,
		Reserved = 2,
		Microsoft = 3
	};

	enum class EWindowsEncoding : uint8
	{
		Symbol,
		UnicodeBMP,
		ShiftJIS,
		PRC,
		BigFive,
		Johab,
		UnicodeUCS4
	};

	enum EGlyphFlag : uint8
	{
		OnCurve = 1, // This point is on the curve.
		XShort = 2,	 // The corresponding byte is 1 byte, otherwise 2.
		YShort = 4,	 // The corresponding byte is 1 byte, otherwise 2.
		Repeat = 8,	 // The next byte specifies how many times this flag is repeated.
		XShortPos = 16,
		YShortPos = 32,
		Reserved1 = 64,	 // Reserved, not set in TrueType
		Reserved2 = 128, // Reserved, not set in TrueType
	};
	DEFINE_BITMASK_OPERATORS(EGlyphFlag);

	inline std::string glyphFlagToString(EGlyphFlag f)
	{
		static const char* flags[] = { "OnCurve", "XShort", "YShort", "Repeat", "XShortPos", "YShortPos", "Res1", "Res2", 0 }; // Synchronise with Color enum!

		// For each possible color string...
		std::string outString;
		for (const char* const* ptr = flags;
			 *ptr != 0;
			 ++ptr)
		{

			// Get whether to print something
			bool output = (f & 0x01) != 0;

			// Is color bit set?
			if (output)
			{
				// Yes! Output that string.
				outString += *ptr;
			} // if

			// Next bit in color
			f = (EGlyphFlag)(f >> 1);

			// All done?
			if (f == 0)
			{
				// Yes! Leave
				break;
			} // if

			// No, so show some more...
			if (output)
			{
				// If output something, need 'OR'
				outString += " | ";
			} // if
		}	  // for
		return outString;
	} // operator <<(Color)

	struct GlyphShape
	{
		uint16					contourCount;
		int16					xMin;
		int16					yMin;
		int16					xMax;
		int16					yMax;
		uint16					instructionLength;
		std::vector<uint8>		instructions;
		std::vector<EGlyphFlag> flags;
		std::vector<vec2i>		coordinates;
		std::vector<uint16>		contourEndPoints;
	};

	struct NameRecord
	{
		uint16 platformId;
		uint16 platformSpecificId;
		uint16 languageId;
		uint16 nameId;
		uint16 length;
		uint16 offset;
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
		ETableType type;
		uint32	   checkSum;
		uint32	   offset;
		uint32	   length;
	};

	struct CMAPEncodingSubTable
	{
		uint16 platformId;
		uint16 platformSpecificId;
		uint32 offset;
	};

	struct Format
	{
		uint16 format;
		uint16 length;
		uint16 language;
	};

	struct Format4 : Format
	{
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

	struct HEAD
	{
		Fixed version;
		Fixed fontRevision;

		uint32 checkSumAdjustment;
		uint32 magicNumber;

		uint16 flags;
		uint16 unitsPerEm;

		LongDateTime created;
		LongDateTime modified;

		FWord xMin;
		FWord yMin;
		FWord xMax;
		FWord yMax;

		uint16 macStyle;
		uint16 lowestRecPPEM;
		int16  fontDirectionHint;
		int16  indexToLocFormat;
		int16  glyphDataFormat;
	};

	struct LOCA
	{
		std::map<char, int32> glyphIndexes;
		std::map<char, int32> glyphOffsets;
	};

	struct GLYF
	{
		std::map<char, GlyphShape> shapes;
	};

	struct NAME
	{
		uint16					format;
		uint16					count;
		uint16					stringOffset;
		std::vector<NameRecord> records;

		std::string family;
		std::string subFamily;
		std::string subFamilyId;
		std::string fullName;
	};

	struct FontInfo
	{
		std::string					fileName;
		OffsetSubtable				offsetSubtable;
		std::map<ETableType, Table> tables;
		std::shared_ptr<Format4>	format = nullptr;
		std::shared_ptr<NAME>		name = nullptr;
		std::shared_ptr<CMAP>		cmap = nullptr;
		std::shared_ptr<HEAD>		head = nullptr;
		std::shared_ptr<LOCA>		loca = nullptr;
		std::shared_ptr<GLYF>		glyf = nullptr;
	};

	inline int32 getGlyphIndex(uint16 codePoint, FontInfo* fontInfo)
	{
		int32 index = -1;
		auto  f = fontInfo->format.get();
		switch (f->format)
		{
			case 4:
			{
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

				int32 startCode = f->startCode[index];
				if (startCode >= codePoint)
				{
					return 0;
				}

				int32 idRangeOffset = f->idRangeOffset[index];
				int32 idDelta = f->idDelta[index];
				if (idRangeOffset != 0)
				{
					int32 glyphIndex = codePoint - startCode;
					int32 result = f->glyphIndexArray[glyphIndex];
					return (result + idDelta) & 0xffff;
				}
				else
				{
					return (codePoint + idDelta) & 0xffff;
				}
			}
		}
		return 0;
	}

	/** Returns the byte offset of the specified Glyph relative to the GLYF table. **/
	inline uint32 getGlyphOffset(ByteReader& reader, FontInfo* fontInfo, uint32 glyphIndex, int32 initialOffset)
	{
		bool  bitSize32 = fontInfo->head->indexToLocFormat != 0;
		int32 indexOffset = bitSize32 ? glyphIndex * sizeof(uint32) : glyphIndex * sizeof(uint16);
		reader.seek(initialOffset + indexOffset, ESeekDir::Beginning); // Reset to beginning of the LOCA table
		return bitSize32 ? reader.readUInt32() : reader.readUInt16() * 2;
	}

	inline void readGlyphCoordinates(ByteReader& reader, GlyphShape* shape, int32 index, EGlyphFlag byteFlag, EGlyphFlag deltaFlag)
	{
		int32 pointCount = shape->coordinates.size();

		// Value for each coordinate. All coordinates are sequential and either:
		// 1. If it's the first coordinate, it's just added to 0.
		// 2. If it's any other coordinate, it's added to the previous coordinate.
		int16 value = 0;

		// https://stevehanov.ca/blog/?id=143
		for (int32 i = 0; i < pointCount; i++)
		{
			// https://github.com/nothings/stb/blob/2e2bef463a5b53ddf8bb788e25da6b8506314c08/stb_truetype.h#L1726
			auto f = shape->flags[i];

			// Char
			if (f & byteFlag)
			{
				// Read uint8 into int16
				int16 delta = reader.readUInt8();
				value += (f & deltaFlag) ? delta : -delta;
			}
			// Short
			else if (!(f & deltaFlag))
			{
				// Read the first and second half of this uint16
				int16 a = reader.readUInt8();
				int16 b = reader.readUInt8();
				value += (a << 8) + b;
			}

			// Set this coordinate's index (x, y) to the current value
			shape->coordinates[i][index] = value;
		}
	}

	inline bool getGlyphShape(ByteReader& reader, FontInfo* directory, GlyphShape* shape)
	{
		auto start = reader.ptr();

		shape->contourCount = reader.readUInt16();
		shape->xMin = reader.readInt16();
		shape->yMin = reader.readInt16();
		shape->xMax = reader.readInt16();
		shape->yMax = reader.readInt16();

		for (int32 i = 0; i < shape->contourCount; i++)
		{
			shape->contourEndPoints.emplace_back(reader.readUInt16());
		}
		if (shape->contourEndPoints.size() != shape->contourCount)
		{
			return false;
		}

		shape->instructionLength = reader.readUInt16(); // Instruction size
		if (!reader.canSeek(shape->instructionLength))
		{
			LOG_ERROR("Unable to seek past instruction length.")
			return false;
		}
		shape->instructions.resize(shape->instructionLength);							 // Resize vector to instruction size
		std::memcpy(shape->instructions.data(), reader.ptr(), shape->instructionLength); // Copy memory from reader ptr to instruction vector
		reader.seek(shape->instructionLength);											 // Offset reader by length of instructions

		// https://stackoverflow.com/a/36371452
		int32	   pointCount = shape->contourEndPoints.back() + 1;
		int32	   flagCount = 0;
		EGlyphFlag flag;
		int32	   end = directory->tables[ETableType::GLYF].length;
		for (int32 i = 0; i < pointCount; ++i)
		{
			if (reader.getPos() >= end)
			{
				LOG_ERROR("End of buffer.")
				return false;
			}
			if (flagCount == 0)
			{
				flag = (EGlyphFlag)(reader.readUInt8());
				if (flag & EGlyphFlag::Repeat)
				{
					flagCount = reader.readUInt8();
				}
			}
			else
			{
				--flagCount;
			}

			shape->flags.emplace_back(flag);
		}

		// Update point count to number of flags
		shape->coordinates.resize(pointCount);
		readGlyphCoordinates(reader, shape, 0, XShort, XShortPos);
		readGlyphCoordinates(reader, shape, 1, YShort, YShortPos);

		return true;
	}

	inline void readOffsetSubtable(ByteReader& reader, OffsetSubtable* offsetSubtable)
	{
		offsetSubtable->scalarType = reader.readUInt32();
		offsetSubtable->tableCount = reader.readUInt16();
		offsetSubtable->searchRange = reader.readUInt16();
		offsetSubtable->entrySelector = reader.readUInt16();
		offsetSubtable->rangeShift = reader.readUInt16();
	}

	inline void readFormat4(ByteReader& reader, Format4* f)
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
		reader.seek(2);
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

	inline bool readCMAP(ByteReader& reader, FontInfo* fontInfo, int32 initialOffset)
	{
		auto cmap = fontInfo->cmap.get();
		reader.seek(initialOffset, ESeekDir::Beginning);
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

		// Go to the offset of this CMAP subtable
		reader.seek(initialOffset + fontInfo->cmap->subTables[0].offset, ESeekDir::Beginning);

		auto fmt = reader.readUInt16();
		auto length = reader.readUInt16();
		auto language = reader.readUInt16();

		switch (fmt)
		{
			case 4:
			{
				fontInfo->format = std::make_shared<Format4>();

				auto f = fontInfo->format.get();
				f->format = fmt;
				f->length = length;
				f->language = language;
				readFormat4(reader, f);
				break;
			}
			default:
			{
#ifdef _DEBUG
				LOG_ERROR("Format {} not implemented.", fmt)
#endif
				return false;
			}
		}
		return true;
	}

	inline void readLOCA(ByteReader& reader, FontInfo* fontInfo, int32 initialOffset)
	{
		auto loca = fontInfo->loca.get();

		for (auto c : g_alphabet)
		{
			if (c == '\0')
			{
				continue;
			}
			int32 index = getGlyphIndex(c, fontInfo);
			loca->glyphIndexes[c] = index;
			int32 offset = getGlyphOffset(reader, fontInfo, index, initialOffset);
			loca->glyphOffsets[c] = offset;
		}
	}

	inline void readHEAD(ByteReader& reader, FontInfo* fontInfo)
	{
		auto head = fontInfo->head.get();

		head->version.s[0] = reader.readUInt16();
		head->version.s[1] = reader.readUInt16();

		head->fontRevision.s[0] = reader.readUInt16();
		head->fontRevision.s[1] = reader.readUInt16();

		head->checkSumAdjustment = reader.readUInt32();
		head->magicNumber = reader.readUInt32();

		head->flags = reader.readUInt16();
		head->unitsPerEm = reader.readUInt16();

		head->created = reader.readInt64();
		head->modified = reader.readInt64();

		head->xMin = reader.readInt16();
		head->yMin = reader.readInt16();
		head->xMax = reader.readInt16();
		head->yMax = reader.readInt16();

		head->macStyle = reader.readUInt16();
		head->lowestRecPPEM = reader.readUInt16();
		head->fontDirectionHint = reader.readInt16();
		head->indexToLocFormat = reader.readInt16();
		head->glyphDataFormat = reader.readInt16();
	}

	inline bool readGLYF(ByteReader& reader, FontInfo* fontInfo, int32 initialOffset)
	{

		for (const auto& [k, v] : fontInfo->loca->glyphOffsets)
		{
			reader.seek(initialOffset + v, ESeekDir::Beginning); // Reset to beginning of the GLYF table
			GlyphShape shape;
			if (!getGlyphShape(reader, fontInfo, &shape))
			{
				LOG_ERROR("Failed to read glyph shape {}", k)
				return false;
			}
			fontInfo->glyf->shapes[k] = shape;
		}
	}

	// https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6name.html
	inline bool readNAME(ByteReader& reader, FontInfo* fontInfo, int32 initialOffset)
	{
		// Get record metadata
		NAME* name = fontInfo->name.get();
		reader.seek(initialOffset, ESeekDir::Beginning);
		name->format = reader.readUInt16();
		name->count = reader.readUInt16();
		name->stringOffset = reader.readUInt16();

		// Construct all records
		for (int32 i = 0; i < name->count; i++)
		{
			NameRecord record;
			record.platformId = reader.readUInt16();
			record.platformSpecificId = reader.readUInt16();
			record.languageId = reader.readUInt16();
			record.nameId = reader.readUInt16();
			record.length = reader.readUInt16();
			record.offset = reader.readUInt16();
			name->records.emplace_back(record);
		}

		// Read all name strings
		for (int32 i = 0; i < name->records.size(); i++)
		{
			auto& record = name->records[i];
			reader.seek(initialOffset + name->stringOffset + record.offset + 1, ESeekDir::Beginning);

			// Read the text
			std::string text;

			// Read the raw record text into a buffer
			std::vector<uint8> textBuffer;
			reader.readSize(record.length, textBuffer);

			// For some reason there can be `\0` in between every character,
			// so we'll loop through the text buffer and if the current
			// element is a valid char, add it to the text string.
			for (int32 j = 0; j < textBuffer.size(); j++)
			{
				if (textBuffer[j] == '\0')
				{
					continue;
				}
				text.push_back(textBuffer[j]);
			}

			switch (record.nameId)
			{
				case 1: // Family
					name->family = text;
					break;
				case 2: // Subfamily
					name->subFamily = text;
					break;
				case 3: // Subfamily ID
					name->subFamilyId = text;
					break;
				case 4: // Full name
					name->fullName = text;
					break;
				default:
					continue;
			}
		}

		return true;
	}

	inline void readTableDirectory(ByteReader& reader, std::map<ETableType, Table>& tables, int32 tableSize)
	{
		for (int32 i = 0; i < tableSize; i++)
		{
			Table t;

			// Read the tag into a string.
			std::string tag = reader.readString(4);

			// Convert to uppercase
			Strings::toUpper(tag);

			// Necessary because no reflection
			SET_TABLE_TYPE(CMAP);
			SET_TABLE_TYPE(GLYF);
			SET_TABLE_TYPE(HEAD);
			SET_TABLE_TYPE(HHEA);
			SET_TABLE_TYPE(HMTX);
			SET_TABLE_TYPE(LOCA);
			SET_TABLE_TYPE(MAXP);
			SET_TABLE_TYPE(NAME);
			SET_TABLE_TYPE(POST);
			SET_TABLE_TYPE(CVT);
			SET_TABLE_TYPE(FPGM);
			SET_TABLE_TYPE(HDMX);
			SET_TABLE_TYPE(KERN);
			SET_TABLE_TYPE(OS2);
			SET_TABLE_TYPE(PREP);

			t.checkSum = reader.readUInt32();
			t.offset = reader.readUInt32();
			t.length = reader.readUInt32();

			if (tables.find(t.type) != tables.end())
			{
				// Skip duplicates
				continue;
			}
			tables[t.type] = t;
		}
	}

	inline bool readTables(ByteReader& reader, FontInfo* fontInfo)
	{
		// Read NAME
		auto nameTable = fontInfo->tables[ETableType::NAME];
		fontInfo->name = std::make_shared<NAME>();
		readNAME(reader, fontInfo, nameTable.offset);

		// Read HEAD
		auto headTable = fontInfo->tables[ETableType::HEAD];
		fontInfo->head = std::make_shared<HEAD>();
		reader.seek(headTable.offset, ESeekDir::Beginning);
		readHEAD(reader, fontInfo);

		//  Read CMAP
		auto cmapTable = fontInfo->tables[ETableType::CMAP];
		fontInfo->cmap = std::make_shared<CMAP>();
		if (!readCMAP(reader, fontInfo, cmapTable.offset))
		{
			return false;
		}
		// Read LOCA
		auto locaTable = fontInfo->tables[ETableType::LOCA];
		fontInfo->loca = std::make_shared<LOCA>();
		readLOCA(reader, fontInfo, locaTable.offset);

		// Read GLYF
		auto glyfTable = fontInfo->tables[ETableType::GLYF];
		fontInfo->glyf = std::make_shared<GLYF>();
		if (!readGLYF(reader, fontInfo, glyfTable.offset))
		{
			return false;
		}

		return true;
	}

	inline bool readfontInfo(ByteReader& reader, FontInfo* fontInfo)
	{
		readOffsetSubtable(reader, &fontInfo->offsetSubtable);

		int32 tableSize = fontInfo->offsetSubtable.tableCount;
		readTableDirectory(reader, fontInfo->tables, tableSize);
		return readTables(reader, fontInfo);
	}
} // namespace TTF

using namespace TTF;

class FontDatabase
{
	std::map<std::string, FontInfo> fonts;

	std::string getfontInfoPath()
	{
#if defined(_WIN32) || defined(_WIN64)
		return "C:\\Windows\\Fonts";
#else
		return "";
#endif
	}

	void registerFont(std::string& data, const std::string& fileName)
	{
		FontInfo font;
		font.fileName = fileName;
		ByteReader buffer(data, data.size(), std::endian::big);
		if (!readfontInfo(buffer, &font))
		{
			return;
		}
		fonts[font.name->family.c_str()] = font;
	}

	void loadFonts()
	{
		std::filesystem::path fontDir = getfontInfoPath();
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

			std::string strBuffer;
			if (!IO::readFile(path, strBuffer))
			{
				LOG_ERROR("Failed to load font {}.", path)
				return;
			}

			registerFont(strBuffer, path);
		}
	}

public:
	FontInfo* getFontInfo(const std::string& name)
	{
		for (auto& [k, v] : fonts)
		{
			if (k == name)
			{
				return &fonts[name.c_str()];
			}
		}
		return nullptr;
	}

	void init()
	{
		loadFonts();

		const char* str = "Test string.";
		FontInfo*	f = getFontInfo("Courier New");
		if (f != nullptr)
		{
			LOG_INFO("Found Courier New")
			LOG_INFO("Shape Count: {}", f->glyf->shapes.size())
		}
		else
		{
			LOG_ERROR("Unable to find Courier New")
		}
	}
};