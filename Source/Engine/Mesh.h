#pragma once

#include <memory>
#include <vector>
#include <cassert>

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
	vec3f normal;	// 12-bytes
	vec2f texCoord; // 8-bytes

	Vertex() = default;
	Vertex(float x, float y, float z, float nx, float ny, float nz, float u, float v) : position(x, y, z), normal(nx, ny, nz), texCoord(u, v) {}
	Vertex(const vec3f& inPosition, const vec3f& inNormal, const vec2f& inTexCoord) : position(inPosition), normal(inNormal), texCoord(inTexCoord) {}
};

struct Vertex2D
{
	Vertex2D*	  prev;
	Vertex2D*	  next;
	int32		  index;
	EWindingOrder windingOrder;
	bool		  reflex;
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

	Triangle(const std::vector<int32>& inPositionIndexes, const std::vector<int32>& inNormalIndexes, const std::vector<int32>& inTexCoordIndexes)
		: positionIndexes(inPositionIndexes), normalIndexes(inNormalIndexes), texCoordIndexes(inTexCoordIndexes)
	{
	}

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

template <typename T> struct Triangle2D
{
	vec2_t<T> v0;
	vec2_t<T> v1;
	vec2_t<T> v2;
};

class Mesh
{
	// Properties
	std::vector<Triangle> m_triangles;
	std::vector<vec3f>	  m_positions;
	std::vector<vec3f>	  m_normals;
	std::vector<vec2f>	  m_texCoords;

	std::vector<float> m_vertexBuffer;

public:
	Mesh() = default;

	Mesh(const std::vector<Triangle>& inTriangles, const std::vector<vec3f>& inPositions, const std::vector<vec3f>& inNormals = {}, const std::vector<vec2f>& inTexCoords = {})
		: m_triangles(inTriangles), m_positions(inPositions), m_normals(inNormals), m_texCoords(inTexCoords)
	{
		processTriangles();
	}

	[[nodiscard]] bool hasNormals() const { return !m_normals.empty(); }

	[[nodiscard]] bool hasTexCoords() const { return !m_texCoords.empty(); }

	void processTriangles();

	std::vector<float> toVertexData();

	std::vector<Triangle>* getTriangles() { return &m_triangles; }

	[[nodiscard]] std::vector<vec3f>* getPositions() { return &m_positions; }

	[[nodiscard]] std::vector<vec3f>* getNormals() { return &m_normals; }

	[[nodiscard]] std::vector<vec2f>* getTexCoords() { return &m_texCoords; }

	void setTriangles(const std::vector<Triangle>& triangles) { m_triangles = triangles; }

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
	/** Vertex count. **/
	uint32 vertexCount = 0;
	/** Index count. **/
	uint32 indexCount = 0;
	/** Stride **/
	uint32 stride = 0;
	/** Vertex data pointer **/
	float* data = nullptr;
};

class Triangulator
{
	// Internal vertex structure for triangulation
	struct Point
	{
		vec2i*		  pos;
		int32		  index;		// Index within the points array
		int32		  prevIndex;	// The previous point index
		int32		  nextIndex;	// The next point index
		EWindingOrder windingOrder; // The winding order of this point
		int32		  interiorAngle;

		bool isReflex() const { return interiorAngle >= 180; }
		bool isConvex() const { return interiorAngle < 180; }
	};

	std::vector<vec2i> vertices;
	std::vector<Point> points;
	std::vector<Point> earTips;
	int32			   topLeftIndex = 0;

	void updateTopLeftIndex(int32 i)
	{
		vec2i& vertex = vertices[i];
		vec2i& topLeftVertex = vertices[topLeftIndex];

		if (vertex.x < topLeftVertex.x			// Left of the top-left most vertex
			|| (vertex.x == topLeftVertex.x		// Equal to the top-left most vertex
				&& vertex.y < topLeftVertex.y)) // But also above the top of the top-left most vertex
		{
			topLeftIndex = i; // Update the top-left most vertex index
		}
	}

	void initializePoint(int32 i, Point* p)
	{
		p->pos = &vertices[i];
		p->index = i;
		p->prevIndex = i == 0 ? vertices.size() - 1 : i - 1;
		p->nextIndex = i == vertices.size() - 1 ? 0 : i + 1;

		// Angle between two vectors
		const vec2i& v0 = vertices[p->prevIndex];
		const vec2i& v1 = vertices[p->index];
		const vec2i& v2 = vertices[p->nextIndex];

		const vec2i a = v0 - v1;
		const vec2i b = v2 - v1;

		float dot = a.x * b.x + a.y * b.y;
		float aLength = sqrt(a.x * a.x + a.y * a.y);
		float bLength = sqrt(b.x * b.x + b.y * b.y);
		int32 angle = acos(dot / (aLength * bLength));
		p->interiorAngle = angle < 0 ? 360 + angle : angle; // Normalize 0..360

		updateTopLeftIndex(i);
	}

	bool triangleContainsAnyPoint(Point* p0, Point* p1, Point* p2)
	{
		Point* p = &points[0];
		for (int32 i = 0; i < points.size(); i++) 
		{
			// Convex verts are never inside a triangle
			if (p->isReflex())
			{
				// Is this a different point from the three we are checking
				if (p->index != p0->index && p->index != p1->index && p->index != p2->index)
				{
					vec2i vp = vertices[p->index];
					vec2i v0 = vertices[p0->index];
					vec2i v1 = vertices[p1->index];
					vec2i v2 = vertices[p2->index];
					if (Math::isBarycentric(v0, v1, v2, vp))
					{
						return true;
					}
				}
			}

			// Move to the next point
			p = &points[p->nextIndex];
		}

		return false;
	}

public:
	Triangulator(const std::vector<vec2i>& inVertices) : vertices(inVertices) {}

	std::vector<int32> triangulate()
	{
		std::vector<int32> indexes;

		int32 pointCount = vertices.size();
		int32 triangleCount = pointCount - 2;

		// Minimum of 3 points.
		if (pointCount < 3)
		{
			return indexes;
		}

		if (pointCount == 3)
		{
			indexes.resize(3);
			indexes[0] = 0;
			indexes[1] = 1;
			indexes[2] = 2;
		}

		points.resize(pointCount);
		for (int i = 0; i < pointCount; i++)
		{
			Point* p = &points[i];
			initializePoint(i, p);
		}

		std::vector<Point*> unprocessedPoints;
		for (int32 i = 0; i < pointCount; i++)
		{
			unprocessedPoints.emplace_back(&points[i]);
		}

		while (unprocessedPoints.size() >= 3)
		{
			Point* current = unprocessedPoints[0];

			if (unprocessedPoints.size() == 3)
			{
				Point* prev = &points[current->prevIndex];
				Point* next = &points[current->nextIndex];

				indexes.emplace_back(prev->index);
				indexes.emplace_back(current->index); // Ear
				indexes.emplace_back(next->index);

				break;
			}

			for (int32 i = 0; i < unprocessedPoints.size(); i++)
			{
				Point* prev = &points[current->prevIndex];
				Point* next = &points[current->nextIndex];

				// Is convex
				if (current->isConvex())
				{
					bool inTriangle = triangleContainsAnyPoint(prev, current, next);
					if (!inTriangle)
					{
						indexes.emplace_back(prev->index);
						indexes.emplace_back(current->index); // Ear
						indexes.emplace_back(next->index);

						unprocessedPoints.erase(unprocessedPoints.begin() + i);
					}
				}

				current = next;
			}
		}
		

		assert(indexes.size() / 3 == triangleCount);
		return indexes;
	}
};
