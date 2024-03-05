#include <algorithm>
#include "Framework/Renderer/Renderer.h"
#include "Framework/Engine/Engine.h"

#if _WIN32
    #include <windows.h>
#endif

#define DRAW_WIREFRAME 1
#define DRAW_SHADED 1

static const FVector3 DEFAULT_CAMERA_TRANSLATION(0, 0, -1);

/* Buffer */

PBuffer::PBuffer(uint32 InWidth, uint32 InHeight)
{
    Resize(InWidth, InHeight);
}

void PBuffer::Resize(uint32 InWidth, uint32 InHeight)
{
    Width = InWidth;
    Height = InHeight;
    Pitch = static_cast<uint32>(sizeof(uint32) * InWidth);
    Realloc();
}

void PBuffer::Realloc()
{
#if _WIN32
    if (Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
    Memory = VirtualAlloc(nullptr, GetMemorySize(), MEM_COMMIT, PAGE_READWRITE);
#endif
}

void PBuffer::SetPixel(const uint32 X, const uint32 Y, const PColor& Color) const
{
    if (X < 0 || X >= Width || Y < 0 || Y >= Height)
    {
        return;
    }
    uint32* Ptr = static_cast<uint32*>(Memory) + GetOffset(X, Y);

    // Placing pixels in memory order within a uint32:
    // BB GG RR AA
    // 00 00 00 00
    *Ptr = ((Color.R << 16) | Color.G << 8) | Color.B; // TODO: Disregard alpha channel for now
}

/* Renderer */

PRenderer::PRenderer(uint32 InWidth, uint32 InHeight)
{
    Buffer = std::make_shared<PBuffer>(InWidth, InHeight);
    Viewport = std::make_shared<PViewport>(InWidth, InHeight, DEFAULT_CAMERA_TRANSLATION);
}

void PRenderer::Resize(uint32 InWidth, uint32 InHeight) const
{
    Buffer->Resize(InWidth, InHeight);
    Viewport->Resize(InWidth, InHeight);
}

void PRenderer::DrawLine(const FVector2& InA, const FVector2& InB, const PColor& Color) const
{
    IVector2 A(InA.X, InA.Y); // NOLINT
    IVector2 B(InB.X, InB.Y); // NOLINT

    bool bIsSteep = false;
    if (Math::Abs(A.X - B.X) < Math::Abs(A.Y - B.Y))
    {
        A = IVector2(A.Y, A.X);
        B = IVector2(B.Y, B.X);
        bIsSteep = true;
    }

    if (A.X > B.X)
    {
        std::swap(A, B);
    }

    const int32 DeltaX = B.X - A.X;
    const int32 DeltaY = B.Y - A.Y;
    const int32 DeltaError = Math::Abs(DeltaY) * 2;
    int32 ErrorCount = 0;

    // https://github.com/ssloy/tinyrenderer/issues/28
    int32 Y = A.Y;
    if (bIsSteep)
    {
        for (int32 X = A.X; X < B.X; ++X)
        {
            Buffer->SetPixel(Y, X, Color);
            ErrorCount += DeltaError;
            if (ErrorCount > DeltaX)
            {
                Y += (B.Y > A.Y ? 1 : -1);
                ErrorCount -= DeltaX * 2;
            }
        }
    }
    else
    {
        for (int32 X = A.X; X < B.X; ++X)
        {
            Buffer->SetPixel(X, Y, Color);
            ErrorCount += DeltaError;
            if (ErrorCount > DeltaX)
            {
                Y += (B.Y > A.Y ? 1 : -1);
                ErrorCount -= DeltaX * 2;
            }
        }
    }
}

void PRenderer::DrawTriangle(const FVector3& V0, const FVector3& V1, const FVector3& V2) const
{
    // Project the world-space points to screen-space
    FVector3 Points[3];
    Viewport->ProjectWorldToScreen(V0, Points[0]);
    Viewport->ProjectWorldToScreen(V1, Points[1]);
    Viewport->ProjectWorldToScreen(V2, Points[2]);

#if DRAW_SHADED
    // Reverse the order to CCW if the order is CW
    const EWindingOrder WindingOrder = FBarycentric::GetVertexOrder(Points[0], Points[1], Points[2]);
    if (WindingOrder != CCW)
    {
        std::swap(Points[0], Points[1]);
    }

    // Get the bounding box of the 2d triangle
    float XValues[3] = {
        Points[0].X,
        Points[1].X,
        Points[2].X
    };
    float YValues[3] = {
        Points[0].Y,
        Points[1].Y,
        Points[2].Y
    };
    
    const int32 MinX = static_cast<int32>(*std::ranges::min_element(XValues));
    const int32 MinY = static_cast<int32>(*std::ranges::min_element(YValues));
    const int32 MaxX = static_cast<int32>(*std::ranges::max_element(XValues));
    const int32 MaxY = static_cast<int32>(*std::ranges::max_element(YValues));
    
    for (int32 Y = MinY; Y < MaxY; Y++)
    {
        for (int32 X = MinX; X < MaxX; X++)
        {
            FVector3 UVW;
            if (FBarycentric::GetBarycentric(FVector3(static_cast<float>(X) + 0.5f, static_cast<float>(Y) + 0.5f, 0.0f), Points, UVW))
            {
                const uint8 R = static_cast<uint8>(UVW.X * 255.0f); // Linear to SRGB
                const uint8 G = static_cast<uint8>(UVW.Y * 255.0f); // Linear to SRGB
                Buffer->SetPixel(X, Y, PColor::FromRgba(R, G, 255));
            }
        }
    }
#endif

#ifdef DRAW_WIREFRAME
    const PColor WireColor = PColor::FromRgba(255, 175, 50);
    DrawLine({Points[0].X, Points[0].Y}, {Points[1].X, Points[1].Y}, WireColor);
    DrawLine({Points[1].X, Points[1].Y}, {Points[2].X, Points[2].Y}, WireColor);
    DrawLine({Points[2].X, Points[2].Y}, {Points[0].X, Points[0].Y}, WireColor);
#endif
}

void PRenderer::DrawMesh(const PMesh* Mesh) const
{
    for (uint32 Index = 0; Index < Mesh->GetTriCount(); Index++)
    {
        const uint32 StartIndex = Index * 3;

        const uint32 V0Idx = Mesh->Indices[StartIndex];
        const uint32 V1Idx = Mesh->Indices[StartIndex + 1];
        const uint32 V2Idx = Mesh->Indices[StartIndex + 2];

        const FVector3* V0 = &Mesh->Vertices[V0Idx];
        const FVector3* V1 = &Mesh->Vertices[V1Idx];
        const FVector3* V2 = &Mesh->Vertices[V2Idx];

        DrawTriangle(*V0, *V1, *V2);
    }
}
void PRenderer::Render() const
{
    Viewport->UpdateViewProjectionMatrix(true);
    Buffer->Clear();

    const PEngine* Engine = PEngine::GetInstance();
    for (const auto& Mesh : Engine->GetMeshes())
    {
        DrawMesh(Mesh.get());
    }
}
