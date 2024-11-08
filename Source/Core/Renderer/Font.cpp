#include "Font.h"

#include "Math/MathCommon.h"

// void FontDatabase::init()
//{
//	//FT_Error error = FT_Init_FreeType(&library);
//	//if (error)
//	//{
//	//	LOG_ERROR("Failed to initialize FreeType: {}", FT_Error_String(error))
//	//}
//	//loadFonts();
// }

void FontDatabase::loadFonts()
{
	for (auto& p : getFontDirectories())
	{
		for (const auto& entry : std::filesystem::directory_iterator(p))
		{
			std::string path = entry.path().string();

			// Only read .ttf files
			std::string lowerPath = path;
			Strings::toLower(lowerPath);
			if (!lowerPath.ends_with(".ttf"))
			{
				continue;
			}

			registerFont(path);
		}
	}
}

std::vector<std::string> FontDatabase::getFontDirectories()
{
#if defined(_WIN32) || defined(_WIN64)
	return { "C:\\Windows\\Fonts", "C:\\Users\\thoma\\AppData\\Local\\Microsoft\\Windows\\Fonts" };
#else
	return "";
#endif
}

void FontDatabase::registerFont(const std::string& fileName)
{
	// FT_Face face;
	// FT_New_Face(library, fileName.c_str(), 0, &face);
	// faces[face->family_name][face->style_name] = face;
}
//
// FT_Face FontDatabase::getFontInfo(const std::string& family, const std::string& subFamily)
//{
//	for (auto& [k, v] : faces)
//	{
//		if (k == family)
//		{
//			for (auto& [k2, v2] : v)
//			{
//				if (k2 == subFamily)
//				{
//					return v2;
//				}
//			}
//
//		}
//	}
//	LOG_ERROR("Font '{}:{}' not found.", family, subFamily)
//	return nullptr;
//}

float TTF::getScaleForPixelHeight(FontInfo* fontInfo, int32 lineHeight)
{
	int deltaHeight = fontInfo->ascender - fontInfo->descender;
	return (float)lineHeight / (float)deltaHeight;
}

int32 TTF::getGlyphIndex(uint16 codePoint, FontInfo* fontInfo)
{
	int32 index = -1;
	auto  f = fontInfo->format4.get();
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
uint32 TTF::getGlyphOffset(ByteReader& reader, FontInfo* fontInfo, uint32 glyphIndex, int32 initialOffset)
{
	bool  bitSize32 = fontInfo->head->indexToLocFormat != 0;
	int32 indexOffset = bitSize32 ? glyphIndex * sizeof(uint32) : glyphIndex * sizeof(uint16);
	reader.seek(initialOffset + indexOffset, ESeekDir::Beginning); // Reset to beginning of the LOCA table
	return bitSize32 ? reader.readUInt32() : reader.readUInt16() * 2;
}

void TTF::readGlyphCoordinates(ByteReader& reader, GlyphShape* shape, int32 index, EGlyphFlag byteFlag, EGlyphFlag deltaFlag)
{
	int32 pointCount = shape->points.size();

	// Value for each coordinate. All coordinates are sequential and either:
	// 1. If it's the first coordinate, it's just added to 0.
	// 2. If it's any other coordinate, it's added to the previous coordinate.
	int16 value = 0;

	// https://stevehanov.ca/blog/?id=143
	for (int32 i = 0; i < pointCount; i++)
	{
		GlyphVertex* v = &shape->points[i];
		EGlyphFlag	 f = v->flag;

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
		v->position[index] = value;
	}

	int a = 5;
}

std::vector<vec2i> TTF::interpolatePoints(std::vector<vec2i>& points)
{
	std::vector<vec2i> in = points; // Local copy
	std::vector<vec2i> out;
	int32			   curvePointCount = points.size();

	int32 limit = in.size() - 2; // Minimum of 3 points

	for (int32 i = 1; i <= limit; i++)
	{
		vec2i p0 = in[i - 1]; // Previous point
		vec2i p1 = in[i];	  // This point
		vec2i p2 = in[i + 1]; // Next point

		// Compute t value for this step
		float t = (float)i / (float)curvePointCount;

		// Lerp with points [0, 1], [1, 2], and then with [01, 12]
		vec2i p01 = Math::lerp(p0, p1, t);
		vec2i p12 = Math::lerp(p1, p2, t);
		vec2i p012 = Math::lerp(p01, p12, t);

		// Store the most recently-interpolated point
		out.emplace_back(p012);
	}

	return out;
}

bool TTF::getSimpleGlyphShape(ByteReader& reader, FontInfo* info, GlyphShape* glyph)
{
	glyph->minX = reader.readInt16();
	glyph->minY = reader.readInt16();
	glyph->maxX = reader.readInt16();
	glyph->maxY = reader.readInt16();
	glyph->width = glyph->maxX - glyph->minX;
	glyph->height = glyph->maxY - glyph->minY;

	for (int32 i = 0; i < glyph->contourCount; i++)
	{
		glyph->contourEndPoints.emplace_back(reader.readUInt16());
	}

	if (glyph->contourEndPoints.size() != glyph->contourCount)
	{
		return false;
	}

	glyph->instructionLength = reader.readUInt16(); // Instruction size
	if (!reader.canSeek(glyph->instructionLength))
	{
		LOG_ERROR("Unable to seek past instruction length.")
		return false;
	}
	glyph->instructions.resize(glyph->instructionLength);							 // Resize vector to instruction size
	std::memcpy(glyph->instructions.data(), reader.ptr(), glyph->instructionLength); // Copy memory from reader ptr to instruction vector
	reader.seek(glyph->instructionLength);											 // Offset reader by length of instructions

	// https://stackoverflow.com/a/36371452
	// https://github.com/freetype/freetype/blob/0ae7e607370cc66218ccfacf5de4db8a35424c2f/src/autofit/afhints.c#L989
	int32	   endPoint = glyph->contourEndPoints.front();
	int32	   pointCount = glyph->contourEndPoints.back() + 1;
	int32	   contourCount = glyph->contourCount;
	int32	   flagCount = 0;
	EGlyphFlag flag;
	int32	   end = info->tables[ETableType::GLYF].length;

	glyph->points.resize(pointCount);
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

		glyph->points[i].flag = flag;
	}

	// Update point count to number of flags
	readGlyphCoordinates(reader, glyph, 0, XShort, XShortPos);
	readGlyphCoordinates(reader, glyph, 1, YShort, YShortPos);

	// Triangulate the glyph
	std::vector<vec2i> points;
	for (auto& p : glyph->points)
	{
		points.emplace_back(p.position);
	}
	if (!Triangulation::triangulate(points, glyph->indexes))
	{
		LOG_ERROR("Failed to triangulate glyph '{}'", glyph->index)
		return false;
	}

	return true;
}

// https://github.com/nothings/stb/blob/2e2bef463a5b53ddf8bb788e25da6b8506314c08/stb_truetype.h#L1813
bool TTF::getCompoundGlyphShape(ByteReader& reader, FontInfo* info, GlyphShape* glyph)
{
	constexpr auto MORE_COMPONENTS = 0x0020;

	int32		   subGlyphCount = 0;
	bool		   canContinue = true;

	do
	{
		// Default transform
		float mtx[6] = { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };

		int flags = reader.readUInt16();
		int index = reader.readUInt16();

		if (ESubGlyphFlag::ARGS_ARE_XY_VALUES)
		{
			if (flags & 1) // Shorts
			{
				mtx[4] = reader.readUInt16();
				mtx[5] = reader.readUInt16();
			}
			else // chars
			{
				mtx[4] = reader.readUInt8();
				mtx[5] = reader.readUInt8();
			}
		}
		else
		{
			LOG_ERROR("Error matching subglyph flags")
			return false;
		}

		if (flags & ESubGlyphFlag::WE_HAVE_A_SCALE)
		{
			mtx[0] = mtx[3] = reader.readUInt16() / 16384.0f;
			mtx[1] = mtx[2] = 0;
		}
		else if (flags & ESubGlyphFlag::WE_HAVE_AN_X_AND_Y_SCALE)
		{
			mtx[0] = reader.readUInt16() / 16384.0f;
			mtx[1] = mtx[2] = 0;
			mtx[3] = reader.readUInt16() / 16384.0f;
		}
		else if (flags & ESubGlyphFlag::WE_HAVE_A_TWO_BY_TWO)
		{
			mtx[0] = reader.readUInt16() / 16384.0f;
			mtx[1] = reader.readUInt16() / 16384.0f;
			mtx[2] = reader.readUInt16() / 16384.0f;
			mtx[3] = reader.readUInt16() / 16384.0f;
		}

		// Find transformation scales
		float m = std::sqrt(mtx[0] * mtx[0] + mtx[1] * mtx[1]);
		float n = std::sqrt(mtx[2] * mtx[2] + mtx[3] * mtx[3]);

		GlyphShape subGlyph;
		if (!getGlyphShape(reader, info, &subGlyph))
		{
			canContinue = false;
		}

		glyph->subGlyphs.emplace_back(subGlyph);
		subGlyphCount++;

		break;
	}
	while (canContinue);

	return true;
}

bool TTF::getGlyphShape(ByteReader& reader, FontInfo* info, GlyphShape* glyph)
{
	auto start = reader.ptr();

	glyph->contourCount = reader.readUInt16();

	if (glyph->contourCount > 0)
	{
		return getSimpleGlyphShape(reader, info, glyph);
	}
	else
	{
		return getCompoundGlyphShape(reader, info, glyph);
	}
}

void TTF::readOffsetSubtable(ByteReader& reader, OffsetSubtable* offsetSubtable)
{
	offsetSubtable->scalarType = reader.readUInt32();
	offsetSubtable->tableCount = reader.readUInt16();
	offsetSubtable->searchRange = reader.readUInt16();
	offsetSubtable->entrySelector = reader.readUInt16();
	offsetSubtable->rangeShift = reader.readUInt16();
}

void TTF::readFormat4(ByteReader& reader, Format4* f)
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

// https://learn.microsoft.com/en-us/typography/opentype/spec/cmap
bool TTF::readCMAP(ByteReader& reader, FontInfo* fontInfo, int32 initialOffset)
{
	auto cmap = fontInfo->cmap.get();
	reader.seek(initialOffset, ESeekDir::Beginning);
	cmap->version = reader.readUInt16();
	cmap->subTableCount = reader.readUInt16();

	for (int32 i = 0; i < cmap->subTableCount; i++)
	{
		CMAPEncodingSubTable encodingSubTable{};
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
			fontInfo->format4 = std::make_shared<Format4>();
			auto f = fontInfo->format4.get();
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

// https://learn.microsoft.com/en-us/typography/opentype/spec/loca
void TTF::readLOCA(ByteReader& reader, FontInfo* fontInfo, int32 initialOffset)
{
	// Load all ASCII characters from 0 to 127
	for (int i = 0; i <= 128; i++)
	{
		char c = (char)i;
		if (c == '\0')
		{
			continue;
		}
		int32 index = getGlyphIndex(c, fontInfo);
		fontInfo->glyphIndexes[c] = index;
		int32 offset = getGlyphOffset(reader, fontInfo, index, initialOffset);
		fontInfo->glyphOffsets[c] = offset;
	}
}

// https://learn.microsoft.com/en-us/typography/opentype/spec/head
void TTF::readHEAD(ByteReader& reader, FontInfo* fontInfo)
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

// https://learn.microsoft.com/en-us/typography/opentype/spec/glyf
bool TTF::readGLYF(ByteReader& reader, FontInfo* fontInfo, int32 initialOffset)
{
	for (const auto& [k, v] : fontInfo->glyphOffsets)
	{
		reader.seek(initialOffset + v, ESeekDir::Beginning); // Reset to beginning of the GLYF table
		GlyphShape shape;
		if (!getGlyphShape(reader, fontInfo, &shape))
		{
			LOG_ERROR("Failed to read glyph shape {}", k)
			return false;
		}

		fontInfo->glyphs[k] = shape;
	}
	return true;
}

// https://learn.microsoft.com/en-us/typography/opentype/spec/name
bool TTF::readNAME(ByteReader& reader, FontInfo* fontInfo, int32 initialOffset)
{
	// Get record metadata
	reader.seek(initialOffset, ESeekDir::Beginning);
	fontInfo->format = reader.readUInt16();
	fontInfo->count = reader.readUInt16();
	fontInfo->stringOffset = reader.readUInt16();

	// Construct all records
	for (int32 i = 0; i < fontInfo->count; i++)
	{
		NameRecord record{};
		record.platformId = reader.readUInt16();
		record.platformSpecificId = reader.readUInt16();
		record.languageId = reader.readUInt16();
		record.nameId = reader.readUInt16();
		record.length = reader.readUInt16();
		record.offset = reader.readUInt16();
		fontInfo->records.emplace_back(record);
	}

	// Read all name strings
	for (int32 i = 0; i < fontInfo->records.size(); i++)
	{
		auto& record = fontInfo->records[i];
		reader.seek(initialOffset + fontInfo->stringOffset + record.offset + 1, ESeekDir::Beginning);

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
				fontInfo->family = text;
				break;
			case 2: // Subfamily
				fontInfo->subFamily = text;
				break;
			case 3: // Subfamily ID
				fontInfo->subFamilyId = text;
				break;
			case 4: // Full name
				fontInfo->fullName = text;
				break;
			default:
				continue;
		}
	}

	return true;
}

// https://learn.microsoft.com/en-us/typography/opentype/spec/hhea
bool TTF::readHHEA(ByteReader& reader, FontInfo* fontInfo, int32 initialOffset)
{
	reader.seek(initialOffset, ESeekDir::Beginning);

	fontInfo->majorVersion = reader.readUInt16();
	fontInfo->minorVersion = reader.readUInt16();
	fontInfo->ascender = reader.readInt16();
	fontInfo->descender = reader.readInt16();
	fontInfo->lineGap = reader.readInt16();
	fontInfo->advanceWidthMax = reader.readUInt16();
	fontInfo->minLeftSideBearing = reader.readInt16();
	fontInfo->minRightSideBearing = reader.readInt16();
	fontInfo->xMaxExtent = reader.readInt16();
	fontInfo->caretSlopeRise = reader.readInt16();
	fontInfo->caretSlopeRun = reader.readInt16();
	fontInfo->caretOffset = reader.readInt16();
	fontInfo->reserved = reader.readInt64();
	fontInfo->metricDataFormat = reader.readInt16();
	fontInfo->numAdvanceWidthMetrics = reader.readUInt16();

	return true;
}

// https://learn.microsoft.com/en-us/typography/opentype/spec/maxp
bool TTF::readMAXP(ByteReader& reader, FontInfo* fontInfo, int32 initialOffset)
{
	reader.seek(initialOffset, ESeekDir::Beginning);
	fontInfo->version.i = reader.readUInt32();
	fontInfo->glyphCount = reader.readUInt16();

	return true;
}

// https://learn.microsoft.com/en-us/typography/opentype/spec/hmtx
bool TTF::readHMTX(ByteReader& reader, FontInfo* fontInfo, int32 initialOffset)
{
	reader.seek(initialOffset, ESeekDir::Beginning);

	auto& glyphIndexes = fontInfo->glyphIndexes;
	int32 numAdvanceWidthMetrics = fontInfo->numAdvanceWidthMetrics;
	int32 glyphCount = fontInfo->glyphCount;
	int32 lastAdvanceWidth = 0;

	std::vector<std::pair<int, int>> layoutData(glyphCount);
	for (int32 i = 0; i < numAdvanceWidthMetrics; i++)
	{
		int32 advanceWidth = reader.readUInt16();
		int32 leftSideBearing = reader.readInt16();
		lastAdvanceWidth = advanceWidth;
		layoutData[i] = { advanceWidth, leftSideBearing };
	}

	int32 remaining = glyphCount - numAdvanceWidthMetrics;
	for (int32 i = 0; i < remaining; i++)
	{
		int32 leftSideBearing = reader.readInt16();
		int32 glyphIndex = numAdvanceWidthMetrics + i;
		layoutData[glyphIndex] = { lastAdvanceWidth, leftSideBearing };
	}

	for (auto& [c, glyphIndex] : glyphIndexes)
	{
		GlyphShape* glyph = &fontInfo->glyphs[c];
		glyph->advanceWidth = layoutData[glyphIndex].first;
		glyph->leftSideBearing = layoutData[glyphIndex].second;
	}

	return true;
}

void TTF::readTableInfo(ByteReader& reader, std::map<ETableType, Table>& tables, int32 tableSize)
{
	for (int32 i = 0; i < tableSize; i++)
	{
		Table t{};

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

bool TTF::readTables(ByteReader& reader, FontInfo* fontInfo)
{
	// Read NAME
	auto& nameTable = fontInfo->tables[ETableType::NAME];
	readNAME(reader, fontInfo, nameTable.offset);

	// Read HEAD
	auto& headTable = fontInfo->tables[ETableType::HEAD];
	fontInfo->head = std::make_shared<HEAD>();
	reader.seek(headTable.offset, ESeekDir::Beginning);
	readHEAD(reader, fontInfo);

	//  Read CMAP
	auto& cmapTable = fontInfo->tables[ETableType::CMAP];
	fontInfo->cmap = std::make_shared<CMAP>();
	if (!readCMAP(reader, fontInfo, cmapTable.offset))
	{
		return false;
	}
	// Read LOCA
	auto& locaTable = fontInfo->tables[ETableType::LOCA];
	readLOCA(reader, fontInfo, locaTable.offset);

	// Read GLYF
	auto& glyfTable = fontInfo->tables[ETableType::GLYF];
	if (!readGLYF(reader, fontInfo, glyfTable.offset))
	{
		return false;
	}

	// Read HHEA
	auto& hheaTable = fontInfo->tables[ETableType::HHEA];
	if (!readHHEA(reader, fontInfo, hheaTable.offset))
	{
		return false;
	}

	// Read MAXP
	auto& maxpTable = fontInfo->tables[ETableType::MAXP];
	if (!readMAXP(reader, fontInfo, maxpTable.offset))
	{
		return false;
	}

	// Read HMTX
	auto& hmtxTable = fontInfo->tables[ETableType::HMTX];
	if (!readHMTX(reader, fontInfo, hmtxTable.offset))
	{
		return false;
	}

	return true;
}

bool TTF::readfontInfo(ByteReader& reader, FontInfo* fontInfo)
{
	readOffsetSubtable(reader, &fontInfo->offsetSubtable);

	int32 tableSize = fontInfo->offsetSubtable.tableCount;
	readTableInfo(reader, fontInfo->tables, tableSize);
	return readTables(reader, fontInfo);
}
