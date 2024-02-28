#pragma once

#include <vector>
#include "Math/Vector.h"

struct PFace
{
    Vector3* V0;
    Vector3* V1;
    Vector3* V2;

    Vector3 GetNormal() const;

    PFace(Vector3* InV0, Vector3* InV1, Vector3* InV2) : V0(InV0), V1(InV1), V2(InV2) {}
};

struct PMesh
{
protected:
    // Properties
    std::vector<Vector3> Vertices;
    std::vector<uint32> Indices;

public:
    // Functions
    void Empty();
    PFace GetFace(const uint32 Index);
    Vector3* GetVertexPosition(const uint32 Index) { return &Vertices[Index]; }
    constexpr uint32 GetVertexCount() const { return static_cast<uint32>(Vertices.size()); }

    // Primitives
    static std::shared_ptr<PMesh> Triangle();
};
