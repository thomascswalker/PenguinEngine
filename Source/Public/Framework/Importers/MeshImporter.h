#pragma once

#include <filesystem>
#include <fstream>
#include <string>

#include "Framework/Core/IO.h"
#include "Framework/Engine/Mesh.h"
#include "Framework/Core/String.h"

class ObjImporter
{
    /**
     * @brief Parses a line representing a 2D vector and adds it to the given vector.
     * 
     * @param Line The line to parse.
     * @param V The vector to add the parsed vector to.
     * 
     * @throws std::invalid_argument If the line does not contain enough components.
     */
    static void ParseVector2(const std::string& Line, std::vector<FVector2>* V)
    {
        // Split the line into its components
        std::vector<std::string> Components;
        Strings::Split(Line, Components, " ");

        // Check if the line contains enough components
        if (Components.size() < 3)
        {
            throw std::invalid_argument("Line does not contain enough components");
        }

        // Create a new FVector3 from the components and add it to the vector
        V->emplace_back(std::stof(Components[1]),
                        std::stof(Components[2]));
    }

    /**
     * @brief Parses a line representing a 3D vector and adds it to the given vector.
     * 
     * @param Line The line to parse.
     * @param V The vector to add the parsed vector to.
     * 
     * @throws std::invalid_argument If the line does not contain enough components.
     */
    static void ParseVector3(const std::string& Line, std::vector<FVector3>* V)
    {
        // Split the line into its components
        std::vector<std::string> Components;
        Strings::Split(Line, Components, " ");

        // Check if the line contains enough components
        if (Components.size() < 4)
        {
            throw std::invalid_argument("Line does not contain enough components");
        }

        // Create a new FVector3 from the components and add it to the vector
        V->emplace_back(std::stof(Components[1]),
                        std::stof(Components[2]),
                        std::stof(Components[3]));
    }

    /**
     * @brief Parses a face line from an OBJ file and populates the given vectors with the corresponding index values.
     * 
     * @param Line The face line from the OBJ file.
     * @param PositionIndexes The vector to store position index values.
     * @param NormalIndexes The vector to store normal index values.
     * @param TexCoordIndexes The vector to store texture coordinate index values.
     * @throws std::runtime_error If the index format is invalid.
     */
    static void ParseFace(const std::string& Line,
                          std::vector<int32>* PositionIndexes,
                          std::vector<int32>* NormalIndexes,
                          std::vector<int32>* TexCoordIndexes)
    {
        std::vector<std::string> IndexComponents;

        // Split the face line by spaces
        Strings::Split(Line, IndexComponents, " ");     // [f, v/vt/vn, v/vt/vn, v/vt/vn]
        IndexComponents.erase(IndexComponents.begin()); // Remove 'f' from vector, [v/vt/vn, v/vt/vn, v/vt/vn]

        // For each index group...
        for (const std::string& IndexGroup : IndexComponents) // [v/vt/vn]
        {
            std::vector<std::string> ComponentGroup;
            Strings::Split(IndexGroup, ComponentGroup, "/"); // [v, vt, vn]

            // Determine the number of components in the index group
            switch (ComponentGroup.size())
            {
            case 1 : // [v]
                {
                    PositionIndexes->emplace_back(std::stoi(IndexGroup) - 1);
                    break;
                }
            case 2 : // [v/vn]
                {
                    PositionIndexes->emplace_back(std::stoi(ComponentGroup[0]) - 1);
                    NormalIndexes->emplace_back(std::stoi(ComponentGroup[1]) - 1);
                    break;
                }
            case 3 : // [v/vt/vn]
                {
                    PositionIndexes->emplace_back(std::stoi(ComponentGroup[0]) - 1);
                    TexCoordIndexes->emplace_back(std::stoi(ComponentGroup[1]) - 1);
                    NormalIndexes->emplace_back(std::stoi(ComponentGroup[2]) - 1);
                    break;
                }
            default :
                throw std::runtime_error("Invalid index format.");
            }
        }
    }

public:
    /**
     * @brief Import a mesh from an OBJ file.
     * 
     * @param FileName The path to the OBJ file.
     * @param Mesh The mesh object to fill.
     * @return true if the import is successful, false otherwise.
     */
    static bool Import(const std::string& FileName, PMesh* Mesh)
    {
        // Read the file into a buffer
        std::string Buffer;
        if (!IO::ReadFile(FileName, Buffer))
        {
            LOG_ERROR("Unable to read file {}", FileName)
            return false;
        }

        // Create a string stream from the buffer
        std::stringstream Stream;
        Stream << Buffer.data();

        // Initialize vectors to store mesh data
        std::vector<FVector3> Positions;
        std::vector<FVector3> Normals;
        std::vector<FVector2> TexCoords;

        std::vector<PTriangle> Triangles;
        int32 TriangleCount = 0;
        std::vector<uint32> PositionIndexes;
        std::vector<uint32> NormalIndexes;
        std::vector<uint32> TexCoordIndexes;

        // Process each line in the file
        while (Stream.peek() != -1)
        {
            std::string Line;
            IO::ReadLine(Stream, Line);

            // Skip empty lines and comments
            if (Line.starts_with('\0') || Line.starts_with('#'))
            {
                continue;
            }

            const char Token = *Line.c_str();
            switch (Token)
            {
            case 'v' : // Parse vertex positions, normals, and texture coordinates
                {
                    if (Line.starts_with("vn"))
                    {
                        ParseVector3(Line, &Normals);
                    }
                    else if (Line.starts_with("vt"))
                    {
                        ParseVector2(Line, &TexCoords);
                    }
                    else if (Line.starts_with("v"))
                    {
                        ParseVector3(Line, &Positions);
                    }
                    else
                    {
                        LOG_ERROR("Failed to parse line: {}", Line)
                        return false;
                    }
                    break;
                }

            case 'f' : // Parse face indices
                {
                    PTriangle T;
                    ParseFace(Line, &T.PositionIndexes, &T.NormalIndexes, &T.TexCoordIndexes);
                    Triangles.emplace_back(T);
                    TriangleCount++;
                    break;
                }
            default :
                LOG_WARNING("Token {} is either invalid or not implemented. (Line: {})", Token, *Line.c_str())
                break;
            }
        }

        Mesh->Triangles = Triangles;
        Mesh->Positions = Positions;
        Mesh->Normals = Normals;
        Mesh->TexCoords = TexCoords;

        return true;
    }
};
