#pragma once

#include <vector>

#include "Core/Types.h"
#include "Math/Color.h"
#include "Math/Rect.h"
#include "Renderer/Texture.h"
#include <Renderer/Font.h>
#include <Renderer/FontTexture.h>

enum class EFontRenderMode : uint8
{
	System,
	Texture
};

class Painter
{
	Texture*		m_data = nullptr;
	recti			m_viewport{};
	FontInfo*		m_font = nullptr;
	int32			m_fontSize = 30;
	Color			m_fontColor = Color::white();
	EFontRenderMode m_glyphRenderMode = EFontRenderMode::System;

	void assertValid();

public:
	Painter(Texture* data, recti viewport);

	/** Getters & Setters **/

	void  setViewport(recti viewport) { m_viewport = viewport; }
	void  setFont(FontInfo* font) { m_font = font; }

	void  setFontSize(int32 fontSize) { m_fontSize = fontSize; }
	int32 getFontSize() const { return m_fontSize; }

	void setFontColor(const Color& color) { m_fontColor = color; }
	Color getFontColor() const { return m_fontColor; }

	/** Drawing **/

	void drawPoint(int32 x, int32 y, const Color& color);

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

	vec2i drawGlyph(GlyphShape* glyph, const vec2f& scale, const vec2i& shift, const vec2i& offset, bool invert);

	void drawGlyphTexture(const GlyphTexture* ft, const vec2i& pos);

	void drawText(const vec2i& pos, const std::string& text);
};