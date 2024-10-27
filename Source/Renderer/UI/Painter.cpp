#include <cassert>

#include "Painter.h"
#include "Engine/Actors/Camera.h"

inline std::string g_defaultFont = "Arial";
constexpr int32	   g_scaleFactor = 150;

void Painter::assertValid()
{
	assert(m_data != nullptr);
}

Painter::Painter(Texture* data, recti viewport)
	: m_data(data), m_viewport(viewport)
{
	setFont(g_fontDatabase->getFontInfo(g_defaultFont));
}

void Painter::drawLine(vec2i a, vec2i b, const Color& color)
{
	int32 x0 = std::clamp(a.x, 0, m_viewport.width);
	int32 y0 = std::clamp(a.y, 0, m_viewport.height);
	int32 x1 = std::clamp(b.x, 0, m_viewport.width);
	int32 y1 = std::clamp(b.y, 0, m_viewport.height);

	int32 dx = std::abs(x1 - x0);
	int32 sx = x0 < x1 ? 1 : -1;

	int32 dy = -std::abs(y1 - y0);
	int32 sy = y0 < y1 ? 1 : -1;

	int32 err = dx + dy;
	int32 e2 = 0;

	while (true)
	{
		if (x0 == x1 && y0 == y1)
		{
			break;
		}
		m_data->setPixelFromColor(x0, y0, color);

		e2 = 2 * err;
		if (e2 >= dy)
		{
			err += dy;
			x0 += sx;
		}
		if (e2 <= dx)
		{
			err += dx;
			y0 += sy;
		}
	}
}

void Painter::drawRect(recti r, const Color& color, int32 thickness)
{
	assertValid();

	// Color value as int32
	int32 value = color.toInt32();

	// Clamp the rectangle to the current viewport size
	r.clamp(m_viewport);

#if defined(_WIN32) || defined(_WIN64)
	// On Windows we have to flip the Y values because Win32 draws
	// from the bottom up, rather than top down.

	vec2i rmin = r.min();
	vec2i rmax = r.max();
	int32 height = m_viewport.height;

	// Start row
	int32 end = std::clamp(height - rmin.y - g_windowHeightClip, 0, m_viewport.max().y);
	// End row
	int32 start = std::clamp(height - rmax.y - g_windowHeightClip, 0, m_viewport.max().y);

	int32 _thickness = thickness - 1; // True thickness in pixels
	for (int32 row = start; row < end; row++)
	{
		// Pointer to the first pixel in this row with the X offset
		uint32* ptr = m_data->scanline(row) + r.min().x;

		// Fill the entire row if it's either the start or end row,
		// accounting for thickness
		if (row <= (start + thickness) || row >= (end - thickness))
		{
			std::fill(ptr, ptr + (int32)r.width + 1, value);
		}
		// Otherwise only set the pixels on the left and right,
		// accounting for thickness.
		else
		{
			for (int32 i = 0; i < thickness; i++)
			{
				*(ptr + i) = value;					   // Left-side
				*(ptr + ((int32)r.width - i)) = value; // Right-side
			}
		}
	}
#else
	// Start row
	int32 start = std::clamp(r.min().y, 0, m_viewport.max().y);
	// End row
	int32 end = std::clamp(r.max().y, 0, m_viewport.max().y);

	int32 _thickness = thickness - 1; // True thickness in pixels
	for (int32 row = start; row < end; row++)
	{
		// Pointer to the first pixel in this row with the X offset
		uint32* ptr = m_data->scanline(row) + r.min().x;

		// Fill the entire row if it's either the start or end row,
		// accounting for thickness
		if (row <= (start + thickness) || row >= (end - thickness))
		{
			std::fill(ptr, ptr + (int32)r.width + 1, value);
		}
		// Otherwise only set the pixels on the left and right,
		// accounting for thickness.
		else
		{
			for (int32 i = 0; i < thickness; i++)
			{
				*(ptr + i) = value;					   // Left-side
				*(ptr + ((int32)r.width - i)) = value; // Right-side
			}
		}
	}
#endif
}

void Painter::drawRectFilled(recti r, const Color& color)
{
	assertValid();

	// Color value as int32
	int32 value = color.toInt32();

	// Clamp the rectangle to the current viewport size
	r.clamp(m_viewport);

#if defined(_WIN32) || defined(_WIN64)
	// On Windows we have to flip the Y values because Win32 draws
	// from the bottom up, rather than top down.

	vec2i rmin = r.min();
	vec2i rmax = r.max();
	int32 height = m_viewport.height;

	// Start row
	int32 end = std::clamp(height - rmin.y - g_windowHeightClip, 0, m_viewport.max().y);
	// End row
	int32 start = std::clamp(height - rmax.y - g_windowHeightClip, 0, m_viewport.max().y);

	// Fill each row with the color
	for (int32 row = start; row < end; row++)
	{
		uint32* ptr = m_data->scanline(row) + r.min().x;
		std::fill(ptr, ptr + (int32)r.width, value);
	}
#else
	// Start row
	int32 start = std::clamp(r.min().y, 0, m_viewport.max().y);
	// End row
	int32 end = std::clamp(r.max().y, 0, m_viewport.max().y);

	// Fill each row with the color
	for (int32 row = start; row < end; row++)
	{
		auto ptr = m_data->scanline(row) + r.min().x;
		std::fill(ptr, ptr + (int32)r.width, value);
	}
#endif
}

void Painter::drawBezierCurve(std::vector<vec2i> points, const Color& color)
{
	assert(points.size() > 2); // Minimum of 3 points to draw a curve
	int32 stepCount = 25;
	float stepInterval = 1.0f / stepCount;

	for (auto& p : points)
	{
		p.x = std::clamp(p.x, 0, m_viewport.width);
		p.y = std::clamp(p.y, 0, m_viewport.height);
	}

	vec2i* pPoint = &points.front(); // Previous point
	vec2i* cPoint = pPoint;			 // Current point
	int32  limit = points.size() - 3;

	for (int32 i = 0; i <= limit; i++)
	{
		vec2i p0 = *cPoint;
		vec2i p1 = *(cPoint + 1);
		vec2i p2 = *(cPoint + 2);

		for (int32 step = 0; step <= stepCount; step++)
		{
			// Compute t value for this step
			float t = stepInterval * (float)step;

			// Lerp with points [0, 1], [1, 2], and then with [01, 12]
			vec2i p01 = Math::lerp(p0, p1, t);
			vec2i p12 = Math::lerp(p1, p2, t);
			vec2i p012 = Math::lerp(p01, p12, t);

			// Draw a line between the previous point and the current interpolated point
			// drawLine(*pPoint, p012, color);
			m_data->setPixelFromColor(p012.x, p012.y, color);

			// Store the most recently-interpolated point
			*pPoint = p012;
		}

		// Increment the current point ptr
		cPoint++;
	}
}

std::vector<vec2i> Painter::getWindings(GlyphShape* glyph)
{
	std::vector<vec2i> windings;
	return windings;
}

vec2i Painter::drawGlyph(GlyphShape* glyph, int32 byteOffset, const vec2f& scale, const vec2f& shift, float flatness, const Color& color)
{
	int32 windingCount = 0;
	std::vector<int32> windingLengths;
	std::vector<vec2f> points;

	// Tesselate and scale

	float fscale = scale.x > scale.y ? scale.y : scale.x;
	float flatScale = flatness / fscale;
	float flatnessSquared = flatScale * flatScale;
	int32 num_points = glyph->contourCount;
	int32 pass = 0;
	int32 n = 0;

	for (pass = 0; pass < 2; pass++)
	{
		float x = 0;
		float y = 0;

		// If we're on the first pass, resize the points array
		if (pass == 1)
		{
			points.resize(num_points);
		}

		num_points = 0;
		n = -1;
		for (int32 i = 0; i < num_points; i++)
		{

		}
	}


	if (points.size() != 0)
	{
		// Rasterize
	}
}

void Painter::drawText(const vec2i& pos, const std::string& text, const Color& color)
{
	int32 lineHeight = 64;
	assert(m_font != nullptr);

	// Get vertical metrics
	float scale = TTF::getScaleForPixelHeight(m_font, lineHeight);
	float ascent = m_font->hhea->ascender;
	float descent = m_font->hhea->descender;
	float lineGap = m_font->hhea->lineGap;

	ascent = std::roundf(ascent * scale);
	descent = std::roundf(descent * scale);

	// Track horizontal position
	int32 x = 0;

	// Draw each character
	for (auto c : text)
	{
		GlyphShape* glyph = &m_font->glyf->shapes[c];
		int32		glyphIndex = m_font->loca->glyphIndexes[c];

		// Get horizontal metrics for this glyph
		int32 advanceWidth = m_font->hmtx->hMetrics[glyphIndex].advanceWidth;
		int32 leftSideBearing = m_font->hmtx->hMetrics[glyphIndex].leftSideBearing;

		int32 x0 = std::floor(glyph->xMin * scale + pos.x);
		int32 y0 = std::floor(-glyph->yMax * scale + pos.y);
		int32 x1 = std::floor(glyph->xMax * scale + pos.x);
		int32 y1 = std::floor(-glyph->yMin * scale + pos.y);

		int32 y = ascent + y0;

		int32 byteOffset = x + roundf(leftSideBearing * scale) + (y * m_viewport.width);
		float flatness = 0.35f;
		drawGlyph(glyph, byteOffset, vec2f(scale, scale), vec2f(x1 - x0, y1 - y0), flatness, color);

		// Advance
		x += roundf(advanceWidth * scale);
	}
}
