#pragma once

#include "Vector.h"

template <typename T>
struct rect_t
{
	struct
	{
		T x      = 0;
		T y      = 0;
		T width  = 0;
		T height = 0;
	};

	rect_t() {}

	rect_t(const vec2_t<T>& inMin, const vec2_t<T>& inMax)
	{
		x      = inMin.x;
		y      = inMin.y;
		width  = inMax.x - inMin.x;
		height = inMax.y - inMin.y;
	}

	rect_t(T inX, T inY, T inWidth, T inHeight)
	{
		x      = inX;
		y      = inY;
		width  = inWidth;
		height = inHeight;
	}

	vec2_t<T> min() const
	{
		return vec2_t(x, y);
	}

	vec2_t<T> max() const
	{
		return vec2_t(x + width, y + height);
	}

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
		x      = std::max(x, other.x);
		y      = std::max(y, other.y);
		width  = std::min(width, other.width);
		height = std::min(height, other.height);
	}

	void grow(T value)
	{
		x -= value;
		y -= value;
		width += value;
		height += value;
	}

	void shrink(T value)
	{
		x += value;
		y += value;
		width -= value;
		height -= value;
	}

	[[nodiscard]] bool contains(const vec2f& point) const
	{
		return point.x >= x && point.y >= y && point.x <= x + width && point.y <= y + height;
	}

	[[nodiscard]] bool overlaps(const rectf& other) const
	{
		return other.x >= x || other.y >= y || other.x + width <= x + width || other.y + height <= y + height;
	}
};
