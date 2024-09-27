#pragma once
#include <vector>

#include "Core/Types.h"
#include "Engine/Mesh.h"
#include "Math/Color.h"

enum class ETileRenderState : uint8
{
	NotStarted,
	InProgress,
	Complete
};

class Tile
{
	using TriangleArray = std::vector<Triangle*>;
	using LineArray     = std::vector<linef>;

	Color m_color;
	uint32 m_id;               // ID of this tile
	rectf m_bounds;            // 2D bounds of this tile
	TriangleArray m_triangles; // The triangles which overlap this tile.
	LineArray m_lines;
	ETileRenderState m_state = ETileRenderState::NotStarted;

public:
	Tile(const vec2f& min, const vec2f& max, const int32 id)
		: m_color(Color::random(128, 255)), m_id(id)
	{
		setBounds(min, max);
	}

	[[nodiscard]] rectf getBounds() const
	{
		return m_bounds;
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

	[[nodiscard]] Triangle* getTriangle(const int32 index) const
	{
		return m_triangles[index];
	}

	TriangleArray* getTriangles()
	{
		return &m_triangles;
	}

	[[nodiscard]] LineArray getLines() const
	{
		return m_lines;
	}

	[[nodiscard]] Color getColor() const
	{
		return m_color;
	}

	[[nodiscard]] ETileRenderState getRenderState() const
	{
		return m_state;
	}

	void setRenderState(const ETileRenderState newState)
	{
		m_state = newState;
	}

	[[nodiscard]] bool inside(const vec2f& point) const
	{
		return m_bounds.contains(point);
	}
};
