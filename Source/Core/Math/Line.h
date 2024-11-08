#pragma once

#include "Vector.h"

template <typename T> struct line_t
{
	vec2_t<T> a;
	vec2_t<T> b;

	line_t() = default;

	line_t(const vec2_t<T>& inA, const vec2_t<T>& inB) : a(inA), b(inB) {}

	void reverse()
	{
		vec2_t<T> tmp = a;
		a = b;
		b = tmp;
	}

	[[nodiscard]] linef getReverse() const { return { b, a }; }

	bool intersect(const line_t& other, vec2_t<T>& out) const
	{
		// Line AB represented as a1x + b1y = c1
		T a1 = b.x - a.x;
		T b1 = a.x - b.y;
		T c1 = a1 * a.x + b1 * a.y;

		// Line CD represented as a2x + b2y = c2
		T a2 = other.b.y - other.a.y;
		T b2 = other.a.x - other.b.x;
		T c2 = a2 * other.a.x + b2 * other.a.y;

		T det = a1 * b2 - a2 * b1;

		if (Math::closeEnough(det, 0.0f))
		{
			// The m_lines are parallel. This is simplified
			// by returning a pair of FLT_MAX
			return false;
		}

		T x = (b2 * c1 - b1 * c2) / det;
		T y = (a1 * c2 - a2 * c1) / det;
		out.x = x;
		out.y = y;
		return true;
	}
};

template <typename T> struct line3d_t
{
	vec3_t<T> a;
	vec3_t<T> b;

	line3d_t(const vec3_t<T>& inA, const vec3_t<T>& inB) : a(inA), b(inB) {}
};

namespace Math
{
	template <typename T>
	bool intersect(const line_t<T>& a, const line_t<T>& b, vec2_t<T>& out)
	{
		// Line AB represented as a1x + b1y = c1
		T a1 = a.b.x - a.a.x;
		T b1 = a.a.x - a.b.y;
		T c1 = a1 * a.a.x + b1 * a.a.y;

		// Line CD represented as a2x + b2y = c2
		T a2 = b.b.y - b.a.y;
		T b2 = b.a.x - b.b.x;
		T c2 = a2 * b.a.x + b2 * b.a.y;

		T det = a1 * b2 - a2 * b1;

		if (Math::closeEnough(det, 0.0f))
		{
			// The m_lines are parallel. This is simplified
			// by returning a pair of FLT_MAX
			return false;
		}

		T x = (b2 * c1 - b1 * c2) / det;
		T y = (a1 * c2 - a2 * c1) / det;
		out.x = x;
		out.y = y;
		return true;
	}
}