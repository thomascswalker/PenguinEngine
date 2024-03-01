#include "Framework/Renderer/Renderer.h"

#include "Framework/Core/Logging.h"
#include "Framework/Engine/Engine.h"
#include "Math/Types.h"

#if _WIN32
    #include <windows.h>
#endif

/* Buffer */

static float CAMERA_X = 0.0f;
static float CAMERA_Y = 0.0f;
static float CAMERA_Z = -25.0f;

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
    Viewport = std::make_shared<PViewport>(InWidth, InHeight);
}

void PRenderer::Resize(uint32 InWidth, uint32 InHeight) const
{
    Buffer->Resize(InWidth, InHeight);
    Viewport->Resize(InWidth, InHeight);
}

void PRenderer::DrawTriangle(const PVector3& V0, const PVector3& V1, const PVector3& V2) const
{
    PVector3 V0ScreenPosition;
    PVector3 V1ScreenPosition;
    PVector3 V2ScreenPosition;

    Viewport->ProjectWorldToScreen(V0, Viewport->MVP, V0ScreenPosition);
    Viewport->ProjectWorldToScreen(V1, Viewport->MVP, V1ScreenPosition);
    Viewport->ProjectWorldToScreen(V2, Viewport->MVP, V2ScreenPosition);

    for (uint32 Y = 0; Y < Buffer->Height; Y++)
    {
        for (uint32 X = 0; X < Buffer->Width; X++)
        {
            PVector3 Point(X, Y, 0.0f); // NOLINT
            if (!Math::IsPointInTriangle({Point.X, Point.Y},
                                   {V0ScreenPosition.X, V0ScreenPosition.Y},
                                   {V1ScreenPosition.X, V1ScreenPosition.Y},
                                   {V2ScreenPosition.X, V2ScreenPosition.Y}))
            {
                continue;
            }

            PVector3 UVW;
            Math::ClosestPointBarycentrics(Point, V0ScreenPosition, V1ScreenPosition, V2ScreenPosition, UVW);

            const uint8 R = UVW.X * 255; // NOLINT
            const uint8 G = UVW.Y * 255; // NOLINT
            Buffer->SetPixel(X, Y, PColor::FromRgba(R, G, 150));
        }
    }
}

void PRenderer::DrawMesh(const PMesh* Mesh) const
{
    for (uint32 Index = 0; Index < Mesh->GetTriCount(); Index++)
    {
        const uint32 StartIndex = Index * 3;
        const uint32 V0Idx = Mesh->Indices[StartIndex];
        const uint32 V1Idx = Mesh->Indices[StartIndex + 1];
        const uint32 V2Idx = Mesh->Indices[StartIndex + 2];

        auto V0 = Mesh->Vertices[V0Idx];
        auto V1 = Mesh->Vertices[V1Idx];
        auto V2 = Mesh->Vertices[V2Idx];

        DrawTriangle(V0, V1, V2);
    }
}
void PRenderer::Render() const
{
    Viewport->GetInfo()->ViewOrigin = {CAMERA_X, CAMERA_Y, CAMERA_Z};
    Viewport->MVP = Viewport->GetInfo()->ComputeViewProjectionMatrix();

    Buffer->Fill(PColor::Black());

    const PEngine* Engine = PEngine::GetInstance();
    for (auto Mesh : Engine->GetMeshes())
    {
        DrawMesh(Mesh.get());
    }
}
