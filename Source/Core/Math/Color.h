﻿#pragma once

#include <cassert>
#include <format>
#include <random>
#include <string>

#include "Core/Logging.h"
#include "Core/String.h"

struct Color;
struct LinearColor;

inline int32 g_redMask = 0xFF000000;
inline int32 g_greenMask = 0xFF0000;
inline int32 g_blueMask = 0xFF00;
inline int32 g_alphaMask = 0xFF;

enum class EBlendMode
{
	Normal,
	Multiply
};

struct Color
{
private:
	static Color lerpColors(const Color& a, const Color& b, float t)
	{
		Color out;
		out.r = (int8)((float)a.r * (1.0f - t) + ((float)b.r * t));
		out.g = (int8)((float)a.g * (1.0f - t) + ((float)b.g * t));
		out.b = (int8)((float)a.b * (1.0f - t) + ((float)b.b * t));
		return out;
	}

public:
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

	Color() {}

	Color(const uint8 inR, const uint8 inG, const uint8 inB, const uint8 inA = 255)
	{
		r = inR;
		g = inG;
		b = inB;
		a = inA;
	}

	Color(const std::string& hex)
	{
		assert(hex.starts_with("#"));
		std::string				 tmp = hex.substr(1, 6);
		std::vector<std::string> result;
		Strings::split(tmp, result, 2);

		r = std::stoi(result[0], nullptr, 16);
		g = std::stoi(result[1], nullptr, 16);
		b = std::stoi(result[2], nullptr, 16);
		a = 255;
	}

	static Color blend(const Color& a, const Color& b, EBlendMode mode, float p)
	{
		Color out;
		switch (mode)
		{
			case EBlendMode::Normal:
			{
				out = Color::lerpColors(a, b, p);
				break;
			}
			case EBlendMode::Multiply:
			{
				out = a * b;
				break;
			}
			default:
			{
				LOG_WARNING("Blend mode {} is not implemented.", (uint8)mode)
				break;
			}
		}

		return out;
	}

	static Color red() { return fromRgba(255, 0, 0); }

	static Color green() { return fromRgba(0, 255, 0); }

	static Color blue() { return fromRgba(0, 0, 255); }

	static Color yellow() { return fromRgba(255, 255, 0); }

	static Color magenta() { return fromRgba(255, 0, 255); }

	static Color cyan() { return fromRgba(0, 255, 255); }

	static Color white() { return fromRgba(255, 255, 255); }

	static Color gray() { return fromRgba(128, 128, 128); }

	static Color black() { return fromRgba(0, 0, 0); }

	static Color fromRgba(uint8 r, uint8 g, uint8 b, uint8 a = 255) { return { r, g, b, a }; }

	static Color random(const uint8 min, const uint8 max)
	{
		std::random_device				rd;
		std::mt19937					generator(rd());
		std::uniform_int_distribution<> distr(min, max); // define the range

		Color out;
		for (uint8 i = 0; i < 4; i++)
		{
			out.rgba[i] = distr(generator);
		}
		return out;
	}

	static Color random() { return random(0, 255); }

	static Color fromUInt32(const int32 value)
	{
		uint8 a = UINT8_MAX & value >> 24;
		uint8 r = UINT8_MAX & value >> 16;
		uint8 g = UINT8_MAX & value >> 8;
		uint8 b = UINT8_MAX & value;
		return { r, g, b, a };
	}

	[[nodiscard]] int32 toInt32() const { return (r << 16) | (g << 8) | b | 0; }

	std::string toString() { return std::format("RGBA[{}, {}, {}, {}]", r, g, b, a); }

	Color operator*(const Color& other) const
	{
		Color out;
		out.r = r * other.r;
		out.g = g * other.g;
		out.b = b * other.b;
		out.a = a * other.a;
		return out;
	}

	Color operator*(float s) const
	{
		Color out = *this;
		out.r *= s;
		out.g *= s;
		out.b *= s;
		out.a *= s;
		return out;
	}

	Color& operator*=(const Color& other)
	{
		this->r *= other.r;
		this->g *= other.g;
		this->b *= other.b;
		this->a *= other.a;
		return *this;
	}

	Color& operator*=(float s)
	{
		this->r *= s;
		this->g *= s;
		this->b *= s;
		this->a *= s;
		return *this;
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
