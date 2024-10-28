#pragma once

#include <vector>

#include "Core/Types.h"
#include "Math/Color.h"
#include "Math/Rect.h"
#include "Renderer/Texture.h"
#include <Renderer/Font.h>

class Painter
{
	Texture*  m_data = nullptr;
	recti	  m_viewport{};
	FontInfo* m_font = nullptr;
	int32	  m_fontSize = 12;

	void assertValid();

public:
	Painter(Texture* data, recti viewport);

	/** Getters & Setters **/

	void setViewport(recti viewport) { m_viewport = viewport; }
	void setFont(FontInfo* font) { m_font = font; }
	void setFontSize(int32 fontSize) { m_fontSize; }
	int32 getFontSize() const { return m_fontSize; }

	/** Drawing **/

	/**
	 * @brief Draws a line from point A to point B with the specified color.
	 */
	void drawLine(vec2i a, vec2i b, const Color& color);

	/**
	 * @brief Draws a rectangle with no fill color.
	 */
	void drawRect(recti r, const Color& color, int32 thickness = 1);

	/**
	 * Draws a rectangle filled with the specified color.
	 */
	void drawRectFilled(recti r, const Color& color);

	/**
	 * @brief Draws a bezier curve along all points with the specified color.
	 */
	void drawBezierCurve(std::vector<vec2i> points, const Color& color);

	std::vector<vec2i> getWindings(GlyphShape* glyph);
	void			   drawGlyph(GlyphShape* glyph, const vec2f& scale, const vec2i& shift, const vec2i& offset, const Color& color);
	void drawText(vec2i pos, const std::string& text, const Color& color);
};