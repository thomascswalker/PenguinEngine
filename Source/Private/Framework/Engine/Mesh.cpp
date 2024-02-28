#include "Framework/Engine/Mesh.h"

// Face

Vector3 PFace::GetNormal() const
{
    const Vector3 U = *V1 - *V0;
    const Vector3 V = *V2 - *V1;
    Vector3 N;
    N.X = (U.Y * V.Z) - (U.Z * V.Y);
    N.Y = (U.Z * V.X) - (U.X * V.Z);
    N.Z = (U.X * V.Y) - (U.Y * V.X);
    return N;
}

// Mesh

void PMesh::Empty()
{
    Vertices.clear();
    Indices.clear();
}

PFace PMesh::GetFace(const uint32 Index)
{
    const uint32 StartIndex = Indices.at(Index * 3); // NOLINT
    Vector3* V0 = &Vertices[StartIndex];
    Vector3* V1 = &Vertices[StartIndex + 1];
    Vector3* V2 = &Vertices[StartIndex + 2];
    return PFace{ V0, V1, V2 };
}

std::shared_ptr<PMesh> PMesh::Triangle()
{
    PMesh TriangleMesh;
    TriangleMesh.Vertices = {
        {-0.5f, -0.5f, 0.0f},
        {0.5f, -0.5f, 0.0f},
        {0.0f, 0.5f, 0.0f}
    };
    TriangleMesh.Indices = {
        0, 1, 3,
        1, 2, 3
    };
    return std::make_shared<PMesh>(TriangleMesh);
}
