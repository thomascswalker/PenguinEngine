#pragma once

#include <vector>

#include "Core/Types.h"
#include "Math/Color.h"
#include "Math/Rect.h"
#include "Renderer/Texture.h"
#include "Renderer/Font.h"
#include "Renderer/FontTexture.h"


#include <ft2build.h>
#include FT_FREETYPE_H


enum class EFontRenderMode : uint8
{
	System,
	Texture,
	FreeType
};

inline const std::string g_defaultFontFamily = "Roboto";
inline const std::string g_defaultFontSubFamily = "Regular";
inline const int32		 g_defaultFontSize = 11;
inline const Color		 g_defaultFontColor = Color::white();

struct Character
{
	std::vector<uint8> buffer;
	vec2i			   size;
	int32			   yOffset = 0;
	int32			   advance = 0;
};

class Painter
{
	Texture*		m_texture = nullptr;
	recti			m_viewport{};
	FT_Face			m_face = nullptr;
	FT_Library		m_library = nullptr;
	std::string		m_fontFamily = g_defaultFontFamily;
	std::string		m_fontSubFamily = g_defaultFontSubFamily;
	int32			m_fontSize = g_defaultFontSize;
	Color			m_fontColor = g_defaultFontColor;
	EFontRenderMode m_glyphRenderMode = EFontRenderMode::FreeType;

	using CharacterMap = std::map<char, Character>;
	CharacterMap m_characters;

	void assertValid();

public:
	Painter(Texture* data, recti viewport);
	~Painter();

	/** Getters & Setters **/

	void setTexture(Texture* texture) { m_texture = texture; }

	void setViewport(recti viewport) { m_viewport = viewport; }

	void initFont();

	FT_Face getFont() const { return m_face; }
	void	setFont(FT_Face font) { m_face = font; }

	int32 getFontSize() const { return m_fontSize; }
	void  setFontSize(int32 fontSize);

	Color getFontColor() const { return m_fontColor; }
	void  setFontColor(const Color& color) { m_fontColor = color; }

	Character* getCharacter(const char c) { return &m_characters[c]; }

	/** Drawing **/

	void fill(const Color& color);

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