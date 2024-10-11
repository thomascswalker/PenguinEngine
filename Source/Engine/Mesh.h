#pragma once

#include <memory>
#include <vector>

#include "Math/Vector.h"
#include "Math/Transform.h"

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
	Vertex v0;
	Vertex v1;
	Vertex v2;

	std::vector<int32> positionIndexes;
	std::vector<int32> normalIndexes;
	std::vector<int32> texCoordIndexes;

	Triangle() = default;

	Triangle(const std::vector<int32>& inPositionIndexes, const std::vector<int32>& inNormalIndexes,
	         const std::vector<int32>& inTexCoordIndexes)
		: positionIndexes(inPositionIndexes)
		  , normalIndexes(inNormalIndexes)
		  , texCoordIndexes(inTexCoordIndexes) {}

	Vertex operator[](int32 index) const
	{
		switch (index)
		{
			case 0:
				return v0;
			case 1:
				return v1;
			case 2:
				return v2;
		}
	}

	Vertex& operator[](int32 index)
	{
		switch (index)
		{
			case 0:
				return v0;
			case 1:
				return v1;
			case 2:
				return v2;
		}
	}
};

class Mesh
{
	// Properties
	std::vector<Triangle> m_triangles;
	std::vector<vec3f> m_positions;
	std::vector<vec3f> m_normals;
	std::vector<vec2f> m_texCoords;

	std::vector<float> m_vertexBuffer;

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

	std::vector<float> toVertexData();

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

	std::vector<float>* getVertexData()
	{
		return &m_vertexBuffer;
	}

	/** Returns the size of this mesh's geometry in bytes. **/
	[[nodiscard]] size_t memorySize() const
	{
		return m_vertexBuffer.size() * sizeof(float);
	}
};

struct MeshDescription
{
	/** Pointer to the transform of this mesh. **/
	transf* transform = nullptr;
	/** Byte size of the mesh. **/
	uint32 byteSize = 0;
	/** Vertex count. **/
	uint32 vertexCount = 0;
	/** Index count. **/
	uint32 indexCount = 0;
	/** Stride **/
	uint32 stride = 0;
	/** Vertex data pointer **/
	float* data = nullptr;
};
