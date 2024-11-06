#pragma once

#include <vector>

#include "Core/Types.h"
#include "Math/Color.h"
#include "Math/Rect.h"
#include "Renderer/Texture.h"
#include "Renderer/Font.h"
#include "Renderer/FontTexture.h"

enum class EFontRenderMode : uint8
{
	System,
	Texture
};

inline const std::string g_defaultFontFamily = "Segoe UI";
inline const std::string g_defaultFontSubFamily = "Normal";
inline const int32		 g_defaultFontSize = 12;
inline const Color		 g_defaultFontColor = Color::white();

class Painter
{
	Texture*		m_data = nullptr;
	recti			m_viewport{};
	FontInfo*		m_font = nullptr;
	std::string		m_fontFamily = g_defaultFontFamily;
	std::string		m_fontSubFamily = g_defaultFontSubFamily;
	int32			m_fontSize = g_defaultFontSize;
	Color			m_fontColor = g_defaultFontColor;
	EFontRenderMode m_glyphRenderMode = EFontRenderMode::System;

	void assertValid();

public:
	Painter(Texture* data, recti viewport);

	/** Getters & Setters **/

	void setViewport(recti viewport) { m_viewport = viewport; }

	FontInfo* getFont() const { return m_font; }
	void setFont(FontInfo* font) { m_font = font; }

	int32	  getFontSize() const { return m_fontSize; }
	void  setFontSize(int32 fontSize) { m_fontSize = fontSize; }
	
	Color getFontColor() const { return m_fontColor; }
	void  setFontColor(const Color& color) { m_fontColor = color; }
	

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

	void drawTriangle(const vec2i& v0, const vec2i& v1, const vec2i& v2, const Color& color);

	std::vector<GlyphEdge> sortEdges(std::vector<GlyphEdge>& edges);

	void drawGlyph(GlyphShape* glyph, float scale, const vec2i& shift, const vec2i& offset, bool invert);

	void drawGlyphTexture(const GlyphTexture* ft, const vec2i& pos);

	void drawText(const vec2i& pos, const std::string& text);
};