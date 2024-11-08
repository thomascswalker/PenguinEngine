#pragma once

#include <memory>
#include <vector>
#include <cassert>

#include "Core/LinkedList.h"
#include "Math/Vector.h"
#include "Math/Transform.h"

class Mesh;

struct Index2;
struct Index3;
struct Index4;

// Vertex3 2D
template <typename T>
struct Vertex2_t;
// Vertex3 2D (Int32)
using Vertex2i = Vertex2_t<int32>;
// Vertex3 2D (float)
using Vertex2f = Vertex2_t<float>;

// Triangle 2D
template <typename T>
struct Triangle2_t;
// Triangle 2D (Int32)
using Triangle2i = Triangle2_t<int32>;
// Triangle 2D (float)
using Triangle2f = Triangle2_t<float>;

// Vertex3 3D (float)
struct Vertex3;
// Triangle 3D (float)
struct Triangle3;

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

struct Index2
{
	union
	{
		struct
		{
			int32 a;
			int32 b;
		};
		int32 ab[2] = { 0, 0 };
	};

	Index2() = default;
	Index2(int32 inA, int32 inB) : a(inA), b(inB) {}

	int32&		 operator[](int32 index) { return ab[index]; }
	const int32& operator[](int32 index) const { return ab[index]; }

	bool contains(int32 index) const { return (a == index) || (b == index); }
};

struct Index3
{
	union
	{
		struct
		{
			int32 a;
			int32 b;
			int32 c;
		};
		int32 abc[3] = { 0, 0, 0 };
	};

	Index3() = default;
	Index3(int32 inA, int32 inB, int32 inC) : a(inA), b(inB), c(inC) {}

	int32&		 operator[](int32 index) { return abc[index]; }
	const int32& operator[](int32 index) const { return abc[index]; }

	bool contains(int32 index) const { return (a == index) || (b == index) || (c == index); }
};

struct Index4
{
	union
	{
		struct
		{
			int32 a;
			int32 b;
			int32 c;
			int32 d;
		};
		int32 abcd[4] = { 0, 0, 0, 0 };
	};

	Index4() = default;
	Index4(int32 inA, int32 inB, int32 inC, int32 inD) : a(inA), b(inB), c(inC), d(inD) {}

	int32&		 operator[](int32 index) { return abcd[index]; }
	const int32& operator[](int32 index) const { return abcd[index]; }

	bool contains(int32 index) const { return (a == index) || (b == index) || (c == index) || (d == index); }
};

template <typename T> 
struct Vertex2_t
{
	vec2_t<T> position;
};

struct Vertex3
{
	vec3f position; // 12-bytes
	vec3f normal;	// 12-bytes
	vec2f texCoord; // 8-bytes

	Vertex3() = default;
	Vertex3(float x, float y, float z, float nx, float ny, float nz, float u, float v) : position(x, y, z), normal(nx, ny, nz), texCoord(u, v) {}
	Vertex3(const vec3f& inPosition, const vec3f& inNormal, const vec2f& inTexCoord) : position(inPosition), normal(inNormal), texCoord(inTexCoord) {}
};

template<typename T>
struct Triangle2_t
{
	Vertex2_t<T> v0;
	Vertex2_t<T> v1;

	static float signedArea(const vec2_t<T>& a, const vec2_t<T>& b, const vec2_t<T>& c) { return T(0.5) * ((a.x * b.y - a.y * b.x) + (b.x * c.y - b.y * c.x) + (c.x * a.y - c.y * a.x)); }
};

struct Triangle3
{
	Vertex3 v0;
	Vertex3 v1;
	Vertex3 v2;

	std::vector<int32> positionIndexes;
	std::vector<int32> normalIndexes;
	std::vector<int32> texCoordIndexes;

	Triangle3() = default;

	Triangle3(const std::vector<int32>& inPositionIndexes, const std::vector<int32>& inNormalIndexes, const std::vector<int32>& inTexCoordIndexes)
		: positionIndexes(inPositionIndexes), normalIndexes(inNormalIndexes), texCoordIndexes(inTexCoordIndexes)
	{
	}

	Vertex3 operator[](int32 index) const
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

	Vertex3& operator[](int32 index)
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
	std::vector<Triangle3> m_triangles;
	std::vector<vec3f>	   m_positions;
	std::vector<vec3f>	   m_normals;
	std::vector<vec2f>	   m_texCoords;

	std::vector<float> m_vertexBuffer;

public:
	Mesh() = default;

	Mesh(const std::vector<Triangle3>& inTriangles, const std::vector<vec3f>& inPositions, const std::vector<vec3f>& inNormals = {}, const std::vector<vec2f>& inTexCoords = {})
		: m_triangles(inTriangles), m_positions(inPositions), m_normals(inNormals), m_texCoords(inTexCoords)
	{
		processTriangles();
	}

	[[nodiscard]] bool hasNormals() const { return !m_normals.empty(); }

	[[nodiscard]] bool hasTexCoords() const { return !m_texCoords.empty(); }

	void processTriangles();

	std::vector<float> toVertexData();

	std::vector<Triangle3>* getTriangles() { return &m_triangles; }

	[[nodiscard]] std::vector<vec3f>* getPositions() { return &m_positions; }

	[[nodiscard]] std::vector<vec3f>* getNormals() { return &m_normals; }

	[[nodiscard]] std::vector<vec2f>* getTexCoords() { return &m_texCoords; }

	void setTriangles(const std::vector<Triangle3>& triangles) { m_triangles = triangles; }

	void setPositions(const std::vector<vec3f>& positions) { m_positions = positions; }

	void setNormals(const std::vector<vec3f>& normals) { m_normals = normals; }

	void setTexCoords(const std::vector<vec2f>& texCoords) { m_texCoords = texCoords; }

	std::vector<float>* getVertexData() { return &m_vertexBuffer; }

	/** Returns the size of this mesh's geometry in bytes. **/
	[[nodiscard]] size_t memorySize() const { return m_vertexBuffer.size() * sizeof(float); }
};

struct MeshDescription
{
	/** Pointer to the transform of this mesh. **/
	transf* transform = nullptr;
	/** Byte size of the mesh. **/
	uint32 byteSize = 0;
	/** Vertex3 count. **/
	uint32 vertexCount = 0;
	/** Index count. **/
	uint32 indexCount = 0;
	/** Stride **/
	uint32 stride = 0;
	/** Vertex3 data pointer **/
	float* data = nullptr;
};

// https://github.com/SebLague/Shape-Editor-Tool/blob/master/Shape%20Editor%20E04/Assets/Geometry/Triangulator.cs
namespace Triangulation
{
	bool isTriangleFlipped(int32 orientation, const vec2i& a, const vec2i& b, const vec2i& c);
	bool triangulate(const std::vector<vec2i>& inVertexPositions, std::vector<Index3>& indexes);
} // namespace Triangulation