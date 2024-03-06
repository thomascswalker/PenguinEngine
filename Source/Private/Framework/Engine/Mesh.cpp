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

std::shared_ptr<PMesh> PMesh::CreateSphere(const float Radius, const int32 Segments)
{
    std::vector<FVector3> Vertices;
    std::vector<uint32> Indices;

    const int32 SectorCount = Segments;
    const int32 StackCount = Segments;

    const float SectorStep = 2.0f * P_PI / static_cast<float>(SectorCount);
    const float StackStep = P_PI / static_cast<float>(SectorCount);

    for (int32 Stack = 0; Stack <= StackCount; ++Stack)
    {
        const float StackAngle = P_PI / 2.0f - static_cast<float>(Stack) * StackStep; // starting from pi/2 to -pi/2
        const float XY = Radius * cosf(StackAngle); // r * cos(u)
        float Z = Radius * sinf(StackAngle); // r * sin(u)

        // add (sectorCount+1) vertices per stack
        // first and last vertices have same position and normal, but different tex coords
        for (int32 Sector = 0; Sector <= SectorCount; ++Sector)
        {
            const float SectorAngle = static_cast<float>(Sector) * SectorStep; // starting from 0 to 2pi

            // vertex position (x, y, z)
            float X = XY * Math::Cos(SectorAngle); // r * cos(u) * cos(v)
            float Y = XY * Math::Sin(SectorAngle); // r * cos(u) * sin(v)
            Vertices.emplace_back(X, Y, Z);
        }
    }

    for (int32 Stack = 0; Stack < StackCount; ++Stack)
    {
        int32 K1 = Stack * (SectorCount + 1); // beginning of current stack
        int32 K2 = K1 + SectorCount + 1; // beginning of next stack

        for (int32 Sector = 0; Sector < SectorCount; ++Sector, ++K1, ++K2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (Stack != 0)
            {
                Indices.emplace_back(K1);
                Indices.emplace_back(K2);
                Indices.emplace_back(K1 + 1);
            }

            // k1+1 => k2 => k2+1
            if (Stack != (StackCount - 1))
            {
                Indices.emplace_back(K1 + 1);
                Indices.emplace_back(K2);
                Indices.emplace_back(K2 + 1);
            }
        }
    }

    return std::make_shared<PMesh>(Vertices, Indices);
}

std::shared_ptr<PMesh> PMesh::CreateCube(float Scale)
{
    std::vector<uint32> Indices {
        //Top
        2, 6, 7,
        2, 3, 7,

        //Bottom
        0, 4, 5,
        0, 1, 5,

        //Left
        0, 2, 6,
        0, 4, 6,

        //Right
        1, 3, 7,
        1, 5, 7,

        //Front
        0, 2, 3,
        0, 1, 3,

        //Back
        4, 6, 7,
        4, 5, 7
    };
        
    std::vector Vertices {
        FVector3(-Scale, -Scale,  Scale), //0
        FVector3(Scale, -Scale,  Scale), //1
        FVector3(-Scale,  Scale,  Scale), //2
        FVector3(Scale,  Scale,  Scale), //3
        FVector3(-Scale, -Scale, -Scale), //4
        FVector3(Scale, -Scale, -Scale), //5
        FVector3(-Scale,  Scale, -Scale), //6
        FVector3(Scale,  Scale, -Scale) //7
    };

    return std::make_shared<PMesh>(Vertices, Indices);
}
