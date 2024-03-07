#include <algorithm>
#include "Framework/Renderer/Renderer.h"
#include "Framework/Engine/Engine.h"

#define DRAW_WIREFRAME 1
#define DRAW_SHADED 1
#define DEPTH_TEST 0
#define LOOK_AT 1

/* Renderer */

PRenderer::PRenderer(uint32 InWidth, uint32 InHeight)
{
    Viewport = std::make_shared<PViewport>(InWidth, InHeight);

    AddBuffer(Data, "Depth");
    AddBuffer(Color, "Color");
}

void PRenderer::Resize(uint32 InWidth, uint32 InHeight) const
{
    Viewport->Resize(InWidth, InHeight);
    for (auto B : Buffers | std::views::values)
    {
        B->Resize(InWidth, InHeight);
    }
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
            GetColorBuffer()->SetPixel(Y, X, Color);
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
            GetColorBuffer()->SetPixel(X, Y, Color);
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
    // https://www.khronos.org/opengl/wiki/Calculating_a_Surface_Normal
    const FVector3 Normal = FTriangle::GetSurfaceNormal(V0, V1, V2);
    const float FacingRatio = Math::Dot(Viewport->GetInfo()->Translation, Normal);
    // if (FacingRatio < 0.0f)
    // {
    //     return; // The entire triangle is behind the camera
    // }
    // FacingRatio = Math::Clamp(Math::Abs(FacingRatio), 0.0f, 1.0f);

    // Project the world-space points to screen-space
    FVector3 ScreenPoints[3];
    Viewport->ProjectWorldToScreen(V0, ScreenPoints[0]);
    Viewport->ProjectWorldToScreen(V1, ScreenPoints[1]);
    Viewport->ProjectWorldToScreen(V2, ScreenPoints[2]);

#if DRAW_SHADED
    // Reverse the order to CCW if the order is CW
    const EWindingOrder WindingOrder = FTriangle::GetVertexOrder(ScreenPoints[0], ScreenPoints[1], ScreenPoints[2]);
    switch (WindingOrder)
    {
    case CW :
        std::swap(ScreenPoints[0], ScreenPoints[1]);
        break;
    case CCW :
        break;
    case CL :
        return;
    }

    // Get the bounding box of the 2d triangle
    const FRect BB = FRect::MakeBoundingBox(ScreenPoints[0], ScreenPoints[1], ScreenPoints[2]);

#if DEPTH_TEST
    // Only calculate the area of the triangle if we're doing the depth test
    float Area = FTriangle::Area(ScreenPoints[0], ScreenPoints[1], ScreenPoints[2]);
#endif

    for (int32 Y = BB.Min().Y; Y < BB.Max().Y; Y++)
    {
        for (int32 X = BB.Min().X; X < BB.Max().X; X++)
        {
            FVector3 P = FVector3(static_cast<float>(X) + 0.5f, static_cast<float>(Y) + 0.5f, 0.0f);
            FVector3 UVW;
            if (!FTriangle::GetBarycentric(P, ScreenPoints[0], ScreenPoints[1], ScreenPoints[2], UVW))
            {
                continue;
            }

            // UVW /= Area;
#if DEPTH_TEST
            float NewDepth = 1.0f / (ScreenPoints[0].Z * UVW.X + ScreenPoints[1].Z * UVW.Y + ScreenPoints[2].Z * UVW.Z);
            const float CurrentDepth = GetDepthBuffer()->GetPixel<float>(X, Y);
            if (NewDepth > CurrentDepth)
            {
                continue;
            }
            // Set the depth buffer to the current pixel depth
            GetDepthBuffer()->SetPixel(X, Y, NewDepth);
            const uint8 R = static_cast<uint8>(NewDepth / GetViewport()->GetInfo()->MaxZ * 255.0f); // Linear to SRGB
#else
            const uint8 R = 128; //Math::Rerange(NewDepth, 0.0f, 100.0f, 0.0f, 1.0f) * 255;
#endif

            // Set the color buffer to this new color
            GetColorBuffer()->SetPixel(X, Y, PColor::FromRgba(UVW.X * 255, UVW.Y * 255, UVW.Z  * 255));
        }
    }
#endif

#if DRAW_WIREFRAME
    DrawLine({ScreenPoints[0].X, ScreenPoints[0].Y}, {ScreenPoints[1].X, ScreenPoints[1].Y}, WireColor);
    DrawLine({ScreenPoints[1].X, ScreenPoints[1].Y}, {ScreenPoints[2].X, ScreenPoints[2].Y}, WireColor);
    DrawLine({ScreenPoints[2].X, ScreenPoints[2].Y}, {ScreenPoints[0].X, ScreenPoints[0].Y}, WireColor);
#endif
}

void PRenderer::DrawMesh(const PMesh* Mesh) const
{
    for (uint32 Index = 0; Index < Mesh->GetTriCount(); Index++)
    {
        const uint32 StartIndex = Index * 3;

        const uint32 I0 = Mesh->Indices[StartIndex];
        const uint32 I1 = Mesh->Indices[StartIndex + 1];
        const uint32 I2 = Mesh->Indices[StartIndex + 2];

        const FVector3* V0 = &Mesh->Vertices[I0];
        const FVector3* V1 = &Mesh->Vertices[I1];
        const FVector3* V2 = &Mesh->Vertices[I2];

        DrawTriangle(*V0, *V1, *V2);
    }
}
void PRenderer::Render() const
{
    Viewport->UpdateViewProjectionMatrix();
    ClearBuffers();

    const PEngine* Engine = PEngine::GetInstance();
    for (const auto& Mesh : Engine->GetMeshes())
    {
        DrawMesh(Mesh.get());
    }
}

void PRenderer::ClearBuffers() const
{
    for (const auto& B : Buffers | std::views::values)
    {
        B->Clear();
    }

    // Fill the depth buffer with the Max Z-depth
    GetDepthBuffer()->Fill(Viewport->GetInfo()->MaxZ);
}
