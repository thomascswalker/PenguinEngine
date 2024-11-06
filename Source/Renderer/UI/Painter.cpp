#include <cassert>

#include "Painter.h"
#include "Engine/Actors/Camera.h"
#include "Engine/Mesh.h"

void Painter::assertValid()
{
	assert(m_data != nullptr);
}

Painter::Painter(Texture* data, recti viewport) : m_data(data), m_viewport(viewport)
{
	setFont(g_fontDatabase->getFontInfo(m_fontFamily, m_fontSubFamily));
}

void Painter::drawPoint(int32 x, int32 y, const Color& color)
{
	m_data->setPixelFromColor(x, y, color);
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
	if (color.a == 255)
	{
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
				Color currentColor = m_data->getPixelAsColor(x, y);
				currentColor = Color::blend(currentColor, color, EBlendMode::Normal, perc);
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

inline void Painter::drawTriangle(const vec2i& v0, const vec2i& v1, const vec2i& v2, const Color& color)
{
	int32 minX = std::min({ v0.x, v1.x, v2.x });
	int32 minY = std::min({ v0.y, v1.y, v2.y });
	int32 maxX = std::max({ v0.x, v1.x, v2.x });
	int32 maxY = std::max({ v0.y, v1.y, v2.y });

	vec2f v0f = v0.toType<float>() + 0.5f;
	vec2f v1f = v1.toType<float>() + 0.5f;
	vec2f v2f = v2.toType<float>() + 0.5f;

#ifdef PENG_ANTIALIASING
	int32 sampleCount = 2;
	int32 sampleSquared = sampleCount * sampleCount;
	float halfSample = 1.0f / sampleCount;
#endif

	for (int32 x = minX; x < maxX; x++)
	{
		for (int32 y = minY; y < maxY; y++)
		{
			// Check pixel is in viewport
			if (x < m_viewport.x || y < m_viewport.y || x > m_viewport.x + m_viewport.width || y > m_viewport.y + m_viewport.height)
			{
				continue;
			}
			if (Math::isInTriangle(v0f, v1f, v2f, vec2f{ x + 0.5f, y + 0.5f }))
			{
				drawPoint(x, y, m_fontColor);
				continue;
			}

#ifdef PENG_ANTIALIASING
			// Subpixel Antialiasing
			float sum = 0.0f;
			for (int32 i = 0; i < sampleCount; i++)
			{
				for (int32 j = 0; j < sampleCount; j++)
				{
					float offset = halfSample * i;
					vec2f p = { (float)x + offset, (float)y + offset };
					if (Math::isInTriangle(v0f, v1f, v2f, p))
					{
						sum += halfSample;
					}
				}
			}

			if (sum == 0.0f)
			{
				continue;
			}

			drawPoint(x, y, m_fontColor);
			Color currentColor = m_data->getPixelAsColor(x, y);
			drawPoint(x, y, Color::blend(currentColor, m_fontColor, EBlendMode::Normal, sum));
#endif
		}
	}

	// drawLine(v0, v1, color);
	// drawLine(v1, v2, color);
	// drawLine(v2, v0, color);
}

/** Sort edges in vertical order, from highest vertical point to lowest. **/
std::vector<GlyphEdge> Painter::sortEdges(std::vector<GlyphEdge>& edges)
{
	auto a = edges;
	if (a.begin() == a.end())
	{
		return a;
	}

	for (auto i = a.begin() + 1; i < a.end(); ++i)
	{
		auto k = *i;
		auto j = i - 1;
		while (j >= a.begin() && *j < k)
		{
			*(j + 1) = *j;
			if (j != a.begin())
			{
				j--;
			}
			else
			{
				break;
			}
		}
		*(j + 1) = k;
	}
	return a;
}

void Painter::drawGlyph(GlyphShape* glyph, float scale, const vec2i& shift, const vec2i& offset, bool invert)
{
	// Tesselate the curves
	int32 pointCount = glyph->points.size();

	// Calculate and scale edges
	float xScale = scale;
	float yScale = invert ? -scale : scale;

	// Build list of points
	std::vector<vec2i> points = glyph->getPositions();

	// Draw each triangle
	for (int32 i = 0; i < glyph->indexes.size(); i++)
	{
		// Get each point of the triangle
		vec2i v0 = points[glyph->indexes[i].a];
		vec2i v1 = points[glyph->indexes[i].b];
		vec2i v2 = points[glyph->indexes[i].c];

		// Scale each point
		v0.x = v0.x * xScale + shift.x + offset.x;
		v0.y = v0.y * yScale + shift.y + offset.y;
		v1.x = v1.x * xScale + shift.x + offset.x;
		v1.y = v1.y * yScale + shift.y + offset.y;
		v2.x = v2.x * xScale + shift.x + offset.x;
		v2.y = v2.y * yScale + shift.y + offset.y;

		drawTriangle(v0, v1, v2, m_fontColor);
	}
}

void Painter::drawGlyphTexture(const GlyphTexture* ft, const vec2i& pos)
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
				m_data->setPixelFromColor(x, y, m_fontColor);			 // Actual text color
			}
		}
	}
}

void Painter::drawText(const vec2i& pos, const std::string& text)
{
	switch (m_glyphRenderMode)
	{
		case EFontRenderMode::Texture:
		{
			int32 x = pos.x;
			for (auto c : text)
			{
				const GlyphTexture* glyphTexture = g_glyphTextureMap[c];
				int32				y = pos.y + glyphTexture->descent;
				drawGlyphTexture(glyphTexture, vec2i(x, y));
				x += g_glyphTextureWidth;
			}
			return;
		}
		case EFontRenderMode::System:
		{
			assert(m_font != nullptr);

			// Compute the font scale from the current font size (height in pixels)
			float scale = (1.0f / m_font->head->unitsPerEm) * m_fontSize;
			int32 letterAdvance = 0;
			int32 wordAdvance = 0;
			int32 lineAdvance = 0;

			for (auto c : text)
			{
				if (c == ' ')
				{
					wordAdvance += 0.33333f * m_fontSize; // hardcoded
				}
				else if (c == '\n')
				{
					lineAdvance += 1.3f * m_fontSize; // hardcoded
					wordAdvance = 0.0f;
					letterAdvance = 0.0f;
				}
				else
				{
					GlyphShape* glyph = &m_font->glyphs[c];

					// Start position of the whole text block
					vec2i screenOffset = pos;

					// Start position of this glyph specifically
					vec2i localOffset;
					localOffset.x = (glyph->minX * scale) + letterAdvance;
					localOffset.y = (-glyph->minY * scale) + lineAdvance;

					// Draw the glyph
					drawGlyph(glyph, scale, screenOffset, localOffset, false);

					// Advance the position of the next glyph
					letterAdvance += glyph->advanceWidth * scale;
				}
			}
		}
	}
}
