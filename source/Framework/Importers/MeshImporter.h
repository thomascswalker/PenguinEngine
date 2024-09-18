#pragma once

#include <filesystem>
#include <string>

#include "Framework/Core/IO.h"
#include "Framework/Engine/Mesh.h"
#include "Framework/Core/String.h"

class ObjImporter
{
	/**
	 * @brief Parses a line representing a 2D vector and adds it to the given vector.
	 *
	 * @param line The line to parse.
	 * @param v The vector to add the parsed vector to.
	 *
	 * @throws std::invalid_argument If the line does not contain enough components.
	 */
	static void parseVec2(const std::string& line, std::vector<vec2f>* v)
	{
		// Split the line into its components
		std::vector<std::string> components;
		Strings::split(line, components, " ");

		// Check if the line contains enough components
		if (components.size() < 3)
		{
			throw std::invalid_argument("Line does not contain enough components");
		}

		// Create a new vec3f from the components and add it to the vector
		v->emplace_back(std::stof(components[1]),
		                std::stof(components[2]));
	}

	/**
	 * @brief Parses a line representing a 3D vector and adds it to the given vector.
	 *
	 * @param line The line to parse.
	 * @param v The vector to add the parsed vector to.
	 *
	 * @throws std::invalid_argument If the line does not contain enough components.
	 */
	static void parseVec3(const std::string& line, std::vector<vec3f>* v)
	{
		// Split the line into its components
		std::vector<std::string> components;
		Strings::split(line, components, " ");

		// Check if the line contains enough components
		if (components.size() < 4)
		{
			throw std::invalid_argument("Line does not contain enough components");
		}

		// Create a new vec3f from the components and add it to the vector
		v->emplace_back(std::stof(components[1]),
		                std::stof(components[2]),
		                std::stof(components[3]));
	}

	/**
	 * @brief Parses a face line from an OBJ file and populates the given vectors with the corresponding index values.
	 *
	 * @param line The face line from the OBJ file.
	 * @param positionIndexes The vector to store position index values.
	 * @param normalIndexes The vector to store normal index values.
	 * @param texCoordIndexes The vector to store texture coordinate index values.
	 * @throws std::runtime_error If the index format is invalid.
	 */
	static void parseFace(const std::string& line,
	                      std::vector<int32>* positionIndexes,
	                      std::vector<int32>* normalIndexes,
	                      std::vector<int32>* texCoordIndexes)
	{
		std::vector<std::string> indexComponents;

		// Split the face line by spaces
		Strings::split(line, indexComponents, " ");     // [f, v/vt/vn, v/vt/vn, v/vt/vn]
		indexComponents.erase(indexComponents.begin()); // remove 'f' from vector, [v/vt/vn, v/vt/vn, v/vt/vn]

		// For each index group...
		for (const std::string& indexGroup : indexComponents) // [v/vt/vn]
		{
			std::vector<std::string> componentGroup;
			Strings::split(indexGroup, componentGroup, "/"); // [v, vt, vn]

			// Determine the number of components in the index group
			switch (componentGroup.size())
			{
			case 1: // [v]
				{
					positionIndexes->emplace_back(std::stoi(indexGroup) - 1);
					break;
				}
			case 2: // [v/vn]
				{
					positionIndexes->emplace_back(std::stoi(componentGroup[0]) - 1);
					normalIndexes->emplace_back(std::stoi(componentGroup[1]) - 1);
					break;
				}
			case 3: // [v/vt/vn]
				{
					positionIndexes->emplace_back(std::stoi(componentGroup[0]) - 1);
					texCoordIndexes->emplace_back(std::stoi(componentGroup[1]) - 1);
					normalIndexes->emplace_back(std::stoi(componentGroup[2]) - 1);
					break;
				}
			default: throw std::runtime_error("Invalid index format.");
			}
		}
	}

public:
	/**
	 * @brief Import a mesh from an OBJ file.
	 *
	 * @param fileName The path to the OBJ file.
	 * @param mesh The mesh object to fill.
	 * @return true if the import is successful, false otherwise.
	 */
	static bool import(const std::string& fileName, Mesh* mesh)
	{
		// Read the file into a buffer
		std::string buffer;
		if (!IO::readFile(fileName, buffer))
		{
			LOG_ERROR("Unable to read file {}", fileName)
			return false;
		}

		// Create a string stream from the buffer
		std::stringstream stream;
		stream << buffer.data();

		// Initialize vectors to store mesh data
		std::vector<vec3f> positions;
		std::vector<vec3f> normals;
		std::vector<vec2f> texCoords;

		std::vector<Triangle> triangles;
		int32 triangleCount = 0;
		std::vector<uint32> positionIndexes;
		std::vector<uint32> normalIndexes;
		std::vector<uint32> texCoordIndexes;

		// Process each line in the file
		while (stream.peek() != -1)
		{
			std::string line;
			IO::readLine(stream, line);

			// Skip empty m_lines and comments
			if (line.starts_with('\0') || line.starts_with('#'))
			{
				continue;
			}

			const char token = *line.c_str();
			switch (token)
			{
			case 'v': // Parse vertex positions, normals, and texture coordinates
				{
					if (line.starts_with("vn"))
					{
						parseVec3(line, &normals);
					}
					else if (line.starts_with("vt"))
					{
						parseVec2(line, &texCoords);
					}
					else if (line.starts_with("v"))
					{
						parseVec3(line, &positions);
					}
					else
					{
						LOG_ERROR("Failed to parse line: {}", line)
						return false;
					}
					break;
				}

			case 'f': // Parse face indices
				{
					Triangle triangle;
					parseFace(line, &triangle.positionIndexes, &triangle.normalIndexes,
					          &triangle.texCoordIndexes);
					triangles.emplace_back(triangle);
					triangleCount++;
					break;
				}
			case '\0':
				{
					break;
				}
			case 'o':
			case 'g':
			case 's':
				LOG_WARNING("Token {} is not implemented (Line: {}", token, *line.c_str());
				break;
			default:
				{
					LOG_WARNING("Token {} is invalid (Line: {})", token, *line.c_str())
					break;
				}
			}
		}

		mesh->m_triangles = triangles;
		mesh->m_positions = positions;
		if (!normals.empty())
		{
			mesh->m_normals = normals;
		}
		if (!texCoords.empty())
		{
			mesh->m_texCoords = texCoords;
		}

		return true;
	}
};
