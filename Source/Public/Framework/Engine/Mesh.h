#pragma once

#include <vector>
#include "Math/MathCommon.h"

enum EPrimitiveType
{
    Plane,
    Cube,
    Sphere,
    Torus
};

struct PTri
{
    FVector3& V0;
    FVector3& V1;
    FVector3& V2;

    FVector3 GetNormal() const;

    PTri(FVector3& InV0, FVector3& InV1, FVector3& InV2) : V0(InV0), V1(InV1), V2(InV2)
    {
    }
};

struct PMesh
{
protected:
    uint32 AddVertex(const FVector3& V);

public:
    // Properties
    std::vector<FVector3> Vertices;
    std::vector<uint32> Indices;

    PMesh()
    {
    }
    PMesh(const std::vector<FVector3>& InVertices, const std::vector<uint32>& InIndices) : Vertices(InVertices), Indices(InIndices)
    {
    }

    // Functions
    void AddTri(const FVector3& InV0, const FVector3& InV1, const FVector3& InV2);
    void AddQuad(const FVector3& V0, const FVector3& V1, const FVector3& V2, const FVector3& V3);
    void Empty();
    PTri GetTri(uint32 Index) const;
    uint32 GetTriCount() const { return static_cast<uint32>(Indices.size()) / 3; }
    FVector3* GetVertexPosition(const uint32 Index) { return &Vertices[Index]; }
    constexpr uint32 GetVertexCount() const { return static_cast<uint32>(Vertices.size()); }
    static EWindingOrder GetWindingOrder(const FVector3& V0, const FVector3& V1, const FVector3& V2);

    // Primitives
    static std::shared_ptr<PMesh> CreateTriangle(float Scale);
    static std::shared_ptr<PMesh> CreatePlane(float Size);
    static std::shared_ptr<PMesh> CreatePlane(float Width, float Height);
    static std::shared_ptr<PMesh> CreateSphere(float Radius, float Segments = 8);
};

namespace Math
{
    static FVector3 GetNormal(const FVector3& V0, const FVector3& V1, const FVector3& V2)
    {
        const FVector3 U = V1 - V0;
        const FVector3 V = V2 - V1;
        
        FVector3 N;
        N.X = (U.Y * V.Z) - (U.Z * V.Y);
        N.Y = (U.Z * V.X) - (U.X * V.Z);
        N.Z = (U.X * V.Y) - (U.Y * V.X);
        
        return N;
    }

}