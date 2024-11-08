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
#include "Math/Rect.h"
#include "Math/Matrix.h"
#include "Math/Line.h"
#include <Engine/Mesh.h>

/** https://handmade.network/forums/articles/t/7330-implementing_a_font_reader_and_rasterizer_from_scratch%252C_part_1__ttf_font_reader. **/
/** https://handmade.network/forums/wip/t/7610-reading_ttf_files_and_rasterizing_them_using_a_handmade_approach%252C_part_2__rasterization **/

struct GlyphVertex;
class FontDatabase;
inline std::shared_ptr<FontDatabase> g_fontDatabase = std::make_shared<FontDatabase>();

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

	enum ESubGlyphFlag : uint16
	{
		ARG_1_AND_2_ARE_WORDS = 0,
		ARGS_ARE_XY_VALUES = 1 << 0,
		ROUND_XY_TO_GRID = 2 << 0,
		WE_HAVE_A_SCALE = 3 << 0,
		OBSOLETE = 4 << 0, // DON'T USE
		MORE_COMPONENTS = 5 << 0,
		WE_HAVE_AN_X_AND_Y_SCALE = 6 << 0,
		WE_HAVE_A_TWO_BY_TWO = 7 << 0,
		WE_HAVE_INSTRUCTIONS = 8 << 0,
		USE_MY_METRICS = 9 << 0,
		OVERLAP_COMPOUND = 10 << 0
	};
	DEFINE_BITMASK_OPERATORS(ESubGlyphFlag);

	enum EGlyphVertexType
	{
		Line,
		Curve,
		End,
	};

	struct GlyphVertex
	{
		vec2i			 position;
		EGlyphVertexType type;
		EGlyphFlag		 flag;
		int32			 contourIndex;
	};

	struct GlyphEdge
	{
		linei line;
		bool  invert = false;

		bool operator>(const GlyphEdge& other)
		{
			bool result = false;
			result |= line.a.y > other.line.a.y;
			result |= line.a.y > other.line.b.y;
			result |= line.b.y > other.line.a.y;
			result |= line.b.y > other.line.b.y;
			return result;
		}

		bool operator<(const GlyphEdge& other)
		{
			bool result = false;
			result |= line.a.y < other.line.a.y;
			result |= line.a.y < other.line.b.y;
			result |= line.b.y < other.line.a.y;
			result |= line.b.y < other.line.b.y;
			return result;
		}
	};

	struct GlyphShape
	{
		// Simple
		uint16					 contourCount;
		std::vector<GlyphVertex> points;
		std::vector<Index3>		 indexes;

		int16 minX;
		int16 maxX;
		int16 minY;
		int16 maxY;
		int16 width;
		int16 height;

		uint16			   instructionLength;
		std::vector<uint8> instructions;

		std::vector<uint16> contourEndPoints;
		int32				advanceWidth;
		int32				leftSideBearing;

		// Compound
		std::vector<GlyphShape> subGlyphs;
		int32					index;
		uint16					flags;
		int32					arg1;
		int32					arg2;
		mat2i					transform;

		std::vector<vec2i> getPositions() const
		{
			std::vector<vec2i> positions;
			for (int32 i = 0; i < points.size(); i++)
			{
				positions.emplace_back(points[i].position);
			}
			return positions;
		}
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

	struct FontInfo
	{
		std::string					fileName;
		OffsetSubtable				offsetSubtable;
		std::map<ETableType, Table> tables;

		// NAME
		uint16					format;
		uint16					count;
		uint16					stringOffset;
		std::vector<NameRecord> records;

		std::string family;
		std::string subFamily;
		std::string subFamilyId;
		std::string fullName;

		// LOCA
		std::map<char, int32> glyphIndexes;
		std::map<char, int32> glyphOffsets;

		// GLYF
		std::map<char, GlyphShape> glyphs;

		// MAXP
		Fixed version;
		int32 glyphCount;

		// HHEA
		uint16 majorVersion;
		uint16 minorVersion;
		FWord  ascender;
		FWord  descender;
		FWord  lineGap;
		UFWord advanceWidthMax;
		FWord  minLeftSideBearing;
		FWord  minRightSideBearing;
		FWord  xMaxExtent;
		int16  caretSlopeRise;
		int16  caretSlopeRun;
		int16  caretOffset;
		int64  reserved;
		int16  metricDataFormat;
		uint16 numAdvanceWidthMetrics;

		std::shared_ptr<Format4> format4 = nullptr;
		std::shared_ptr<CMAP>	 cmap = nullptr;
		std::shared_ptr<HEAD>	 head = nullptr;
	};

	float getScaleForPixelHeight(FontInfo* fontInfo, int32 pixelHeight);
	int32 getGlyphIndex(uint16 codePoint, FontInfo* fontInfo);
	/** Returns the byte offset of the specified Glyph relative to the GLYF table. **/
	uint32			   getGlyphOffset(ByteReader& reader, FontInfo* fontInfo, uint32 glyphIndex, int32 initialOffset);
	void			   readGlyphCoordinates(ByteReader& reader, GlyphShape* shape, int32 index, EGlyphFlag byteFlag, EGlyphFlag deltaFlag);
	std::vector<vec2i> interpolatePoints(std::vector<vec2i>& points);
	bool			   getSimpleGlyphShape(ByteReader& reader, FontInfo* info, GlyphShape* glyph);
	// https://github.com/nothings/stb/blob/2e2bef463a5b53ddf8bb788e25da6b8506314c08/stb_truetype.h#L1813
	bool getCompoundGlyphShape(ByteReader& reader, FontInfo* info, GlyphShape* glyph);
	bool getGlyphShape(ByteReader& reader, FontInfo* info, GlyphShape* glyph);
	void readOffsetSubtable(ByteReader& reader, OffsetSubtable* offsetSubtable);
	void readFormat4(ByteReader& reader, Format4* f);
	// https://learn.microsoft.com/en-us/typography/opentype/spec/cmap
	bool readCMAP(ByteReader& reader, FontInfo* fontInfo, int32 initialOffset);
	// https://learn.microsoft.com/en-us/typography/opentype/spec/loca
	void readLOCA(ByteReader& reader, FontInfo* fontInfo, int32 initialOffset);
	// https://learn.microsoft.com/en-us/typography/opentype/spec/head
	void readHEAD(ByteReader& reader, FontInfo* fontInfo);
	// https://learn.microsoft.com/en-us/typography/opentype/spec/glyf
	bool readGLYF(ByteReader& reader, FontInfo* fontInfo, int32 initialOffset);
	// https://learn.microsoft.com/en-us/typography/opentype/spec/name
	bool readNAME(ByteReader& reader, FontInfo* fontInfo, int32 initialOffset);
	// https://learn.microsoft.com/en-us/typography/opentype/spec/hhea
	bool readHHEA(ByteReader& reader, FontInfo* fontInfo, int32 initialOffset);
	// https://learn.microsoft.com/en-us/typography/opentype/spec/maxp
	bool readMAXP(ByteReader& reader, FontInfo* fontInfo, int32 initialOffset);
	// https://learn.microsoft.com/en-us/typography/opentype/spec/hmtx
	bool readHMTX(ByteReader& reader, FontInfo* fontInfo, int32 initialOffset);
	void readTableInfo(ByteReader& reader, std::map<ETableType, Table>& tables, int32 tableSize);
	bool readTables(ByteReader& reader, FontInfo* fontInfo);
	bool readfontInfo(ByteReader& reader, FontInfo* fontInfo);

	struct FontFamily
	{
		std::map<std::string, FontInfo> fonts;
	};
} // namespace TTF

using namespace TTF;

class FontDatabase
{
	//FT_Library					   library;
	//std::map<std::string, std::map<std::string, FT_Face>> faces;

	std::vector<std::string> getFontDirectories();
	void		registerFont(const std::string& fileName);
	void		loadFonts();

public:
	//FT_Face getFontInfo(const std::string& family, const std::string& subFamily);
	//void	  init();
	//FT_Library getLibrary() { return library; }
};