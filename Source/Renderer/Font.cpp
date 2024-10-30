#include "Font.h"

#include "Math/MathCommon.h"

float TTF::getScaleForPixelHeight(FontInfo* fontInfo, int32 lineHeight)
{
	int deltaHeight = fontInfo->hhea->ascender - fontInfo->hhea->descender;
	return (float)lineHeight / (float)deltaHeight;
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

/**
 * @brief Converts non-control points to control points by interpolating the
 * beginning and end control points of that segment.
 */
void TTF::convertGlyphPoints(GlyphShape  * glyph)
{
	// Copy existing vertices to a local array
	std::vector<GlyphVertex>& vertices = glyph->vertices;

	// Resize the vertex array to double size
	int32 vertexCount = vertices.size();

	// Starting point for any uninterpreted vertices
	int32 nextEndPointIndex = glyph->contourEndPoints[0];
	int32 endPointIndex = 0;

	int32 i = 0;
	int32 j = 0;

	while (i < glyph->contourCount)
	{
		GlyphContour contour;
		int32 endPoint = glyph->contourEndPoints[i];
		while (j < endPoint)
		{
			GlyphVertex v = vertices[j];
			v.type = Line;
			v.contourIndex = endPoint;
			contour.points.emplace_back(v);
			j++;
		}
		GlyphVertex v = vertices[j];
		v.type = End;
		v.contourIndex = endPoint;
		contour.points.emplace_back(v);
		i++;
		j++;

		glyph->contours.emplace_back(contour);
	}

	int a = 5;
	// Pointer to the start of the new vertex array
	// GlyphVertex* v = vertices.data();
	//
	// for (int32 i = 0; i < vertexCount; i++)
	//{
	//	auto  flag = v[i].flag;
	//	int32 x = v[i].position.x;
	//	int32 y = v[i].position.y;

	//	if (nextMove == i)
	//	{
	//		// We're at the final point of this contour, so close the shape
	//		if (i != 0)
	//		{
	//			if (startOff)
	//			{
	//				if (wasOff)
	//				{
	//					setVertexProperties(&v[num_vertices++], EGlyphVertexType::Curve, (cx + scx) >> 1, (cy + scy) >> 1, cx, cy);
	//				}
	//				setVertexProperties(&v[num_vertices++], EGlyphVertexType::Curve, sx, sy, scx, scy);
	//			}
	//			else
	//			{
	//				if (wasOff)
	//				{
	//					setVertexProperties(&v[num_vertices++], EGlyphVertexType::Curve, sx, sy, cx, cy);
	//				}
	//				else
	//				{
	//					setVertexProperties(&v[num_vertices++], EGlyphVertexType::Control, sx, sy, 0, 0);
	//				}
	//			}
	//		}

	//		startOff = !(flag & EGlyphFlag::OnCurve);

	//		// if we start off with an off-curve point, then when we need to find a point on the curve
	//		// where we can start, and we need to save some state for when we wraparound.
	//		if (startOff)
	//		{

	//			scx = x;
	//			scy = y;

	//			// If the next point is also an off-curve point, interpolate an on-point curve
	//			GlyphVertex* vn = &v[ i + 1];
	//			if (!(vn->flag & EGlyphFlag::OnCurve))
	//			{
	//				sx = (x + vn->position.x) >> 1;
	//				sy = (y + vn->position.y) >> 1;
	//			}
	//			// Otherwise use the next point as our start point
	//			else
	//			{
	//				sx = vn->position.x;
	//				sy = vn->position.y;
	//				i++;
	//			}
	//		}
	//		else
	//		{
	//			sx = x;
	//			sy = y;
	//		}
	//		setVertexProperties(&v[num_vertices++], EGlyphVertexType::New, sx, sy, 0, 0);
	//		wasOff = false;
	//		nextMove = glyph->contourEndPoints[endPointIndex] + 1; // TODO: FIX
	//		++endPointIndex;
	//	}
	//	else
	//	{
	//		if (flag & EGlyphFlag::OnCurve) // Control point
	//		{
	//			if (wasOff) // Two curve control points in a row
	//			{
	//				setVertexProperties(&v[num_vertices++], EGlyphVertexType::Curve, (cx + x) >> 1, (cy + y) >> 1, cx, cy);
	//			}
	//			cx = x;
	//			cy = y;
	//			wasOff = true;
	//		}
	//		else // Curve point
	//		{
	//			if (wasOff)
	//			{
	//				setVertexProperties(&v[num_vertices++], EGlyphVertexType::Curve, sx, sy, cx, cy);
	//			}
	//			else
	//			{
	//				setVertexProperties(&v[num_vertices++], EGlyphVertexType::Control, sx, sy, 0, 0);
	//			}
	//		}
	//	}
	//}
}

bool TTF::getSimpleGlyphShape(ByteReader& reader, FontInfo* info, GlyphShape* glyph)
{
	int32 x0 = reader.readInt16();
	int32 y0 = reader.readInt16();
	int32 x1 = reader.readInt16();
	int32 y1 = reader.readInt16();
	glyph->bounds = recti(x0, y0, x0 + x1, y0 + y1);

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

	glyph->vertices.resize(pointCount);
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

		glyph->vertices[i].flag = flag;
	}

	// Update point count to number of flags
	readGlyphCoordinates(reader, glyph, 0, XShort, XShortPos);
	readGlyphCoordinates(reader, glyph, 1, YShort, YShortPos);

	convertGlyphPoints(glyph);

	return true;
}

// https://github.com/nothings/stb/blob/2e2bef463a5b53ddf8bb788e25da6b8506314c08/stb_truetype.h#L1813
bool TTF::getCompoundGlyphShape(ByteReader& reader, FontInfo* info, GlyphShape* glyph)
{
#define MORE_COMPONENTS 0x0020
	int32 subGlyphCount = 0;
	bool  canContinue = true;

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
