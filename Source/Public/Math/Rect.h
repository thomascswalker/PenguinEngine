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

	rect_t(const vec2_t<T>& inMin, const vec2_t<T>& inMax)
	{
		X = inMin.x;
		Y = inMin.y;
		Width = inMax.x - inMin.x;
		Height = inMax.y - inMin.y;
	}

	rect_t(T inX, T inY, T inWidth, T inHeight)
	{
		X = inX;
		Y = inY;
		Width = inWidth;
		Height = inHeight;
	}

	vec2_t<T> min() const { return vec2_t(X, Y); }
	vec2_t<T> max() const { return vec2_t(X + Width, Y + Height); }

	static rect_t makeBoundingBox(const vec2_t<T>& v0, const vec2_t<T>& v1)
	{
		T xValues[2] = {v0.x, v1.x};
		T yValues[2] = {v0.y, v1.y};

		const T minX = *std::ranges::min_element(xValues);
		const T minY = *std::ranges::min_element(yValues);
		const T maxX = *std::ranges::max_element(xValues);
		const T maxY = *std::ranges::max_element(yValues);

		vec2_t<T> bbMin(minX, minY);
		vec2_t<T> bbMax(maxX, maxY);

		return rect_t(bbMin, bbMax);
	}

	static rect_t makeBoundingBox(const vec2_t<T>& v0, const vec2_t<T>& v1, const vec2_t<T>& v2)
	{
		T xValues[3] = {v0.x, v1.x, v2.x};
		T yValues[3] = {v0.y, v1.y, v2.y};

		const T minX = *std::ranges::min_element(xValues);
		const T minY = *std::ranges::min_element(yValues);
		const T maxX = *std::ranges::max_element(xValues);
		const T maxY = *std::ranges::max_element(yValues);

		vec2_t<T> bbMin(minX, minY);
		vec2_t<T> bbMax(maxX, maxY);

		return rect_t(bbMin, bbMax);
	}

	void clamp(const rect_t& other)
	{
		X = std::max(X, other.X);
		Y = std::max(Y, other.Y);
		Width = std::min(Width, other.Width);
		Height = std::min(Height, other.Height);
	}

	void grow(T value)
	{
		X -= value;
		Y -= value;
		Width += value;
		Height += value;
	}

	void shrink(T value)
	{
		X += value;
		Y += value;
		Width -= value;
		Height += value;
	}
};
