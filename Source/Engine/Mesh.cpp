#include "Engine/Mesh.h"

void Mesh::processTriangles()
{
	for (Triangle& triangle : m_triangles)
	{
		triangle.v0.position = m_positions[triangle.positionIndexes[0]];
		triangle.v1.position = m_positions[triangle.positionIndexes[1]];
		triangle.v2.position = m_positions[triangle.positionIndexes[2]];

		if (hasNormals() && !triangle.normalIndexes.empty())
		{
			/** Flip normals **/
			triangle.v0.normal = m_normals[triangle.normalIndexes[0]] * -1.0f;
			triangle.v1.normal = m_normals[triangle.normalIndexes[1]] * -1.0f;
			triangle.v2.normal = m_normals[triangle.normalIndexes[2]] * -1.0f;
		}

		if (hasTexCoords() && !triangle.texCoordIndexes.empty())
		{
			triangle.v0.texCoord = m_texCoords[triangle.texCoordIndexes[0]];
			triangle.v1.texCoord = m_texCoords[triangle.texCoordIndexes[1]];
			triangle.v2.texCoord = m_texCoords[triangle.texCoordIndexes[2]];
		}
	}
	m_vertexBuffer = toVertexData();
}

std::vector<float> Mesh::toVertexData()
{
	int32			   vertexSize = (int32)sizeof(Vertex);
	auto			   size = m_triangles.size() * 3 * vertexSize;
	std::vector<float> data(size);
	size_t			   offset = 0;

	float* dataPtr = data.data();
	// Triangle
	for (Triangle& tri : m_triangles)
	{
		// Vertex 0
		data[offset++] = tri.v0.position.x;
		data[offset++] = tri.v0.position.y;
		data[offset++] = tri.v0.position.z;
		data[offset++] = tri.v0.normal.x;
		data[offset++] = tri.v0.normal.y;
		data[offset++] = tri.v0.normal.z;
		data[offset++] = tri.v0.texCoord.x;
		data[offset++] = tri.v0.texCoord.y;

		// Vertex 1
		data[offset++] = tri.v1.position.x;
		data[offset++] = tri.v1.position.y;
		data[offset++] = tri.v1.position.z;
		data[offset++] = tri.v1.normal.x;
		data[offset++] = tri.v1.normal.y;
		data[offset++] = tri.v1.normal.z;
		data[offset++] = tri.v1.texCoord.x;
		data[offset++] = tri.v1.texCoord.y;

		// Vertex 2
		data[offset++] = tri.v2.position.x;
		data[offset++] = tri.v2.position.y;
		data[offset++] = tri.v2.position.z;
		data[offset++] = tri.v2.normal.x;
		data[offset++] = tri.v2.normal.y;
		data[offset++] = tri.v2.normal.z;
		data[offset++] = tri.v2.texCoord.x;
		data[offset++] = tri.v2.texCoord.y;
	}

	return data;
}

bool Triangulation::isTriangleFlipped(int32 orientation, const vec2i& a, const vec2i& b, const vec2i& c)
{
	int32 triSignedArea = Triangle2::signedArea(a, b, c);
	return triSignedArea * orientation < 0;
}

// Triangulation
// https://github.com/EpicGames/UnrealEngine/blob/40eea367040d50aadd9f030ed5909fc890c159c2/Engine/Source/Runtime/GeometryCore/Private/CompGeom/PolygonTriangulation.cpp#L29
bool Triangulation::triangulate(const std::vector<vec2i>& inVertexPositions, std::vector<Index3>& indexes)
{
	std::vector<vec2i> vertexPositions(inVertexPositions);
	indexes.clear();

	int32 polygonVertexCount = vertexPositions.size();
	if (polygonVertexCount < 3)
	{
		return false;
	}

	// Compute the signed area of the polygon
	float polySignedArea2 = 0;
	for (int32 i = 0; i < polygonVertexCount; ++i)
	{
		const vec2i& v1 = vertexPositions[i];
		const vec2i& v2 = vertexPositions[(i + 1) % polygonVertexCount];
		polySignedArea2 += v1.x * v2.y - v1.y * v2.x;
	}
	bool isClockwise = polySignedArea2 < 0;
	int32 orientationSign = isClockwise ? -1 : 1;

	if (polygonVertexCount == 3)
	{
		indexes.emplace_back(Index3(0, 1, 2));
		return true;
	}

	// Deconstructed linked list simplified for this use case
	std::vector<int32> prevVertexNumbers, nextVertexNumbers;
	prevVertexNumbers.resize(polygonVertexCount);
	nextVertexNumbers.resize(polygonVertexCount);

	for (int32 vertexNumber = 0; vertexNumber < polygonVertexCount; ++vertexNumber)
	{
		prevVertexNumbers[vertexNumber] = vertexNumber - 1;
		nextVertexNumbers[vertexNumber] = vertexNumber + 1;
	}
	prevVertexNumbers[0] = polygonVertexCount - 1;
	nextVertexNumbers[polygonVertexCount - 1] = 0;

	int32 earVertexNumber = 0;
	int32 earTestCount = 0;

	for (int32 remainingVertexCount = polygonVertexCount; remainingVertexCount >= 3;)
	{
		bool isEar = true;

		if (remainingVertexCount > 3 && earTestCount < remainingVertexCount)
		{
			const vec2i prevVertexPosition = vertexPositions[prevVertexNumbers[earVertexNumber]];
			const vec2i earVertexPosition = vertexPositions[earVertexNumber];
			const vec2i nextVertexPosition = vertexPositions[nextVertexNumbers[earVertexNumber]];

			if (!isTriangleFlipped(orientationSign, prevVertexPosition, earVertexPosition, nextVertexPosition)) 
			{
				// Test the next-next vertex
				int32 testVertexNumber = nextVertexNumbers[nextVertexNumbers[earVertexNumber]];
				do
				{
					vec2i testVertexPosition = vertexPositions[testVertexNumber];
					if (Math::isBarycentric(prevVertexPosition, earVertexPosition, nextVertexPosition, testVertexPosition))
					{
						isEar = false;
						break;
					}
					// Go to the next vertex to test
					testVertexNumber = nextVertexNumbers[testVertexNumber];
				}
				while (testVertexNumber != prevVertexNumbers[earVertexNumber]);
			}
			else
			{
				isEar = false;
			}
		}

		if (isEar)
		{
			int32 a = prevVertexNumbers[earVertexNumber];
			int32 b = earVertexNumber;
			int32 c = nextVertexNumbers[earVertexNumber];
			indexes.emplace_back(Index3(a, b, c));

			nextVertexNumbers[prevVertexNumbers[earVertexNumber]] = nextVertexNumbers[earVertexNumber];
			prevVertexNumbers[nextVertexNumbers[earVertexNumber]] = prevVertexNumbers[earVertexNumber];
			--remainingVertexCount;

			earVertexNumber = prevVertexNumbers[earVertexNumber];

			earTestCount = 0;
		}
		else
		{
			earVertexNumber = nextVertexNumbers[earVertexNumber];
			earTestCount++;
		}
	}

	return true;
}
