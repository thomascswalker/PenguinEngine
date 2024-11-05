#pragma once

#include "Math/Line.h"
#include "Math/MathCommon.h"

class Grid
{
	std::vector<line3d> m_lines;

public:
	Grid(const float divisions, const float cellSize)
	{
		for (float step = 0; step <= divisions; step += cellSize)
		{
			m_lines.emplace_back(vec3f(-divisions, 0.0f, step), vec3f(divisions, 0.0f, step));
			m_lines.emplace_back(vec3f(-divisions, 0.0f, -step), vec3f(divisions, 0.0f, -step));
			m_lines.emplace_back(vec3f(step, 0.0f, -divisions), vec3f(step, 0.0f, divisions));
			m_lines.emplace_back(vec3f(-step, 0.0f, -divisions), vec3f(-step, 0.0f, divisions));
		}
	}

	[[nodiscard]] std::vector<line3d> getLines() const
	{
		return m_lines;
	}
};
