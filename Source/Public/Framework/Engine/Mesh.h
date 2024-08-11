#pragma once

#include <vector>
#include "Object.h"

enum EPrimitiveType
{
	Plane,
	Cube,
	Sphere,
	Torus
};

struct Vertex
{
	vec3f m_position;
	vec3f m_normal;
	vec3f m_texCoord;

	Vertex() = default;

	Vertex(const vec3f& inPosition, const vec3f& inNormal, const vec3f& inTexCoord)
		: m_position(inPosition),
		  m_normal(inNormal),
		  m_texCoord(inTexCoord)
	{
	}
};

struct Triangle
{
	std::vector<int32> m_positionIndexes;
	std::vector<int32> m_normalIndexes;
	std::vector<int32> m_texCoordIndexes;

	Vertex m_v0;
	Vertex m_v1;
	Vertex m_v2;

	Triangle() = default;

	Triangle(const std::vector<int32>& inPositionIndexes, const std::vector<int32>& inNormalIndexes,
	         const std::vector<int32>& inTexCoordIndexes)
		: m_positionIndexes(inPositionIndexes),
		  m_normalIndexes(inNormalIndexes),
		  m_texCoordIndexes(inTexCoordIndexes)
	{
	}
};

struct Mesh : Object
{
	// Properties
	std::vector<Triangle> m_triangles;
	std::vector<vec3f> m_positions;
	std::vector<vec3f> m_normals;
	std::vector<vec2f> m_texCoords;

	Mesh() = default;

	Mesh(const std::vector<Triangle>& inTriangles, const std::vector<vec3f>& inPositions,
	     const std::vector<vec3f>& inNormals = {}, const std::vector<vec2f>& inTexCoords = {})
		: m_triangles(inTriangles), m_positions(inPositions), m_normals(inNormals), m_texCoords(inTexCoords)
	{
		processTriangles();
	}

	void processTriangles();

	// Primitives
	static std::shared_ptr<Mesh> createPlane(float size);
	static std::shared_ptr<Mesh> createPlane(float width, float height);
};
