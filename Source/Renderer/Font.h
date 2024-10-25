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

struct Font;
class FontDatabase;

inline std::unique_ptr<FontDatabase> g_fontDatabase = std::make_unique<FontDatabase>();
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

	struct GLYF
	{
		std::map<char, GlyphShape> shapes;
	};

	struct FontDirectory
	{
		OffsetSubtable				offsetSubtable;
		std::map<ETableType, Table> tables;
		std::unique_ptr<Format4>	format = nullptr;
		std::unique_ptr<CMAP>		cmap = nullptr;
		std::unique_ptr<HEAD>		head = nullptr;
		std::unique_ptr<LOCA>		loca = nullptr;
		std::unique_ptr<GLYF>		glyf = nullptr;
	};

	inline int32 getGlyphIndex(uint16 codePoint, FontDirectory* fontDirectory)
	{
		int32 index = -1;
		auto  f = fontDirectory->format.get();
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
	inline uint32 getGlyphOffset(ByteReader& reader, FontDirectory* fontDirectory, uint32 glyphIndex, int32 initialOffset)
	{
		bool  bitSize32 = fontDirectory->head->indexToLocFormat != 0;
		int32 indexOffset = bitSize32 ? glyphIndex * sizeof(uint32) : glyphIndex * sizeof(uint16);
		reader.seek(initialOffset + indexOffset, ESeekDir::Beginning); // Reset to beginning of the LOCA table
		return bitSize32 ? reader.readUInt32() : reader.readUInt16() * 2;
	}

	inline void readGlyphCoordinates(ByteReader& reader, GlyphShape* shape, int32 index, EGlyphFlag byteFlag, EGlyphFlag deltaFlag)
	{
		int32 pointCount = shape->coordinates.size();
		int16 value = 0;

		// https://stevehanov.ca/blog/?id=143
		for (int32 i = 0; i < pointCount; i++)
		{
			// https://github.com/nothings/stb/blob/2e2bef463a5b53ddf8bb788e25da6b8506314c08/stb_truetype.h#L1726
			auto f = shape->flags[i];

			// Short
			if (f & byteFlag)
			{
				int16 delta = reader.readUInt8();
				value += (f & deltaFlag) ? delta : -delta;
			}
			// Long
			else if (!(f & deltaFlag))
			{
				int16 a = reader.readUInt8();
				int16 b = reader.readUInt8();
				value += (a << 8) + b;
			}
			shape->coordinates[i][index] = value;
		}
	}

	inline GlyphShape getGlyphShape(ByteReader& reader, FontDirectory* directory)
	{
		auto start = reader.ptr();

		GlyphShape shape{};
		shape.contourCount = reader.readUInt16();
		shape.xMin = reader.readUInt16();
		shape.yMin = reader.readUInt16();
		shape.xMax = reader.readUInt16();
		shape.yMax = reader.readUInt16();

		for (int32 i = 0; i < shape.contourCount; i++)
		{
			shape.contourEndPoints.emplace_back(reader.readUInt16());
		}
		assert(shape.contourEndPoints.size() == shape.contourCount);

		shape.instructionLength = reader.readUInt16();								   // Instruction size
		shape.instructions.resize(shape.instructionLength);							   // Resize vector to instruction size
		std::memcpy(shape.instructions.data(), reader.ptr(), shape.instructionLength); // Copy memory from reader ptr to instruction vector
		reader.seek(shape.instructionLength);										   // Offset reader by length of instructions

		// https://stackoverflow.com/a/36371452
		int32	   pointCount = shape.contourEndPoints.back() + 1;
		int32	   flagCount = 0;
		EGlyphFlag flag;
		for (int32 i = 0; i < pointCount; ++i)
		{
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

			shape.flags.emplace_back(flag);
		}

		// Update point count to number of flags
		shape.coordinates.resize(pointCount);
		readGlyphCoordinates(reader, &shape, 0, XShort, XShortPos);
		readGlyphCoordinates(reader, &shape, 1, YShort, YShortPos);
		return shape;
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

	inline void readCMAP(ByteReader& reader, FontDirectory* fontDirectory, int32 initialOffset)
	{
		auto cmap = fontDirectory->cmap.get();
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
		reader.seek(initialOffset + fontDirectory->cmap->subTables[0].offset, ESeekDir::Beginning);

		auto fmt = reader.readUInt16();
		auto length = reader.readUInt16();
		auto language = reader.readUInt16();

		switch (fmt)
		{
			case 4:
			{
				fontDirectory->format = std::make_unique<Format4>();

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
				break;
			}
		}
	}

	inline void readLOCA(ByteReader& reader, FontDirectory* fontDirectory, int32 initialOffset)
	{
		auto loca = fontDirectory->loca.get();

		for (auto c : g_alphabet)
		{
			int32 index = getGlyphIndex(c, fontDirectory);
			loca->glyphIndexes[c] = index;
			int32 offset = getGlyphOffset(reader, fontDirectory, index, initialOffset);
			loca->glyphOffsets[c] = offset;
#ifdef _DEBUG
			LOG_INFO("Glyph '{}' = Index: {}\tOffset: {}", c, index, offset)
#endif
		}
	}

	inline void readHEAD(ByteReader& reader, FontDirectory* fontDirectory)
	{
		auto head = fontDirectory->head.get();

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

	inline void readGLYF(ByteReader& reader, FontDirectory* fontDirectory, int32 initialOffset)
	{

		for (const auto& [k, v] : fontDirectory->loca->glyphOffsets)
		{
			reader.seek(initialOffset + v, ESeekDir::Beginning); // Reset to beginning of the GLYF table
			GlyphShape shape = getGlyphShape(reader, fontDirectory);
			fontDirectory->glyf->shapes[k] = shape;
		}
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

	inline bool readTables(ByteReader& reader, FontDirectory* fontDirectory)
	{

		// Read HEAD
		auto headTable = fontDirectory->tables[ETableType::HEAD];
		fontDirectory->head = std::make_unique<HEAD>();
		reader.seek(headTable.offset, ESeekDir::Beginning);
		readHEAD(reader, fontDirectory);

		//  Read CMAP
		auto cmapTable = fontDirectory->tables[ETableType::CMAP];
		fontDirectory->cmap = std::make_unique<CMAP>();
		readCMAP(reader, fontDirectory, cmapTable.offset);

		// Read LOCA
		auto locaTable = fontDirectory->tables[ETableType::LOCA];
		fontDirectory->loca = std::make_unique<LOCA>();
		readLOCA(reader, fontDirectory, locaTable.offset);

		// Read GLYF
		auto glyfTable = fontDirectory->tables[ETableType::GLYF];
		fontDirectory->glyf = std::make_unique<GLYF>();
		readGLYF(reader, fontDirectory, glyfTable.offset);

		return true;
	}

	inline void readFontDirectory(ByteReader& reader, FontDirectory* fontDirectory)
	{
		readOffsetSubtable(reader, &fontDirectory->offsetSubtable);

		int32 tableSize = fontDirectory->offsetSubtable.tableCount;
		readTableDirectory(reader, fontDirectory->tables, tableSize);
		readTables(reader, fontDirectory);

		if (!fontDirectory->format)
		{
			LOG_ERROR("Failed to read tables on font.")
			return;
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
			// if (path.find("arial") == std::string::npos)
			//{
			//	return;
			//}
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