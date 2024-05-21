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
    std::vector<FVector3> VertexPositions;
    std::vector<uint32> VertexPositionIndexes;
    
    std::vector<FVector3> VertexNormals;
    std::vector<uint32> VertexNormalIndexes;

    std::vector<FVector3> VertexTexCoords;
    std::vector<uint32> VertexTexCoordIndexes;

    PMesh()
    {
    }
    PMesh(const std::vector<FVector3>& InPositions, const std::vector<uint32>& InIndices) : VertexPositionIndexes(InIndices)
    {
        for (const FVector3& Position : InPositions)
        {
            VertexPositions.emplace_back(Position);
        }
    }

    // Mesh Functions
    void AddTri(const FVector3& InV0, const FVector3& InV1, const FVector3& InV2);
    void AddQuad(const FVector3& V0, const FVector3& V1, const FVector3& V2, const FVector3& V3);
    void Empty();
    uint32 GetTriCount() const { return static_cast<uint32>(VertexPositionIndexes.size()) / 3; }
    FVector3* GetVertexPosition(const uint32 Index) { return &VertexPositions[Index]; }
    constexpr uint32 GetVertexCount() const { return static_cast<uint32>(VertexPositions.size()); }

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