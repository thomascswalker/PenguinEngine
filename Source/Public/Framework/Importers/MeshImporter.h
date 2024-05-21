#pragma once

#include <filesystem>
#include <fstream>
#include <string>

#include "Framework/Core/IO.h"
#include "Framework/Engine/Mesh.h"
#include "Framework/Core/String.h"

class ObjImporter
{
    static void ParseVector(const std::string& Line, std::vector<FVector3>* Normals)
    {
        std::vector<std::string> Components;

        Strings::Split(Line, Components, " ");

        Normals->emplace_back(std::stof(Components[1]),
                              std::stof(Components[2]),
                              std::stof(Components[3]));
    }

    static void ParseFace(const std::string& Line,
                          std::vector<uint32>* PositionIndexes,
                          std::vector<uint32>* NormalIndexes,
                          std::vector<uint32>* TexCoordIndexes)
    {
        std::vector<std::string> IndexComponents;

        // Split by spaces
        Strings::Split(Line, IndexComponents, " "); // [f, v/vt/vn, v/vt/vn, v/vt/vn]
        IndexComponents.erase(IndexComponents.begin()); // Remove 'f' from vector, [v/vt/vn, v/vt/vn, v/vt/vn]

        // For each index group...
        for (const std::string& IndexGroup : IndexComponents) // [v/vt/vn]
        {
            std::vector<std::string> ComponentGroup;
            Strings::Split(IndexGroup, ComponentGroup, "/"); // [v, vt, vn]

            // https://paulbourke.net/dataformats/obj/
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
    static bool Import(const std::string& FileName, PMesh* Mesh)
    {
        std::string Buffer;
        if (!IO::ReadFile(FileName, Buffer))
        {
            return false;
        }

        std::stringstream Stream;
        Stream << Buffer.data();

        std::vector<FVector3> Positions;
        std::vector<FVector3> Normals;
        std::vector<FVector3> TexCoords;

        std::vector<uint32> PositionIndexes;
        std::vector<uint32> NormalIndexes;
        std::vector<uint32> TexCoordIndexes;

        while (Stream.peek() != -1)
        {
            std::string Line;
            IO::ReadLine(Stream, Line);

            if (Line.starts_with('\0') || Line.starts_with('#'))
            {
                continue;
            }

            const char Token = *Line.c_str();
            switch (Token)
            {
            case 'v' :
                {
                    if (Line.starts_with("vn"))
                    {
                        ParseVector(Line, &Normals);
                    }
                    else if (Line.starts_with("vt"))
                    {
                        ParseVector(Line, &TexCoords);
                    }
                    else
                    {
                        ParseVector(Line, &Positions);
                    }
                    break;
                }
            case 'f' :
                {
                    ParseFace(Line, &PositionIndexes, &NormalIndexes, &TexCoordIndexes);
                    break;
                }
            default :
                Logging::Warning("Token %c is either invalid or not implemented. (Line: %s)", Token, *Line.c_str());
                break;
            }
        }

        Mesh->VertexPositions = Positions;
        Mesh->VertexPositionIndexes = PositionIndexes;

        Mesh->VertexNormals = Normals;
        Mesh->VertexNormalIndexes = NormalIndexes;

        Mesh->VertexTexCoords = TexCoords;
        Mesh->VertexTexCoordIndexes = TexCoordIndexes;

        return true;
    }
};
