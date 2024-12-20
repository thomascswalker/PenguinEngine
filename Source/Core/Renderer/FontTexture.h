#pragma once

#include <map>

constexpr int	g_glyphTextureDataSize = 96; // All chars are 128 bytes (12 row * 8 columns)
constexpr int	g_glyphTextureWidth = 8;
constexpr int	g_glyphTextureHeight = 12;
constexpr float g_glyphTextureAspect = 0.6666667f;

struct GlyphTexture
{
	char		c;
	const char* data;
	int			ascent;
	int			descent;

	GlyphTexture(const char* inC, const char* inData, int ascent) : c(inC[0]), data(inData), ascent(-ascent), descent(ascent) {}
};
#define DECLARE_GLYPH_TEXTURE(c, ascent) inline const GlyphTexture g_ft_##c(#c, g_glyph_##c, ascent)

// https://www.emigre.com/Fonts/Oblong

inline const char g_glyph_a[] = {

	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 1, 1, 1, 1, 1, 0, 0, //
	0, 0, 0, 0, 0, 0, 1, 0, //
	0, 0, 0, 0, 0, 0, 1, 0, //
	0, 0, 1, 1, 1, 1, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 0, 1, 1, 1, 1, 1, 1, // <<<
};
DECLARE_GLYPH_TEXTURE(a, 0);

inline const char g_glyph_b[] = {
	0, 1, 1, 0, 0, 0, 0, 0, //
	0, 0, 1, 0, 0, 0, 0, 0, //
	0, 0, 1, 0, 0, 0, 0, 0, //
	0, 0, 1, 0, 1, 1, 1, 0, //
	0, 0, 1, 1, 0, 0, 0, 1, //
	0, 0, 1, 0, 0, 0, 0, 1, //
	0, 0, 1, 0, 0, 0, 0, 1, //
	0, 0, 1, 0, 0, 0, 0, 1, //
	0, 0, 1, 0, 0, 0, 0, 1, //
	0, 0, 1, 0, 0, 0, 0, 1, //
	0, 0, 1, 0, 0, 0, 0, 1, //
	0, 1, 1, 1, 1, 1, 1, 0, // <<<
};
DECLARE_GLYPH_TEXTURE(b, 0);

inline const char g_glyph_c[] = {
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 1, 1, 1, 1, 0, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 0, 0, //
	0, 1, 0, 0, 0, 0, 0, 0, //
	0, 1, 0, 0, 0, 0, 0, 0, //
	0, 1, 0, 0, 0, 0, 0, 0, //
	0, 1, 0, 0, 0, 0, 0, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 0, 1, 1, 1, 1, 0, 0, // <<<
};
DECLARE_GLYPH_TEXTURE(c, 0);

inline const char g_glyph_d[] = {
	0, 0, 0, 0, 0, 1, 0, 0, //
	0, 0, 0, 0, 0, 1, 0, 0, //
	0, 0, 0, 0, 0, 1, 0, 0, //
	0, 1, 1, 1, 0, 1, 0, 0, //
	1, 0, 0, 0, 1, 1, 0, 0, //
	1, 0, 0, 0, 0, 1, 0, 0, //
	1, 0, 0, 0, 0, 1, 0, 0, //
	1, 0, 0, 0, 0, 1, 0, 0, //
	1, 0, 0, 0, 0, 1, 0, 0, //
	1, 0, 0, 0, 0, 1, 0, 0, //
	1, 0, 0, 0, 0, 1, 0, 0, //
	0, 1, 1, 1, 1, 1, 1, 0, // <<<
};
DECLARE_GLYPH_TEXTURE(d, 0);

inline const char g_glyph_e[] = {
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 1, 1, 1, 1, 0, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 1, 1, 1, 1, 0, 0, //
	0, 1, 0, 0, 0, 0, 0, 0, //
	0, 1, 0, 0, 0, 0, 0, 0, //
	0, 1, 0, 0, 0, 0, 0, 0, //
	0, 0, 1, 1, 1, 1, 1, 0, // <<<
};
DECLARE_GLYPH_TEXTURE(e, 0);

inline const char g_glyph_f[] = {
	0, 0, 0, 1, 1, 1, 0, 0, //
	0, 0, 1, 0, 0, 0, 1, 0, //
	0, 0, 1, 0, 0, 0, 0, 0, //
	0, 0, 1, 0, 0, 0, 0, 0, //
	0, 1, 1, 1, 1, 1, 1, 0, //
	0, 0, 1, 0, 0, 0, 0, 0, //
	0, 0, 1, 0, 0, 0, 0, 0, //
	0, 0, 1, 0, 0, 0, 0, 0, //
	0, 0, 1, 0, 0, 0, 0, 0, //
	0, 0, 1, 0, 0, 0, 0, 0, //
	0, 0, 1, 0, 0, 0, 0, 0, //
	0, 1, 1, 1, 1, 0, 0, 0, // <<<
};
DECLARE_GLYPH_TEXTURE(f, 0);

inline const char g_glyph_g[] = {
	0, 0, 1, 1, 1, 1, 0, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 1, 1, 0, // <<<
	0, 0, 1, 1, 1, 0, 1, 0, //
	0, 0, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 0, 1, 1, 1, 1, 0, 0, //
};
DECLARE_GLYPH_TEXTURE(g, 3);

inline const char g_glyph_h[] = {
	0, 1, 1, 0, 0, 0, 0, 0, //
	0, 0, 1, 0, 0, 0, 0, 0, //
	0, 0, 1, 0, 0, 0, 0, 0, //
	0, 0, 1, 0, 1, 1, 0, 0, //
	0, 0, 1, 1, 0, 0, 1, 0, //
	0, 0, 1, 0, 0, 0, 1, 0, //
	0, 0, 1, 0, 0, 0, 1, 0, //
	0, 0, 1, 0, 0, 0, 1, 0, //
	0, 0, 1, 0, 0, 0, 1, 0, //
	0, 0, 1, 0, 0, 0, 1, 0, //
	0, 0, 1, 0, 0, 0, 1, 0, //
	0, 1, 1, 0, 0, 0, 1, 0, // <<<
};
DECLARE_GLYPH_TEXTURE(h, 0);

inline const char g_glyph_i[] = {
	0, 0, 0, 0, 1, 0, 0, 0, //
	0, 0, 0, 0, 1, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 1, 1, 1, 0, 0, 0, //
	0, 0, 0, 0, 1, 0, 0, 0, //
	0, 0, 0, 0, 1, 0, 0, 0, //
	0, 0, 0, 0, 1, 0, 0, 0, //
	0, 0, 0, 0, 1, 0, 0, 0, //
	0, 0, 0, 0, 1, 0, 0, 0, //
	0, 0, 0, 0, 1, 0, 0, 0, //
	0, 0, 0, 0, 1, 0, 0, 0, //
	0, 0, 1, 1, 1, 1, 0, 0, //
};
DECLARE_GLYPH_TEXTURE(i, 0);

inline const char g_glyph_j[] = {
	0, 0, 0, 0, 1, 0, 0, 0, //
	0, 0, 0, 0, 1, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 1, 1, 0, 0, 0, //
	0, 0, 0, 0, 1, 0, 0, 0, //
	0, 0, 0, 0, 1, 0, 0, 0, //
	0, 0, 0, 0, 1, 0, 0, 0, //
	0, 0, 0, 0, 1, 0, 0, 0, //
	0, 0, 0, 0, 1, 0, 0, 0, //
	0, 0, 0, 0, 1, 0, 0, 0, //
	1, 0, 0, 0, 1, 0, 0, 0, //
	0, 1, 1, 1, 0, 0, 0, 0, //
};
DECLARE_GLYPH_TEXTURE(j, 5);

inline const char g_glyph_k[] = {
	1, 1, 0, 0, 0, 0, 0, 0, //
	0, 1, 0, 0, 0, 0, 0, 0, //
	0, 1, 0, 0, 0, 0, 0, 0, //
	0, 1, 0, 0, 0, 0, 0, 0, //
	0, 1, 0, 0, 0, 1, 0, 0, //
	0, 1, 0, 0, 1, 0, 0, 0, //
	0, 1, 0, 1, 0, 0, 0, 0, //
	0, 1, 1, 0, 0, 0, 0, 0, //
	0, 1, 1, 0, 0, 0, 0, 0, //
	0, 1, 0, 1, 0, 0, 0, 0, //
	0, 1, 0, 0, 1, 0, 0, 0, //
	1, 1, 0, 0, 0, 1, 0, 0, //
};
DECLARE_GLYPH_TEXTURE(k, 0);

inline const char g_glyph_l[] = {
	1, 1, 1, 0, 0, 0, 0, 0, //
	0, 0, 1, 0, 0, 0, 0, 0, //
	0, 0, 1, 0, 0, 0, 0, 0, //
	0, 0, 1, 0, 0, 0, 0, 0, //
	0, 0, 1, 0, 0, 0, 0, 0, //
	0, 0, 1, 0, 0, 0, 0, 0, //
	0, 0, 1, 0, 0, 0, 0, 0, //
	0, 0, 1, 0, 0, 0, 0, 0, //
	0, 0, 1, 0, 0, 0, 0, 0, //
	0, 0, 1, 0, 0, 0, 0, 0, //
	0, 0, 1, 0, 0, 0, 1, 0, //
	0, 0, 0, 1, 1, 1, 0, 0, //
};
DECLARE_GLYPH_TEXTURE(l, 0);

inline const char g_glyph_m[] = {
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 1, 1, 0, 1, 1, 0, 0, //
	1, 0, 0, 1, 0, 0, 1, 0, //
	1, 0, 0, 1, 0, 0, 1, 0, //
	1, 0, 0, 1, 0, 0, 1, 0, //
	1, 0, 0, 1, 0, 0, 1, 0, //
	1, 0, 0, 1, 0, 0, 1, 0, //
	1, 0, 0, 1, 0, 0, 1, 0, //
	1, 0, 0, 1, 0, 0, 1, 0, //
	1, 0, 0, 1, 0, 0, 1, 0, //
};
DECLARE_GLYPH_TEXTURE(m, 0);

inline const char g_glyph_n[] = {
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 1, 1, 1, 1, 1, 0, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
};
DECLARE_GLYPH_TEXTURE(n, 0);

inline const char g_glyph_o[] = {
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 1, 1, 1, 1, 0, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 0, 1, 1, 1, 1, 0, 0, //
};
DECLARE_GLYPH_TEXTURE(o, 0);

inline const char g_glyph_p[] = {
	0, 0, 1, 1, 1, 1, 0, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 1, 0, 0, 0, 1, 0, //
	0, 1, 0, 1, 1, 1, 0, 0, // <<<
	0, 1, 0, 0, 0, 0, 0, 0, //
	0, 1, 0, 0, 0, 0, 0, 0, //
	1, 1, 0, 0, 0, 0, 0, 0, //
};
DECLARE_GLYPH_TEXTURE(p, 3);

inline const char g_glyph_q[] = {
	0, 0, 1, 1, 1, 1, 0, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 1, 1, 0, //
	0, 0, 1, 1, 1, 0, 1, 0, //
	0, 0, 0, 0, 0, 0, 1, 0, //
	0, 0, 0, 0, 0, 0, 1, 0, //
	0, 0, 0, 0, 0, 1, 1, 1, //
};
DECLARE_GLYPH_TEXTURE(q, 3);

inline const char g_glyph_r[] = {
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	1, 0, 1, 1, 1, 1, 0, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 0, 0, //
	0, 1, 0, 0, 0, 0, 0, 0, //
	0, 1, 0, 0, 0, 0, 0, 0, //
	0, 1, 0, 0, 0, 0, 0, 0, //
	0, 1, 0, 0, 0, 0, 0, 0, //
	1, 1, 1, 0, 0, 0, 0, 0, //
};
DECLARE_GLYPH_TEXTURE(r, 0);

inline const char g_glyph_s[] = {
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 1, 1, 1, 1, 0, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 0, 0, //
	0, 1, 0, 0, 0, 0, 0, 0, //
	0, 0, 1, 1, 1, 1, 0, 0, //
	0, 0, 0, 0, 0, 0, 1, 0, //
	0, 0, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 0, 1, 1, 1, 1, 0, 0, //
};
DECLARE_GLYPH_TEXTURE(s, 0);

inline const char g_glyph_t[] = {
	0, 0, 0, 1, 0, 0, 0, 0, //
	0, 0, 0, 1, 0, 0, 0, 0, //
	0, 0, 0, 1, 0, 0, 0, 0, //
	0, 1, 1, 1, 1, 1, 1, 0, //
	0, 0, 0, 1, 0, 0, 0, 0, //
	0, 0, 0, 1, 0, 0, 0, 0, //
	0, 0, 0, 1, 0, 0, 0, 0, //
	0, 0, 0, 1, 0, 0, 0, 0, //
	0, 0, 0, 1, 0, 0, 0, 0, //
	0, 0, 0, 1, 0, 0, 0, 0, //
	0, 0, 0, 1, 0, 0, 1, 0, //
	0, 0, 0, 0, 1, 1, 0, 0, //
};
DECLARE_GLYPH_TEXTURE(t, 0);

inline const char g_glyph_u[] = {
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 1, 1, 0, //
	0, 0, 1, 1, 1, 0, 1, 1, //
};
DECLARE_GLYPH_TEXTURE(u, 0);

inline const char g_glyph_v[] = {
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 0, 1, 0, 0, 1, 0, 0, //
	0, 0, 1, 0, 0, 1, 0, 0, //
	0, 0, 1, 0, 0, 1, 0, 0, //
	0, 0, 1, 0, 0, 1, 0, 0, //
	0, 0, 0, 1, 1, 0, 0, 0, //
};
DECLARE_GLYPH_TEXTURE(v, 0);

inline const char g_glyph_w[] = {
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	1, 0, 0, 1, 0, 0, 1, 0, //
	1, 0, 0, 1, 0, 0, 1, 0, //
	1, 0, 0, 1, 0, 0, 1, 0, //
	1, 0, 0, 1, 0, 0, 1, 0, //
	1, 0, 0, 1, 0, 0, 1, 0, //
	1, 0, 0, 1, 0, 0, 1, 0, //
	1, 0, 0, 1, 0, 0, 1, 0, //
	1, 0, 0, 1, 0, 0, 1, 0, //
	0, 1, 1, 0, 1, 1, 1, 0, //
};
DECLARE_GLYPH_TEXTURE(w, 0);

inline const char g_glyph_x[] = {
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 0, 1, 0, 0, 1, 0, 0, //
	0, 0, 1, 0, 0, 1, 0, 0, //
	0, 0, 0, 1, 1, 0, 0, 0, //
	0, 0, 0, 1, 1, 0, 0, 0, //
	0, 0, 0, 1, 1, 0, 0, 0, //
	0, 0, 1, 0, 0, 1, 0, 0, //
	0, 0, 1, 0, 0, 1, 0, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
};
DECLARE_GLYPH_TEXTURE(x, 0);

inline const char g_glyph_y[] = {
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 1, 1, 0, //
	0, 0, 1, 1, 1, 0, 1, 0, //
	0, 0, 0, 0, 0, 0, 1, 0, //
	0, 1, 0, 0, 0, 0, 1, 0, //
	0, 0, 1, 1, 1, 1, 0, 0, //
};
DECLARE_GLYPH_TEXTURE(y, 3);

inline const char g_glyph_z[] = {
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 1, 1, 1, 1, 1, 1, 0, //
	0, 0, 0, 0, 0, 0, 1, 0, //
	0, 0, 0, 0, 0, 1, 0, 0, //
	0, 0, 0, 0, 1, 0, 0, 0, //
	0, 0, 0, 1, 1, 0, 0, 0, //
	0, 0, 0, 1, 0, 0, 0, 0, //
	0, 0, 1, 0, 0, 0, 0, 0, //
	0, 1, 0, 0, 0, 0, 0, 0, //
	0, 1, 1, 1, 1, 1, 1, 0, //
};
DECLARE_GLYPH_TEXTURE(z, 0);

inline const char g_glyph_space[] = {
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
	0, 0, 0, 0, 0, 0, 0, 0, //
};
inline const GlyphTexture g_ft_space(" ", g_glyph_space, 0);

inline std::map<char, const GlyphTexture*> g_glyphTextureMap{
	{ 'a', &g_ft_a },	 //
	{ 'b', &g_ft_b },	 //
	{ 'c', &g_ft_c },	 //
	{ 'd', &g_ft_d },	 //
	{ 'e', &g_ft_e },	 //
	{ 'f', &g_ft_f },	 //
	{ 'g', &g_ft_g },	 //
	{ 'h', &g_ft_h },	 //
	{ 'i', &g_ft_i },	 //
	{ 'j', &g_ft_j },	 //
	{ 'k', &g_ft_k },	 //
	{ 'l', &g_ft_l },	 //
	{ 'm', &g_ft_m },	 //
	{ 'n', &g_ft_n },	 //
	{ 'o', &g_ft_o },	 //
	{ 'p', &g_ft_p },	 //
	{ 'q', &g_ft_q },	 //
	{ 'r', &g_ft_r },	 //
	{ 's', &g_ft_s },	 //
	{ 't', &g_ft_t },	 //
	{ 'u', &g_ft_u },	 //
	{ 'v', &g_ft_v },	 //
	{ 'w', &g_ft_w },	 //
	{ 'x', &g_ft_x },	 //
	{ 'y', &g_ft_y },	 //
	{ 'z', &g_ft_z },	 //
	{ ' ', &g_ft_space } //
};