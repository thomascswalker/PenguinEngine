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

std::vector<vec2i> Painter::getWindings(GlyphShape* glyph)
{
	std::vector<vec2i> windings;
	return windings;
}

vec2i Painter::drawGlyph(GlyphShape* glyph, const vec2f& scale, const vec2i& shift, const vec2i& offset, bool invert)
{
	// Tesselate the curves
	int32 pointCount = glyph->vertices.size();

	// Calculate and scale edges
	float xScale = scale.x;
	float yScale = invert ? -scale.y : scale.y;

	std::vector<GlyphEdge> edges;
	int32				   i = 0;

	int32 minX = 16384;
	int32 minY = 16384;
	int32 maxX = 0;
	int32 maxY = 0;
	for (auto& contour : glyph->contours)
	{
		for (int32 i = 0; i < contour.points.size(); i++)
		{
			auto  p0 = contour.points[i].position;
			int32 nextIndex = i + 1 == contour.points.size() ? 0 : i + 1;
			auto  p1 = contour.points[nextIndex].position;

			GlyphEdge edge;
			edge.v0.x = p0.x * xScale + shift.x + offset.x;
			edge.v0.y = p0.y * yScale + shift.y + offset.y;
			edge.v1.x = p1.x * xScale + shift.x + offset.x;
			edge.v1.y = p1.y * yScale + shift.y + offset.y;
			edges.emplace_back(edge);

			if (edge.v0.x > maxX) { maxX = edge.v0.x; } //
			if (edge.v0.y > maxY) { maxY = edge.v0.y; } //
			if (edge.v0.x < minX) { minX = edge.v0.x; } //
			if (edge.v0.y < minY) { minY = edge.v0.y; } //
			if (edge.v1.x > maxX) { maxX = edge.v1.x; } //
			if (edge.v1.y > maxY) { maxY = edge.v1.y; } //
			if (edge.v1.x < minX) { minX = edge.v1.x; } //
			if (edge.v1.y < minY) { minY = edge.v1.y; } //
		}
	}

	// Rasterize edges, from left to right alternating drawing when we hit a line
	bool on = false;
	for (int32 x = minX; x < maxX; x++)
	{
		for (int32 y = minY; y < maxY; y++)
		{
			//drawPoint(x, y, m_fontColor);
		}
	}

	for (int i = 0; i < edges.size(); i++)
	{
		auto e = edges[i];
		drawLine(e.v0, e.v1, Color::red());
	}

	return vec2i(maxX - minX, maxY - minY);
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
				auto  shift = vec2i(x1 - x0, y1 - y0);
				auto  offset = vec2i(x, y);

				// Only draw actual characters
				if (c != ' ')
				{
					offset = drawGlyph(glyph, vec2f(scale, -scale), shift, offset, false);
				}

				// Advance
				x += (float)advanceWidth * scale;
			}
		}
	}
}
