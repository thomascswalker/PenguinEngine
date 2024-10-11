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
