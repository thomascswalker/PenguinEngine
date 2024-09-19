#pragma once
#include <vector>

#include "Math/Color.h"

class Tile
{
	using TriangleArray = std::vector<Triangle*>;
	using LineArray     = std::vector<linef>;

	Color m_color;
	uint32 m_id;               // ID of this tile
	rectf m_bounds;            // 2D bounds of this tile
	TriangleArray m_triangles; // The triangles which overlap this tile.
	LineArray m_lines;

public:
	Tile(const vec2f& min, const vec2f& max, const int32 id)
		: m_color(Color::random(128, 255)), m_id(id)
	{
		setBounds(min, max);
	}

	void setBounds(const vec2f& min, const vec2f& max)
	{
		m_bounds = rectf(min, max);

		// Compute each line
		m_lines.clear();
		m_lines.emplace_back(vec2f(min.x + 1, min.y + 1), vec2f(max.x - 1, min.y + 1)); // Top-left to top-right
		m_lines.emplace_back(vec2f(max.x - 1, min.y + 1), vec2f(max.x - 1, max.y - 1)); // Top-right to bottom-right
		m_lines.emplace_back(vec2f(max.x - 1, max.y - 1), vec2f(min.x + 1, max.y - 1)); // Top-left to top-right
		m_lines.emplace_back(vec2f(min.x + 1, max.y - 1), vec2f(min.x + 1, min.y + 1)); // Top-left to top-right
	}

	void addTriangle(Triangle* triangle)
	{
		m_triangles.push_back(triangle);
	}

	Triangle* getTriangle(const int32 index) const
	{
		return m_triangles[index];
	}

	TriangleArray* getTriangles()
	{
		return &m_triangles;
	}

	LineArray getLines() const
	{
		return m_lines;
	}

	Color getColor() const
	{
		return m_color;
	}

	[[nodiscard]] rectf getBounds() const
	{
		return m_bounds;
	}

	[[nodiscard]] bool inside(const vec2f& point) const
	{
		return m_bounds.contains(point);
	}
};
