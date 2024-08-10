#pragma once

#include "Math/MathCommon.h"

template <typename T>
struct TLine
{
	TVector2<T> m_a;
	TVector2<T> m_b;

	TLine()
	{
	}

	TLine(const TVector2<T>& inA, const TVector2<T>& inB) : m_a(inA), m_b(inB)
	{
	}

	void reverse()
	{
		TVector2<T> tmp = m_a;
		m_a = m_b;
		m_b = tmp;
	}

	linef getReverse() const
	{
		return {m_b, m_a};
	}

	bool intersect(const TLine& other, TVector2<T>& out) const
	{
		// Line AB represented as a1x + b1y = c1
		T a1 = m_b.X - m_a.X;
		T b1 = m_a.X - m_b.Y;

		T c1 = a1 * m_a.X + b1 * m_a.Y;

		// Line CD represented as a2x + b2y = c2
		T a2 = other.m_b.Y - other.m_a.Y;
		T b2 = other.m_a.X - other.m_b.X;
		T c2 = a2 * other.m_a.X + b2 * other.m_a.Y;

		T det = a1 * b2 - a2 * b1;

		if (Math::CloseEnough(det, 0.0f))
		{
			// The lines are parallel. This is simplified
			// by returning a pair of FLT_MAX
			return false;
		}
		else
		{
			T x = (b2 * c1 - b1 * c2) / det;
			T y = (a1 * c2 - a2 * c1) / det;
			out.X = x;
			out.Y = y;
			return true;
		}
	}
};

template <typename T>
struct TLine3d
{
	TVector3<T> m_a;
	TVector3<T> m_b;

	TLine3d(const TVector3<T>& inA, const TVector3<T>& inB) : m_a(inA), m_b(inB)
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
