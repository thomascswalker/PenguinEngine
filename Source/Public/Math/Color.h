#pragma once

#include <format>
#include "MathFwd.h"

struct Color;
struct LinearColor;

inline int32 g_redMask = 0xFF000000;
inline int32 g_greenMask = 0xFF0000;
inline int32 g_blueMask = 0xFF00;
inline int32 g_alphaMask = 0xFF;

struct Color
{
	union
	{
		struct // NOLINT
		{
			uint8 r;
			uint8 g;
			uint8 b;
			uint8 a;
		};
		uint8 rgba[4] = { 0, 0, 0, 0 };
	};

protected:
	Color() {}
	Color(const uint8 inR, const uint8 inG, const uint8 inB, const uint8 inA = 255)
	{
		r = inR;
		g = inG;
		b = inB;
		a = inA;
	}

public:
	static Color red()
	{
		return fromRgba(255, 0, 0);
	}

	static Color green()
	{
		return fromRgba(0, 255, 0);
	}

	static Color blue()
	{
		return fromRgba(0, 0, 255);
	}

	static Color yellow()
	{
		return fromRgba(255, 255, 0);
	}

	static Color magenta()
	{
		return fromRgba(255, 0, 255);
	}

	static Color cyan()
	{
		return fromRgba(0, 255, 255);
	}

	static Color white()
	{
		return fromRgba(255, 255, 255);
	}

	static Color gray()
	{
		return fromRgba(128, 128, 128);
	}

	static Color black()
	{
		return fromRgba(0, 0, 0);
	}

	static Color fromRgba(uint8 r, uint8 g, uint8 b, uint8 a = 255)
	{
		return { r, g, b, a };
	}

	static Color fromUInt32(const int32 value)
	{
		uint8 a = UINT8_MAX & value >> 24;
		uint8 r = UINT8_MAX & value >> 16;
		uint8 g = UINT8_MAX & value >> 8;
		uint8 b = UINT8_MAX & value;
		return { r, g, b, a };
	}

	[[nodiscard]] int32 toInt32() const
	{
		return (r << 16) | (g << 8) | b | 0;
	}

	std::string toString()
	{
		return std::format("RGBA[{}, {}, {}, {}]", r, g, b, a);
	}
};

struct LinearColor
{
	union
	{
		struct
		{
			float r;
			float g;
			float b;
			float a;
		};

		float rgba[4];
	};

	static LinearColor fromRgba(const float r, const float g, const float b, const float a = 1.0f)
	{
		LinearColor color;
		color.r = r;
		color.g = g;
		color.b = b;
		color.a = a;
		return color;
	}

	void clamp()
	{
		for (float& v : rgba)
		{
			v = v > 1.0f ? 1.0f : v;
		}
	}
};
