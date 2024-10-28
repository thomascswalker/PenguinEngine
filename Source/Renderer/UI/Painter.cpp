#include <cassert>

#include "Painter.h"
#include "Engine/Actors/Camera.h"

inline std::string g_defaultFont = "Arial";
constexpr int32	   g_scaleFactor = 150;

void Painter::assertValid()
{
	assert(m_data != nullptr);
}

Painter::Painter(Texture* data, recti viewport) : m_data(data), m_viewport(viewport)
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
}

void Painter::drawRectFilled(recti r, const Color& color)
{
	assertValid();

	// Color value as int32
	int32 value = color.toInt32();

	// Clamp the rectangle to the current viewport size
	r.clamp(m_viewport);

	// Start row
	int32 start = std::clamp(r.min().y, 0, m_viewport.max().y);
	// End row
	int32 end = std::clamp(r.max().y, 0, m_viewport.max().y);

	// Fill each row with the color
	if (color.a == 255) {
		for (int32 row = start; row < end; row++)
		{
			auto ptr = m_data->scanline(row) + r.min().x;
			std::fill(ptr, ptr + (int32)r.width, value);
		}
	}
	else if (color.a < 255 && color.a > 0)
	{
		float perc = (float)color.a / 255.0f;
		for (int32 y = start; y < end; y++)
		{
			auto ptr = m_data->scanline(y) + r.min().x;
			for (int32 x = r.min().x; x < r.max().x; x++)
			{
				auto currentColor = m_data->getPixelAsColor(x, y);
				currentColor.r = Math::lerp(currentColor.r, color.r, perc);
				currentColor.g = Math::lerp(currentColor.g, color.g, perc);
				currentColor.b = Math::lerp(currentColor.b, color.b, perc);
				m_data->setPixelFromColor(x, y, currentColor);
			}
		}
	}
	else
	{
		// do nothing
	}

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

void Painter::drawGlyph(GlyphShape* glyph, const vec2f& scale, const vec2i& shift, const vec2i& offset, const Color& color)
{
	std::vector<vec2i> points = TTF::tessellateGlyph(glyph);

	for (int32 i = 0; i < points.size() - 1; i++)
	{
		vec2i p0 = points[i];
		vec2i p1 = i == points.size() - 1 ? points[0] : points[i + 1];

		p0.x = (float)p0.x * scale.x;
		p0.y = (float)p0.y * scale.y;

		p1.x = (float)p1.x * scale.x;
		p1.y = (float)p1.y * scale.y;

		p0 += shift + offset;
		p1 += shift + offset;

		drawLine(p0, p1, color);
	}
}

void Painter::drawGlyphTexture(const GlyphTexture* ft, const vec2i& pos, const Color& color)
{
	int32		maxX = pos.x + g_glyphTextureWidth;
	int32		maxY = pos.y + g_glyphTextureHeight;
	const char* ptr = ft->data;
	for (int y = pos.y; y < maxY; y++)
	{
		for (int x = pos.x; x < maxX; x++)
		{
			int32 v = *ptr++;
			if (v)
			{
				m_data->setPixelFromColor(x + 1, y + 1, Color::black()); // Outline
				m_data->setPixelFromColor(x, y, color);					 // Actual text color
			}
		}
	}
}

void Painter::drawText(const vec2i& pos, const std::string& text)
{
	if (m_glyphRenderMode == EFontRenderMode::Texture)
	{
		int32 totalWidth = text.size() * g_glyphTextureWidth;

		int x = pos.x;
		int y = pos.y;
		for (auto c : text)
		{
			if (c == ' ')
			{
				x += g_glyphTextureWidth;
				continue;
			}
			const GlyphTexture* glyphTexture = g_glyphTextureMap[c];
			y += glyphTexture->descent;
			drawGlyphTexture(glyphTexture, { x, y }, m_fontColor);
			x += g_glyphTextureWidth;
			y += glyphTexture->ascent;
		}
		return;
	}

	assert(m_font != nullptr);

	// Get vertical metrics
	float scale = TTF::getScaleForPixelHeight(m_font, m_fontSize);
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

		int32 x0 = std::floor(glyph->bounds.min().x * scale + pos.x);
		int32 y0 = std::floor(-glyph->bounds.max().y * scale + pos.y);
		int32 x1 = std::floor(glyph->bounds.max().x * scale + pos.x);
		int32 y1 = std::floor(-glyph->bounds.min().y * scale + pos.y);

		int32 y = ascent + y0;

		// Only draw actual characters
		if (c != ' ')
		{
			drawGlyph(glyph, vec2f(scale, -scale), vec2i(x1 - x0, y1 - y0), vec2i(x, y), m_fontColor);
		}

		// Advance
		x += roundf((float)advanceWidth * scale);
	}
}
