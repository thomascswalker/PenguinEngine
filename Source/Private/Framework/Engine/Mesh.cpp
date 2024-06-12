#include "Framework/Engine/Mesh.h"

void PMesh::ProcessTriangle(const PTriangle& Triangle, PVertex* V0, PVertex* V1, PVertex* V2) const
{
    V0->Position = Positions[Triangle.PositionIndexes[0]];
    V1->Position = Positions[Triangle.PositionIndexes[1]];
    V2->Position = Positions[Triangle.PositionIndexes[2]];

    V0->Normal = Normals[Triangle.NormalIndexes[0]];
    V1->Normal = Normals[Triangle.NormalIndexes[1]];
    V2->Normal = Normals[Triangle.NormalIndexes[2]];

    V0->TexCoord = TexCoords[Triangle.TexCoordIndexes[0]];
    V1->TexCoord = TexCoords[Triangle.TexCoordIndexes[1]];
    V2->TexCoord = TexCoords[Triangle.TexCoordIndexes[2]];
}

std::shared_ptr<PMesh> PMesh::CreatePlane(float Size)
{
    return CreatePlane(Size, Size);
}

std::shared_ptr<PMesh> PMesh::CreatePlane(float Width, float Height)
{
    std::vector<FVector3> Positions;
    Positions.emplace_back(-Width, 0.0f, Height);
    Positions.emplace_back(Width, 0.0f, Height);
    Positions.emplace_back(-Width, 0.0f, -Height);
    Positions.emplace_back(Width, 0.0f, -Height);

    std::vector<FVector3> Normals;
    Normals.emplace_back(0.0f, 1.0f, 0.0f); // Only one normal because our plane is flat

    std::vector<FVector2> TexCoords;
    TexCoords.emplace_back(0.0f, 0.0f);
    TexCoords.emplace_back(1.0f, 0.0f);
    TexCoords.emplace_back(0.0f, 0.0f);
    TexCoords.emplace_back(1.0f, 0.0f);
    TexCoords.emplace_back(0.0f, 0.0f);
    TexCoords.emplace_back(1.0f, 0.0f);
    TexCoords.emplace_back(0.0f, 1.0f);
    TexCoords.emplace_back(1.0f, 1.0f);

    std::vector<PTriangle> Triangles;
    Triangles.emplace_back(
        std::vector{2, 0, 3}, // Position indexes
        std::vector{0, 0, 0}, // Normal indexes
        std::vector{6, 4, 7}  // TexCoord indexes
    );
    Triangles.emplace_back(
        std::vector{1, 3, 0}, // Position indexes
        std::vector{0, 0, 0}, // Normal indexes
        std::vector{5, 7, 4}  // TexCoord indexes
    );
    return std::make_shared<PMesh>(Triangles, Positions, Normals, TexCoords);
}
