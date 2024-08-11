#pragma once

#include "Vector.h"

template <typename T>
struct rect_t
{
	struct
	{
		T X;
		T Y;
		T Width;
		T Height;
	};

	rect_t()
	{
	}

	rect_t(const vec2_t<T>& InMin, const vec2_t<T>& InMax)
	{
		X = InMin.x;
		Y = InMin.y;
		Width = InMax.x - InMin.x;
		Height = InMax.y - InMin.y;
	}

	rect_t(T InX, T InY, T InWidth, T InHeight)
	{
		X = InX;
		Y = InY;
		Width = InWidth;
		Height = InHeight;
	}

	vec2_t<T> Min() const { return vec2_t(X, Y); }
	vec2_t<T> Max() const { return vec2_t(X + Width, Y + Height); }

	static rect_t MakeBoundingBox(const vec2_t<T>& V0, const vec2_t<T>& V1)
	{
		T XValues[2] = {V0.x, V1.x};
		T YValues[2] = {V0.y, V1.y};

		const T MinX = *std::ranges::min_element(XValues);
		const T MinY = *std::ranges::min_element(YValues);
		const T MaxX = *std::ranges::max_element(XValues);
		const T MaxY = *std::ranges::max_element(YValues);

		vec2_t<T> BBMin(MinX, MinY);
		vec2_t<T> BBMax(MaxX, MaxY);

		return rect_t(BBMin, BBMax);
	}

	static rect_t MakeBoundingBox(const vec2_t<T>& V0, const vec2_t<T>& V1, const vec2_t<T>& V2)
	{
		T XValues[3] = {V0.x, V1.x, V2.x};
		T YValues[3] = {V0.y, V1.y, V2.y};

		const T MinX = *std::ranges::min_element(XValues);
		const T MinY = *std::ranges::min_element(YValues);
		const T MaxX = *std::ranges::max_element(XValues);
		const T MaxY = *std::ranges::max_element(YValues);

		vec2_t<T> BBMin(MinX, MinY);
		vec2_t<T> BBMax(MaxX, MaxY);

		return rect_t(BBMin, BBMax);
	}

	void Clamp(const rect_t& Other)
	{
		X = std::max(X, Other.X);
		Y = std::max(Y, Other.Y);
		Width = std::min(Width, Other.Width);
		Height = std::min(Height, Other.Height);
	}

	void Grow(T Value)
	{
		X -= Value;
		Y -= Value;
		Width += Value;
		Height += Value;
	}

	void Shrink(T Value)
	{
		X += Value;
		Y += Value;
		Width -= Value;
		Height += Value;
	}
};
