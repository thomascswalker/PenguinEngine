#pragma once

#include <vector>
#include "Object.h"

enum EPrimitiveType
{
    Plane,
    Cube,
    Sphere,
    Torus
};
//
// struct PVertex
// {
//     FVector3 Position;
//     FVector3 Normal;
//     FVector2 TexCoord;
//
//     explicit PVertex(const FVector3& InPosition = FVector3::ZeroVector(),
//                      const FVector3& InNormal = FVector3::ZeroVector(),
//                      const FVector3& InTexCoord = FVector3::ZeroVector())
//         : Position(InPosition), Normal(InNormal), TexCoord(InTexCoord)
//     {
//     }
// };

struct PVertex
{
    FVector3 Position;
    FVector3 Normal;
    FVector3 TexCoord;

    PVertex()
    {
    }
    PVertex(const FVector3& InPosition, const FVector3& InNormal, const FVector3& InTexCoord)
        : Position(InPosition),
          Normal(InNormal),
          TexCoord(InTexCoord)
    {
    }
};

struct PTriangle
{
    std::vector<int32> PositionIndexes;
    std::vector<int32> NormalIndexes;
    std::vector<int32> TexCoordIndexes;

    PTriangle()
    {
    }
    PTriangle(const std::vector<int32>& InPositionIndexes, const std::vector<int32>& InNormalIndexes, const std::vector<int32>& InTexCoordIndexes)
        : PositionIndexes(InPositionIndexes),
          NormalIndexes(InNormalIndexes),
          TexCoordIndexes(InTexCoordIndexes)
    {
    }
};

struct PMesh : PObject
{
protected:
    uint32 AddVertex(const FVector3& Position, const FVector3& Normal, const FVector3& TexCoord) { return 0; }

public:
    // Properties
    std::vector<PTriangle> Triangles;
    std::vector<FVector3> Positions;
    std::vector<FVector3> Normals;
    std::vector<FVector2> TexCoords;
    

    PMesh()
    {
    }
    PMesh(const std::vector<PTriangle>& InTriangles, const std::vector<FVector3>& InPositions, const std::vector<FVector3>& InNormals = {}, const std::vector<FVector2>& InTexCoords = {})
        : Triangles(InTriangles),
          Positions(InPositions),
          Normals(InNormals),
          TexCoords(InTexCoords)
    {
    }

    void ComputeNormals()
    {
        // Normals.clear();
        // std::vector<FVector3> TriNormals;
        // std::vector<FVector3> TriNormalsWeighted;
        //
        // for (const auto& Triangle : Triangles)
        // {
        //     FVector3 V0 = Positions[Triangle.PositionIndexes[0]];
        //     FVector3 V1 = Positions[Triangle.PositionIndexes[1]];
        //     FVector3 V2 = Positions[Triangle.PositionIndexes[2]];
        //
        //     FVector3 N = (V1 - V0).Cross(V2 - V0);
        //     TriNormals.emplace_back(N);
        // }
    }
    void ProcessTriangle(const PTriangle& Triangle, PVertex* V0, PVertex* V1, PVertex* V2) const;

    // Primitives
    static std::shared_ptr<PMesh> CreatePlane(float Size);
    static std::shared_ptr<PMesh> CreatePlane(float Width, float Height);
};

namespace Math
{
    static EWindingOrder GetWindingOrder(const FVector3& V0, const FVector3& V1, const FVector3& V2)
    {
        const float Result = (V1.X - V0.X) * (V2.Y - V0.Y) - (V2.X - V0.X) * (V1.Y - V0.Y);
        if (Result > 0)
        {
            return EWindingOrder::CCW;
        }
        if (Result < 0)
        {
            return EWindingOrder::CW;
        }
        return EWindingOrder::CL;
    }
}
