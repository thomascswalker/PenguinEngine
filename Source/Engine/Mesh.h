#pragma once

#include <memory>
#include <vector>
#include "math/Vector.h"

class Mesh;
/* Global container for all mesh objects. */
inline std::vector<Mesh*> g_meshes;

namespace MeshManager
{
	inline size_t count()
	{
		return g_meshes.size();
	}

	inline Mesh* getMesh(const int32 index)
	{
		return g_meshes[index];
	}
} // namespace MeshManager

enum EPrimitiveType
{
	Plane,
	Cube,
	Sphere,
	Torus
};

struct Vertex
{
	vec3f position; // 12-bytes
	vec3f normal;   // 12-bytes
	vec2f texCoord; // 8-bytes

	Vertex() = default;

	Vertex(const vec3f& inPosition, const vec3f& inNormal, const vec2f& inTexCoord)
		: position(inPosition)
		  , normal(inNormal)
		  , texCoord(inTexCoord) {}
};

struct Triangle
{
	std::vector<int32> positionIndexes;
	std::vector<int32> normalIndexes;
	std::vector<int32> texCoordIndexes;

	Vertex v0;
	Vertex v1;
	Vertex v2;

	Triangle() = default;

	Triangle(const std::vector<int32>& inPositionIndexes, const std::vector<int32>& inNormalIndexes,
	         const std::vector<int32>& inTexCoordIndexes)
		: positionIndexes(inPositionIndexes)
		  , normalIndexes(inNormalIndexes)
		  , texCoordIndexes(inTexCoordIndexes) {}
};

class Mesh
{
	// Properties
	std::vector<Triangle> m_triangles;
	std::vector<vec3f> m_positions;
	std::vector<vec3f> m_normals;
	std::vector<vec2f> m_texCoords;

public:
	Mesh() = default;

	Mesh(const std::vector<Triangle>& inTriangles, const std::vector<vec3f>& inPositions,
	     const std::vector<vec3f>& inNormals = {}, const std::vector<vec2f>& inTexCoords = {})
		: m_triangles(inTriangles)
		  , m_positions(inPositions)
		  , m_normals(inNormals)
		  , m_texCoords(inTexCoords)
	{
		processTriangles();
	}

	[[nodiscard]] bool hasNormals() const
	{
		return !m_normals.empty();
	}

	[[nodiscard]] bool hasTexCoords() const
	{
		return !m_texCoords.empty();
	}

	void processTriangles();

	std::vector<Triangle>* getTriangles()
	{
		return &m_triangles;
	}

	[[nodiscard]] std::vector<vec3f>* getPositions()
	{
		return &m_positions;
	}

	[[nodiscard]] std::vector<vec3f>* getNormals()
	{
		return &m_normals;
	}

	[[nodiscard]] std::vector<vec2f>* getTexCoords()
	{
		return &m_texCoords;
	}

	void setTriangles(const std::vector<Triangle>& triangles)
	{
		m_triangles = triangles;
	}

	void setPositions(const std::vector<vec3f>& positions)
	{
		m_positions = positions;
	}

	void setNormals(const std::vector<vec3f>& normals)
	{
		m_normals = normals;
	}

	void setTexCoords(const std::vector<vec2f>& texCoords)
	{
		m_texCoords = texCoords;
	}

	// Primitives
	static std::shared_ptr<Mesh> createPlane(float size);
	static std::shared_ptr<Mesh> createPlane(float width, float height);
};
