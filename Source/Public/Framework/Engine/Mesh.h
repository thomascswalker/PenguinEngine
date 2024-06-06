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

struct PVertex
{
    FVector3 Position;
    FVector3 Normal;
    FVector2 TexCoord;

    explicit PVertex(const FVector3& InPosition = FVector3::ZeroVector(),
                     const FVector3& InNormal = FVector3::ZeroVector(),
                     const FVector3& InTexCoord = FVector3::ZeroVector())
        : Position(InPosition), Normal(InNormal), TexCoord(InTexCoord)
    {
    }
};

struct PMesh : PObject
{
protected:
    uint32 AddVertex(const FVector3& Position, const FVector3& Normal, const FVector3& TexCoord);

public:
    // Properties
    std::vector<PVertex> Vertexes;
    std::vector<uint32> PositionIndexes;
    std::vector<uint32> NormalIndexes;
    std::vector<uint32> TexCoordIndexes;

    PMesh()
    {
    }
    PMesh(const std::vector<PVertex>& InVertexes, const std::vector<uint32>& InIndexes) : Vertexes(InVertexes), PositionIndexes(InIndexes)
    {
    }
    PMesh(const std::vector<FVector3>& InPositions)
    {
        for (const auto P : InPositions)
        {
            Vertexes.emplace_back(P);
        }
    }

    // Mesh Functions
    void AddTri(const FVector3& InV0, const FVector3& InV1, const FVector3& InV2);
    void AddQuad(const FVector3& V0, const FVector3& V1, const FVector3& V2, const FVector3& V3);
    void Empty();
    uint32 GetTriCount() const { return static_cast<uint32>(PositionIndexes.size()) / 3; }
    FVector3* GetVertexPosition(const uint32 Index) { return &Vertexes[Index].Position; }
    constexpr uint32 GetVertexCount() const { return static_cast<uint32>(Vertexes.size()); }

    bool HasNormals() const { return !NormalIndexes.empty(); }
    bool HasTexCoords() const { return !TexCoordIndexes.empty(); }

    // Primitives
    static std::shared_ptr<PMesh> CreateTriangle(float Scale);
    static std::shared_ptr<PMesh> CreatePlane(float Size);
    static std::shared_ptr<PMesh> CreatePlane(float Width, float Height);
    static std::shared_ptr<PMesh> CreateSphere(float Radius, int32 Segments = 8);
    static std::shared_ptr<PMesh> CreateCube(float Scale);
    static std::shared_ptr<PMesh> CreateTeapot(float Scale);
    static std::shared_ptr<PMesh> CreateCone();
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
