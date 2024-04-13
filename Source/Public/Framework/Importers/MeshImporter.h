#pragma once

#include <filesystem>
#include <fstream>
#include <string>

#include "Framework/Core/IO.h"
#include "Framework/Engine/Mesh.h"
#include "Framework/Core/String.h"

class ObjImporter
{
    static void ParseVertex(const std::string& Line, std::vector<FVector3>& Vertices)
    {
        std::vector<std::string> StringPositions;

        Strings::Split(Line, StringPositions, " ");

        Vertices.emplace_back(std::stof(StringPositions[1]),
                              std::stof(StringPositions[2]),
                              std::stof(StringPositions[3]));
    }
    static void ParseFace(const std::string& Line, std::vector<uint32>& Indices)
    {
        std::vector<std::string> StringIndices;

        Strings::Split(Line, StringIndices, " ");

        Indices.emplace_back(std::stoi(StringIndices[1]) - 1);
        Indices.emplace_back(std::stoi(StringIndices[2]) - 1);
        Indices.emplace_back(std::stoi(StringIndices[3]) - 1);
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
        std::vector<uint32> Indices;

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
                    ParseVertex(Line, Positions);
                    break;
                }
            case 'f' :
                {
                    ParseFace(Line, Indices);
                    break;
                }
            default :
                Logging::Warning("Token %c is either invalid or not implemented. (Line: %s)", Token, *Line.c_str());
                break;
            }
        }

        Mesh->Positions = Positions;
        Mesh->Indices = Indices;

        return true;
    }
};
