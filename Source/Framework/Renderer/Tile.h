#pragma once

#include "Framework/Core/Array.h"

class Tile
{
	using TriangleArray = Array<Triangle*>;

	uint32 m_id;               // ID of this tile
	rectf m_bounds;            // 2D bounds of this tile
	TriangleArray m_triangles; // The triangles which overlap this tile.

public:
	Tile(const vec2f& min, const vec2f& max, const int32 id)
		: m_id(id)
	{
		m_bounds = rectf(min, max);
	}

	void addTriangle(Triangle* triangle)
	{
		m_triangles.append(triangle);
	}

	Triangle* getTriangle(const int32 index)
	{
		return m_triangles[index];
	}

	TriangleArray* getTriangles()
	{
		return &m_triangles;
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
