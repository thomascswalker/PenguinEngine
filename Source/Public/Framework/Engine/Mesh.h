#pragma once

#include <vector>
#include "Math/Vector.h"

enum EPrimitiveType
{
    Plane,
    Cube,
    Sphere,
    Torus
};

struct PTri
{
    PVector3& V0;
    PVector3& V1;
    PVector3& V2;

    PVector3 GetNormal() const;

    PTri(PVector3& InV0, PVector3& InV1, PVector3& InV2) : V0(InV0), V1(InV1), V2(InV2)
    {
    }
};

struct PMesh
{
protected:

    uint32 AddVertex(const PVector3& V);
    void AddTri(const PVector3& InV0, const PVector3& InV1, const PVector3& InV2);
    void AddQuad(const PVector3& V0, const PVector3& V1, const PVector3& V2, const PVector3& V3);

public:
    // Properties
    std::vector<PVector3> Vertices;
    std::vector<uint32> Indices;
    
    // Functions
    void Empty();
    PTri GetTri(uint32 Index) const;
    uint32 GetTriCount() const { return static_cast<uint32>(Indices.size()) / 3; }
    PVector3* GetVertexPosition(const uint32 Index) { return &Vertices[Index]; }
    constexpr uint32 GetVertexCount() const { return static_cast<uint32>(Vertices.size()); }

    // Primitives
    static std::shared_ptr<PMesh> CreateTriangle(float Scale);
    static std::shared_ptr<PMesh> CreatePlane(float Width);
    static std::shared_ptr<PMesh> CreatePlane(float Width, float Height);
};
