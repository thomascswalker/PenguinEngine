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

struct PMesh : PObject
{
protected:
    uint32 AddVertex(const FVector3& V);

public:
    // Properties
    std::vector<FVector3> Positions;
    std::vector<FVector3> Normals;
    std::vector<uint32> Indices;

    PMesh()
    {
    }
    PMesh(const std::vector<FVector3>& InPositions, const std::vector<uint32>& InIndices) : Indices(InIndices)
    {
        for (const FVector3& Position : InPositions)
        {
            Positions.emplace_back(Position);
        }
    }

    // Mesh Functions
    void AddTri(const FVector3& InV0, const FVector3& InV1, const FVector3& InV2);
    void AddQuad(const FVector3& V0, const FVector3& V1, const FVector3& V2, const FVector3& V3);
    void Empty();
    uint32 GetTriCount() const { return static_cast<uint32>(Indices.size()) / 3; }
    FVector3* GetVertexPosition(const uint32 Index) { return &Positions[Index]; }
    constexpr uint32 GetVertexCount() const { return static_cast<uint32>(Positions.size()); }

    // Primitives
    static std::shared_ptr<PMesh> CreateTriangle(float Scale);
    static std::shared_ptr<PMesh> CreatePlane(float Size);
    static std::shared_ptr<PMesh> CreatePlane(float Width, float Height);
    static std::shared_ptr<PMesh> CreateSphere(float Radius, int32 Segments = 8);
    static std::shared_ptr<PMesh> CreateCube(float Scale);
    static std::shared_ptr<PMesh> CreateTeapot(float Scale);
};

namespace Math
{
    static EWindingOrder GetWindingOrder(const FVector3& V0, const FVector3& V1, const FVector3& V2);
}