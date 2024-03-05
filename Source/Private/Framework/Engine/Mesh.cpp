#include "Framework/Engine/Mesh.h"

#include "Framework/Core/Logging.h"

// Triangle

FVector3 PTri::GetNormal() const
{
    const FVector3 U = V1 - V0;
    const FVector3 V = V2 - V1;
    FVector3 N;
    N.X = (U.Y * V.Z) - (U.Z * V.Y);
    N.Y = (U.Z * V.X) - (U.X * V.Z);
    N.Z = (U.X * V.Y) - (U.Y * V.X);
    return N;
}

// Mesh

uint32 PMesh::AddVertex(const FVector3& V)
{
    auto Iter = std::ranges::find(Vertices, V);
    if (Iter == Vertices.end())
    {
        // Add new vert
        Vertices.emplace_back(V);
        return static_cast<uint32>(Vertices.size()) - 1; // NOLINT
    }

    // Get index and return pointer of existing vert
    return static_cast<uint32>(std::distance(Vertices.begin(), Iter)); // NOLINT
}

void PMesh::AddTri(const FVector3& InV0, const FVector3& InV1, const FVector3& InV2)
{
    uint32 V0Idx = AddVertex(InV0);
    uint32 V1Idx = AddVertex(InV1);
    uint32 V2Idx = AddVertex(InV2);

    Indices.emplace_back(V0Idx);
    Indices.emplace_back(V1Idx);
    Indices.emplace_back(V2Idx);
}

void PMesh::AddQuad(const FVector3& V0, const FVector3& V1, const FVector3& V2, const FVector3& V3)
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
    FVector3 V0 = Vertices[Indices.at(StartIndex)];
    FVector3 V1 = Vertices[Indices.at(StartIndex + 1)];
    FVector3 V2 = Vertices[Indices.at(StartIndex + 2)];
    return PTri(V0, V1, V2);
}

EWindingOrder PMesh::GetWindingOrder(const FVector3& V0, const FVector3& V1, const FVector3& V2)
{
    const float Result = (V1.X - V0.X) * (V2.Y - V0.Y) - (V2.X - V0.X) * (V1.Y - V0.Y);
    if (Result > 0)
    {
        return CCW;
    }
    if (Result < 0)
    {
        return CW;
    }
    return CL;
}

/*
 *        (0, s)
 *          v0
 *         /  \
 *        /    \
 *       v1----v2
 *   (-s, -s)  (s, -s)
 *   
 */
std::shared_ptr<PMesh> PMesh::CreateTriangle(float Scale)
{
    PMesh Mesh;

    // Construct triangle in counter-clockwise order
    Mesh.AddTri(
        {Scale, -Scale, 0.0f}, // v0
        {0.0f, Scale, 0.0f}, // v2
        {-Scale, -Scale, 0.0f} // v1
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

std::shared_ptr<PMesh> PMesh::CreateSphere(float Radius, float Segments)
{
    std::vector<FVector3> Vertices;
    std::vector<uint32> Indices;

    float SectorCount = Segments;
    float StackCount = Segments;
    float X, Y, Z, XY; // vertex position

    float SectorStep = 2 * P_PI / SectorCount;
    float StackStep = P_PI / StackCount;

    for (int32 Stack = 0; Stack <= StackCount; ++Stack)
    {
        float StackAngle = P_PI / 2 - Stack * StackStep; // starting from pi/2 to -pi/2
        XY = Radius * cosf(StackAngle); // r * cos(u)
        Z = Radius * sinf(StackAngle); // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // first and last vertices have same position and normal, but different tex coords
        for (int32 Sector = 0; Sector <= SectorCount; ++Sector)
        {
            float SectorAngle = Sector * SectorStep; // starting from 0 to 2pi

            // vertex position (x, y, z)
            X = XY * cosf(SectorAngle); // r * cos(u) * cos(v)
            Y = XY * sinf(SectorAngle); // r * cos(u) * sin(v)
            Vertices.emplace_back(FVector3{X, Y, Z});
        }
    }

    int K1, K2;
    for (int32 Stack = 0; Stack < StackCount; ++Stack)
    {
        K1 = Stack * (SectorCount + 1); // beginning of current stack
        K2 = K1 + SectorCount + 1; // beginning of next stack

        for (int32 Sector = 0; Sector < SectorCount; ++Sector, ++K1, ++K2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (Stack != 0)
            {
                Indices.push_back(K1);
                Indices.push_back(K2);
                Indices.push_back(K1 + 1);
            }

            // k1+1 => k2 => k2+1
            if (Stack != (StackCount - 1))
            {
                Indices.push_back(K1 + 1);
                Indices.push_back(K2);
                Indices.push_back(K2 + 1);
            }
        }
    }

    return std::make_shared<PMesh>(Vertices, Indices);
}
