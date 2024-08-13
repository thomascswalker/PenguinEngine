#include "Framework/Engine/Mesh.h"

void Mesh::processTriangles()
{
	for (Triangle& triangle : m_triangles)
	{
		triangle.v0.position = m_positions[triangle.positionIndexes[0]];
		triangle.v1.position = m_positions[triangle.positionIndexes[1]];
		triangle.v2.position = m_positions[triangle.positionIndexes[2]];

		triangle.v0.normal = m_normals[triangle.normalIndexes[0]];
		triangle.v1.normal = m_normals[triangle.normalIndexes[1]];
		triangle.v2.normal = m_normals[triangle.normalIndexes[2]];

		triangle.v0.texCoord = m_texCoords[triangle.texCoordIndexes[0]];
		triangle.v1.texCoord = m_texCoords[triangle.texCoordIndexes[1]];
		triangle.v2.texCoord = m_texCoords[triangle.texCoordIndexes[2]];
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
		std::vector{6, 4, 7}  // TexCoord indexes
	);
	triangles.emplace_back(
		std::vector{1, 3, 0}, // Position indexes
		std::vector{0, 0, 0}, // Normal indexes
		std::vector{5, 7, 4}  // TexCoord indexes
	);
	return std::make_shared<Mesh>(triangles, positions, normals, texCoords);
}
