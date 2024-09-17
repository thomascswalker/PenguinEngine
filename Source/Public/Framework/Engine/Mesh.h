#pragma once

#include <vector>
#include "Object.h"

class Mesh;
/* Global container for all mesh objects. */
inline std::vector<std::shared_ptr<Mesh>> g_meshes;

namespace MeshManager
{
	inline int32 count()
	{
		return g_meshes.size();
	}

	inline Mesh* getMesh(int32 index)
	{
		return g_meshes[index].get();
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
	vec3f position;
	vec3f normal;
	vec2f texCoord;

	Vertex() = default;

	Vertex(const vec3f& inPosition, const vec3f& inNormal, const vec2f& inTexCoord)
		: position(inPosition),
		  normal(inNormal),
		  texCoord(inTexCoord) {}
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
		: positionIndexes(inPositionIndexes),
		  normalIndexes(inNormalIndexes),
		  texCoordIndexes(inTexCoordIndexes) {}
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

	bool hasNormals() const { return !m_normals.empty(); }
	bool hasTexCoords() const { return !m_texCoords.empty(); }
	void processTriangles();

	// Primitives
	static std::shared_ptr<Mesh> createPlane(float size);
	static std::shared_ptr<Mesh> createPlane(float width, float height);
};
