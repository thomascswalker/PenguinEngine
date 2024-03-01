#include "Framework/Engine/Mesh.h"

#include "Framework/Core/Logging.h"

// Triangle

PVector3 PTri::GetNormal() const
{
    const PVector3 U = V1 - V0;
    const PVector3 V = V2 - V1;
    PVector3 N;
    N.X = (U.Y * V.Z) - (U.Z * V.Y);
    N.Y = (U.Z * V.X) - (U.X * V.Z);
    N.Z = (U.X * V.Y) - (U.Y * V.X);
    return N;
}

// Mesh

uint32 PMesh::AddVertex(const PVector3& V)
{
    auto Iter = std::ranges::find(Vertices, V);
    if (Iter == Vertices.end())
    {
        // Add new vert
        Vertices.emplace_back(V);
        return Vertices.size() - 1; // NOLINT
    }

    // Get index and return pointer of existing vert
    return std::distance(Vertices.begin(), Iter); // NOLINT
}

void PMesh::AddTri(const PVector3& InV0, const PVector3& InV1, const PVector3& InV2)
{
    uint32 V0Idx = AddVertex(InV0);
    uint32 V1Idx = AddVertex(InV1);
    uint32 V2Idx = AddVertex(InV2);

    Indices.emplace_back(V0Idx);
    Indices.emplace_back(V1Idx);
    Indices.emplace_back(V2Idx);
}

void PMesh::AddQuad(const PVector3& V0, const PVector3& V1, const PVector3& V2, const PVector3& V3)
{
    AddTri(V2, V0, V1);
    AddTri(V0, V2, V3);
}

void PMesh::Empty()
{
    Vertices.clear();
    Indices.clear();
}

PTri PMesh::GetTri(const uint32 Index) const
{
    const uint32 StartIndex = Index * 3;
    PVector3 V0 = Vertices[Indices.at(StartIndex)];
    PVector3 V1 = Vertices[Indices.at(StartIndex + 1)];
    PVector3 V2 = Vertices[Indices.at(StartIndex + 2)];
    return PTri(V0, V1, V2);
}

std::shared_ptr<PMesh> PMesh::CreateTriangle(float Scale)
{
    PMesh Mesh;
    Mesh.AddTri(
        {-Scale, -Scale, 0.0f},
        {Scale, -Scale, 0.0f},
        {0.0f, Scale, 0.0f}
    );
    return std::make_shared<PMesh>(Mesh);
}

std::shared_ptr<PMesh> PMesh::CreatePlane(float Size)
{
    return CreatePlane(Size, Size);
}

std::shared_ptr<PMesh> PMesh::CreatePlane(float Width, float Height)
{
    PMesh Mesh;
    Mesh.AddQuad(
        {-Width, -Height, 0.0f},
        {Width, -Height, 0.0f},
        {Width, Height, 0.0f},
        {-Width, Height, 0.0f}
    );
    return std::make_shared<PMesh>(Mesh);
}
