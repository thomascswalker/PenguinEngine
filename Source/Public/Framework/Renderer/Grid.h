#pragma once

#include "Math/MathCommon.h"

template <typename T>
struct line_t
{
	vec2_t<T> m_a;
	vec2_t<T> m_b;

	line_t()
	{
	}

	line_t(const vec2_t<T>& inA, const vec2_t<T>& inB) : m_a(inA), m_b(inB)
	{
	}

	void reverse()
	{
		vec2_t<T> tmp = m_a;
		m_a = m_b;
		m_b = tmp;
	}

	linef getReverse() const
	{
		return {m_b, m_a};
	}

	bool intersect(const line_t& other, vec2_t<T>& out) const
	{
		// Line AB represented as a1x + b1y = c1
		T a1 = m_b.x - m_a.x;
		T b1 = m_a.x - m_b.y;

		T c1 = a1 * m_a.x + b1 * m_a.y;

		// Line CD represented as a2x + b2y = c2
		T a2 = other.m_b.y - other.m_a.y;
		T b2 = other.m_a.x - other.m_b.x;
		T c2 = a2 * other.m_a.x + b2 * other.m_a.y;

		T det = a1 * b2 - a2 * b1;

		if (Math::closeEnough(det, 0.0f))
		{
			// The lines are parallel. This is simplified
			// by returning a pair of FLT_MAX
			return false;
		}
		else
		{
			T x = (b2 * c1 - b1 * c2) / det;
			T y = (a1 * c2 - a2 * c1) / det;
			out.x = x;
			out.y = y;
			return true;
		}
	}
};

template <typename T>
struct line3d_t
{
	vec3_t<T> m_a;
	vec3_t<T> m_b;

	line3d_t(const vec3_t<T>& inA, const vec3_t<T>& inB) : m_a(inA), m_b(inB)
	{
	}
};

struct FGrid
{
	std::vector<line3d> m_lines;

	FGrid(const float divisions, const float cellSize)
	{
		for (float step = 0; step <= divisions; step += cellSize)
		{
			m_lines.emplace_back(vec3f(-divisions, 0.0f, step), vec3f(divisions, 0.0f, step));
			m_lines.emplace_back(vec3f(-divisions, 0.0f, -step), vec3f(divisions, 0.0f, -step));
			m_lines.emplace_back(vec3f(step, 0.0f, -divisions), vec3f(step, 0.0f, divisions));
			m_lines.emplace_back(vec3f(-step, 0.0f, -divisions), vec3f(-step, 0.0f, divisions));
		}
	}
};
