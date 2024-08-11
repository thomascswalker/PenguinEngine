#include "Framework/Engine/Mesh.h"


void Mesh::processTriangles()
{
	for (Triangle& triangle : m_triangles)
	{
		triangle.m_v0.m_position = m_positions[triangle.m_positionIndexes[0]];
		triangle.m_v1.m_position = m_positions[triangle.m_positionIndexes[1]];
		triangle.m_v2.m_position = m_positions[triangle.m_positionIndexes[2]];

		triangle.m_v0.m_normal = m_normals[triangle.m_normalIndexes[0]];
		triangle.m_v1.m_normal = m_normals[triangle.m_normalIndexes[1]];
		triangle.m_v2.m_normal = m_normals[triangle.m_normalIndexes[2]];

		triangle.m_v0.m_texCoord = m_texCoords[triangle.m_texCoordIndexes[0]];
		triangle.m_v1.m_texCoord = m_texCoords[triangle.m_texCoordIndexes[1]];
		triangle.m_v2.m_texCoord = m_texCoords[triangle.m_texCoordIndexes[2]];
	}
}

std::shared_ptr<Mesh> Mesh::createPlane(const float size)
{
	return createPlane(size, size);
}

std::shared_ptr<Mesh> Mesh::createPlane(float width, float height)
{
	std::vector<vec3f> positions;
	positions.emplace_back(-width, 0.0f, height);
	positions.emplace_back(width, 0.0f, height);
	positions.emplace_back(-width, 0.0f, -height);
	positions.emplace_back(width, 0.0f, -height);

	std::vector<vec3f> normals;
	normals.emplace_back(0.0f, 1.0f, 0.0f); // Only one normal because our plane is flat

	std::vector<vec2f> texCoords;
	texCoords.emplace_back(0.0f, 0.0f);
	texCoords.emplace_back(1.0f, 0.0f);
	texCoords.emplace_back(0.0f, 0.0f);
	texCoords.emplace_back(1.0f, 0.0f);
	texCoords.emplace_back(0.0f, 0.0f);
	texCoords.emplace_back(1.0f, 0.0f);
	texCoords.emplace_back(0.0f, 1.0f);
	texCoords.emplace_back(1.0f, 1.0f);

	std::vector<Triangle> triangles;
	triangles.emplace_back(
		std::vector{2, 0, 3}, // Position indexes
		std::vector{0, 0, 0}, // Normal indexes
		std::vector{6, 4, 7} // TexCoord indexes
	);
	triangles.emplace_back(
		std::vector{1, 3, 0}, // Position indexes
		std::vector{0, 0, 0}, // Normal indexes
		std::vector{5, 7, 4} // TexCoord indexes
	);
	return std::make_shared<Mesh>(triangles, positions, normals, texCoords);
}
