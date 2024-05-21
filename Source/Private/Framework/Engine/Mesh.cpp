#include "Framework/Engine/Mesh.h"

#include "Framework/Core/Logging.h"

// Triangle

uint32 PMesh::AddVertex(const FVector3& V)
{
    // Add new vert
    VertexPositions.emplace_back(V);
    return static_cast<uint32>(VertexPositions.size()) - 1; // NOLINT
}

void PMesh::AddTri(const FVector3& InV0, const FVector3& InV1, const FVector3& InV2)
{
    uint32 V0Idx = AddVertex(InV0);
    uint32 V1Idx = AddVertex(InV1);
    uint32 V2Idx = AddVertex(InV2);

    VertexPositionIndexes.emplace_back(V0Idx);
    VertexPositionIndexes.emplace_back(V1Idx);
    VertexPositionIndexes.emplace_back(V2Idx);
}

void PMesh::AddQuad(const FVector3& V0, const FVector3& V1, const FVector3& V2, const FVector3& V3)
{
    AddTri(V2, V0, V1);
    AddTri(V0, V2, V3);
}

void PMesh::Empty()
{
    VertexPositions.clear();
    VertexPositionIndexes.clear();
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
        {-Width, 0.0f, -Height},
        {Width, 0.0f, -Height},
        {Width, 0.0f, Height},
        {-Width, 0.0f, Height}
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
    std::vector<uint32> Indices{
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

    std::vector Vertices{
        FVector3(-Scale, -Scale, Scale), //0
        FVector3(Scale, -Scale, Scale), //1
        FVector3(-Scale, Scale, Scale), //2
        FVector3(Scale, Scale, Scale), //3
        FVector3(-Scale, -Scale, -Scale), //4
        FVector3(Scale, -Scale, -Scale), //5
        FVector3(-Scale, Scale, -Scale), //6
        FVector3(Scale, Scale, -Scale) //7
    };

    return std::make_shared<PMesh>(Vertices, Indices);
}

std::shared_ptr<PMesh> PMesh::CreateTeapot(float Scale = 1.0f)
{
    std::vector Vertices
    {
        FVector3(-0.3911f, 0.4617f, -0.4981f),
        FVector3(-0.5946f, 0.4617f, -0.0000f),
        FVector3(-0.5929f, 0.4125f, -0.0000f),
        FVector3(-0.3900f, 0.4125f, -0.4970f),
        FVector3(-0.4255f, 0.4125f, -0.5325f),
        FVector3(-0.6429f, 0.4125f, -0.0000f),
        FVector3(0.1071f, 0.4617f, -0.7016f),
        FVector3(0.1071f, 0.4125f, -0.7000f),
        FVector3(0.1071f, 0.4125f, -0.7500f),
        FVector3(0.6072f, 0.4617f, -0.4981f),
        FVector3(0.6209f, 0.4125f, -0.4970f),
        FVector3(0.6396f, 0.4125f, -0.5325f),
        FVector3(0.8087f, 0.4617f, 0.0000f),
        FVector3(0.8070f, 0.4125f, 0.0000f),
        FVector3(0.8571f, 0.4125f, 0.0000f),
        FVector3(0.6051f, 0.4617f, 0.4981f),
        FVector3(0.6041f, 0.4125f, 0.4970f),
        FVector3(0.6396f, 0.4125f, 0.5325f),
        FVector3(0.1071f, 0.4617f, 0.7016f),
        FVector3(0.1071f, 0.4125f, 0.7000f),
        FVector3(0.1071f, 0.4125f, 0.7500f),
        FVector3(-0.3911f, 0.4617f, 0.4981f),
        FVector3(-0.3900f, 0.4125f, 0.4970f),
        FVector3(-0.4255f, 0.4125f, 0.5325f),
        FVector3(-0.5475f, 0.0234f, -0.6545f),
        FVector3(-0.8148f, 0.0234f, -0.0000f),
        FVector3(-0.6029f, -0.3375f, -0.7100f),
        FVector3(-0.8929f, -0.3375f, -0.0000f),
        FVector3(0.1071f, 0.0234f, -0.9219f),
        FVector3(0.1071f, -0.3375f, -1.0000f),
        FVector3(0.7616f, 0.0234f, -0.6545f),
        FVector3(0.8170f, -0.3375f, -0.7100f),
        FVector3(1.0289f, 0.0234f, 0.0000f),
        FVector3(1.1070f, -0.3375f, 0.0000f),
        FVector3(0.7616f, 0.0234f, 0.6545f),
        FVector3(0.8170f, -0.3375f, 0.7100f),
        FVector3(0.1071f, 0.0234f, 0.9219f),
        FVector3(0.1071f, -0.3375f, 1.0000f),
        FVector3(-0.5475f, 0.0234f, 0.6545f),
        FVector3(-0.6029f, -0.3375f, 0.7100f),
        FVector3(-0.5142f, -0.5953f, -0.6213f),
        FVector3(-0.7679f, -0.5953f, -0.0000f),
        FVector3(-0.4255f, -0.7125f, -0.5325f),
        FVector3(-0.6429f, -0.7125f, -0.0000f),
        FVector3(0.1071f, -0.5953f, -0.8750f),
        FVector3(0.1071f, -0.7125f, -0.7500f),
        FVector3(0.7283f, -0.5953f, -0.6213f),
        FVector3(0.6396f, -0.7125f, -0.5325f),
        FVector3(0.9821f, -0.5953f, 0.0000f),
        FVector3(0.8571f, -0.7125f, 0.0000f),
        FVector3(0.7283f, -0.5953f, 0.6213f),
        FVector3(0.6396f, -0.7125f, 0.5325f),
        FVector3(0.1071f, -0.5953f, 0.8750f),
        FVector3(0.1071f, -0.7125f, 0.7500f),
        FVector3(-0.5142f, -0.5953f, 0.6213f),
        FVector3(-0.4255f, -0.7125f, 0.5325f),
        FVector3(-0.3489f, -0.7641f, -0.4560f),
        FVector3(-0.5351f, -0.7641f, -0.0000f),
        FVector3(0.1071f, -0.7875f, -0.0000f),
        FVector3(0.1071f, -0.7641f, -0.6422f),
        FVector3(0.5631f, -0.7641f, -0.4560f),
        FVector3(0.7492f, -0.7641f, -0.0000f),
        FVector3(0.5631f, -0.7641f, 0.4560f),
        FVector3(0.1071f, -0.7641f, 0.6422f),
        FVector3(-0.3489f, -0.7641f, 0.4560f),
        FVector3(1.3665f, 0.2602f, -0.1125f),
        FVector3(1.3133f, 0.2109f, 0.0000f),
        FVector3(0.9071f, 0.2250f, 0.0000f),
        FVector3(0.8820f, 0.2813f, -0.1125f),
        FVector3(1.5320f, 0.1125f, -0.1125f),
        FVector3(1.4571f, 0.1125f, 0.0000f),
        FVector3(1.4195f, 0.3094f, 0.0000f),
        FVector3(0.8571f, 0.3375f, 0.0000f),
        FVector3(1.6070f, 0.1125f, 0.0000f),
        FVector3(1.3665f, 0.2602f, 0.1125f),
        FVector3(0.8820f, 0.2813f, 0.1125f),
        FVector3(1.5320f, 0.1125f, 0.1125f),
        FVector3(1.4242f, -0.1605f, -0.1125f),
        FVector3(1.3758f, -0.1125f, 0.0000f),
        FVector3(1.0821f, -0.4125f, -0.1125f),
        FVector3(1.4727f, -0.2086f, 0.0000f),
        FVector3(1.0570f, -0.4875f, 0.0000f),
        FVector3(1.4242f, -0.1605f, 0.1125f),
        FVector3(1.0821f, -0.4125f, 0.1125f),
        FVector3(-1.1616f, 0.0234f, -0.1706f),
        FVector3(-1.0868f, 0.1125f, -0.0000f),
        FVector3(-0.7430f, -0.0750f, -0.0000f),
        FVector3(-0.7430f, -0.2812f, -0.2475f),
        FVector3(-1.3930f, 0.4125f, -0.0938f),
        FVector3(-1.2429f, 0.4125f, -0.0000f),
        FVector3(-1.2366f, -0.0656f, -0.0000f),
        FVector3(-0.7430f, -0.4875f, -0.0000f),
        FVector3(-1.5429f, 0.4125f, -0.0000f),
        FVector3(-1.1616f, 0.0234f, 0.1706f),
        FVector3(-0.7430f, -0.2813f, 0.2475f),
        FVector3(-1.3930f, 0.4125f, 0.0938f),
        FVector3(-1.4562f, 0.4459f, -0.0750f),
        FVector3(-1.3054f, 0.4406f, -0.0000f),
        FVector3(-1.3930f, 0.4125f, -0.0563f),
        FVector3(-1.2929f, 0.4125f, -0.0000f),
        FVector3(-1.6070f, 0.4512f, -0.0000f),
        FVector3(-1.4929f, 0.4125f, -0.0000f),
        FVector3(-1.4562f, 0.4459f, 0.0750f),
        FVector3(-1.3930f, 0.4125f, 0.0563f),
        FVector3(-0.0085f, 0.7031f, -0.1155f),
        FVector3(-0.0554f, 0.7031f, 0.0000f),
        FVector3(0.1071f, 0.7875f, 0.0000f),
        FVector3(0.0360f, 0.5625f, -0.0710f),
        FVector3(0.0071f, 0.5625f, 0.0000f),
        FVector3(0.1071f, 0.7031f, -0.1625f),
        FVector3(0.1071f, 0.5625f, -0.1000f),
        FVector3(0.2226f, 0.7031f, -0.1155f),
        FVector3(0.1781f, 0.5625f, -0.0710f),
        FVector3(0.2695f, 0.7031f, 0.0000f),
        FVector3(0.2070f, 0.5625f, 0.0000f),
        FVector3(0.2226f, 0.7031f, 0.1155f),
        FVector3(0.1781f, 0.5625f, 0.0710f),
        FVector3(0.1071f, 0.7031f, 0.1625f),
        FVector3(0.1071f, 0.5625f, 0.1000f),
        FVector3(-0.0085f, 0.7031f, 0.1155f),
        FVector3(0.0360f, 0.5625f, 0.0710f),
        FVector3(-0.1858f, 0.4875f, -0.2929f),
        FVector3(-0.3054f, 0.4875f, 0.0000f),
        FVector3(-0.3544f, 0.4125f, -0.4615f),
        FVector3(-0.5429f, 0.4125f, -0.0000f),
        FVector3(0.1071f, 0.4875f, -0.4125f),
        FVector3(0.1071f, 0.4125f, -0.6500f),
        FVector3(0.3999f, 0.4875f, -0.2929f),
        FVector3(0.5685f, 0.4125f, -0.4615f),
        FVector3(0.5195f, 0.4875f, 0.0000f),
        FVector3(0.7570f, 0.4125f, 0.0000f),
        FVector3(0.3999f, 0.4875f, 0.2929f),
        FVector3(0.5685f, 0.4125f, 0.4615f),
        FVector3(0.1071f, 0.4875f, 0.4125f),
        FVector3(0.1071f, 0.4125f, 0.6500f),
        FVector3(-0.1858f, 0.4875f, 0.2929f),
        FVector3(-0.3544f, 0.4125f, 0.4615f)
    };
    for (auto& V : Vertices)
    {
        V *= Scale;
        V.Y *= -1;
    }

    std::vector<uint32> Indices
    {
        0, 1, 2,
        2, 3, 0,
        4, 5, 1,
        1, 0, 4,
        6, 0, 3,
        3, 7, 6,
        8, 4, 0,
        0, 6, 8,
        9, 6, 7,
        7, 10, 9,
        11, 8, 6,
        6, 9, 11,
        12, 9, 10,
        10, 13, 12,
        14, 11, 9,
        9, 12, 14,
        15, 12, 13,
        13, 16, 15,
        17, 14, 12,
        12, 15, 17,
        18, 15, 16,
        16, 19, 18,
        20, 17, 15,
        15, 18, 20,
        21, 18, 19,
        19, 22, 21,
        23, 20, 18,
        18, 21, 23,
        1, 21, 22,
        22, 2, 1,
        5, 23, 21,
        21, 1, 5,
        24, 25, 5,
        5, 4, 24,
        26, 27, 25,
        25, 24, 26,
        28, 24, 4,
        4, 8, 28,
        29, 26, 24,
        24, 28, 29,
        30, 28, 8,
        8, 11, 30,
        31, 29, 28,
        28, 30, 31,
        32, 30, 11,
        11, 14, 32,
        33, 31, 30,
        30, 32, 33,
        34, 32, 14,
        14, 17, 34,
        35, 33, 32,
        32, 34, 35,
        36, 34, 17,
        17, 20, 36,
        37, 35, 34,
        34, 36, 37,
        38, 36, 20,
        20, 23, 38,
        39, 37, 36,
        36, 38, 39,
        25, 38, 23,
        23, 5, 25,
        27, 39, 38,
        38, 25, 27,
        40, 41, 27,
        27, 26, 40,
        42, 43, 41,
        41, 40, 42,
        44, 40, 26,
        26, 29, 44,
        45, 42, 40,
        40, 44, 45,
        46, 44, 29,
        29, 31, 46,
        47, 45, 44,
        44, 46, 47,
        48, 46, 31,
        31, 33, 48,
        49, 47, 46,
        46, 48, 49,
        50, 48, 33,
        33, 35, 50,
        51, 49, 48,
        48, 50, 51,
        52, 50, 35,
        35, 37, 52,
        53, 51, 50,
        50, 52, 53,
        54, 52, 37,
        37, 39, 54,
        55, 53, 52,
        52, 54, 55,
        41, 54, 39,
        39, 27, 41,
        43, 55, 54,
        54, 41, 43,
        56, 57, 43,
        43, 42, 56,
        57, 56, 58,
        59, 56, 42,
        42, 45, 59,
        56, 59, 58,
        60, 59, 45,
        45, 47, 60,
        59, 60, 58,
        61, 60, 47,
        47, 49, 61,
        60, 61, 58,
        62, 61, 49,
        49, 51, 62,
        61, 62, 58,
        63, 62, 51,
        51, 53, 63,
        62, 63, 58,
        64, 63, 53,
        53, 55, 64,
        63, 64, 58,
        57, 64, 55,
        55, 43, 57,
        64, 57, 58,
        65, 66, 67,
        67, 68, 65,
        69, 70, 66,
        66, 65, 69,
        71, 65, 68,
        68, 72, 71,
        73, 69, 65,
        65, 71, 73,
        74, 71, 72,
        72, 75, 74,
        76, 73, 71,
        71, 74, 76,
        66, 74, 75,
        75, 67, 66,
        70, 76, 74,
        74, 66, 70,
        77, 78, 70,
        70, 69, 77,
        79, 33, 78,
        78, 77, 79,
        80, 77, 69,
        69, 73, 80,
        81, 79, 77,
        77, 80, 81,
        82, 80, 73,
        73, 76, 82,
        83, 81, 80,
        80, 82, 83,
        78, 82, 76,
        76, 70, 78,
        33, 83, 82,
        82, 78, 33,
        84, 85, 86,
        86, 87, 84,
        88, 89, 85,
        85, 84, 88,
        90, 84, 87,
        87, 91, 90,
        92, 88, 84,
        84, 90, 92,
        93, 90, 91,
        91, 94, 93,
        95, 92, 90,
        90, 93, 95,
        85, 93, 94,
        94, 86, 85,
        89, 95, 93,
        93, 85, 89,
        96, 97, 89,
        89, 88, 96,
        98, 99, 97,
        97, 96, 98,
        100, 96, 88,
        88, 92, 100,
        101, 98, 96,
        96, 100, 101,
        102, 100, 92,
        92, 95, 102,
        103, 101, 100,
        100, 102, 103,
        97, 102, 95,
        95, 89, 97,
        99, 103, 102,
        102, 97, 99,
        104, 105, 106,
        107, 108, 105,
        105, 104, 107,
        109, 104, 106,
        110, 107, 104,
        104, 109, 110,
        111, 109, 106,
        112, 110, 109,
        109, 111, 112,
        113, 111, 106,
        114, 112, 111,
        111, 113, 114,
        115, 113, 106,
        116, 114, 113,
        113, 115, 116,
        117, 115, 106,
        118, 116, 115,
        115, 117, 118,
        119, 117, 106,
        120, 118, 117,
        117, 119, 120,
        105, 119, 106,
        108, 120, 119,
        119, 105, 108,
        121, 122, 108,
        108, 107, 121,
        123, 124, 122,
        122, 121, 123,
        125, 121, 107,
        107, 110, 125,
        126, 123, 121,
        121, 125, 126,
        127, 125, 110,
        110, 112, 127,
        128, 126, 125,
        125, 127, 128,
        129, 127, 112,
        112, 114, 129,
        130, 128, 127,
        127, 129, 130,
        131, 129, 114,
        114, 116, 131,
        132, 130, 129,
        129, 131, 132,
        133, 131, 116,
        116, 118, 133,
        134, 132, 131,
        131, 133, 134,
        135, 133, 118,
        118, 120, 135,
        136, 134, 133,
        133, 135, 136,
        122, 135, 120,
        120, 108, 122,
        124, 136, 135,
        135, 122, 124
    };

    return std::make_shared<PMesh>(Vertices, Indices);
}

EWindingOrder Math::GetWindingOrder(const FVector3& V0, const FVector3& V1, const FVector3& V2)
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
