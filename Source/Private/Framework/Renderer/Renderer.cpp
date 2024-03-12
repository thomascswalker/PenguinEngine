#include <algorithm>
#include "Framework/Renderer/Renderer.h"
#include "Framework/Engine/Engine.h"

#define DRAW_WIREFRAME 1
#define DRAW_SHADED 1
#define DEPTH_TEST 0
#define LOOK_AT 0

/* Renderer */
PRenderer::PRenderer(uint32 InWidth, uint32 InHeight)
{
    Viewport = std::make_shared<PViewport>(InWidth, InHeight);

    AddBuffer(EBufferType::Data, "Depth");
    AddBuffer(EBufferType::Color, "Color");
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
    const FVector3 LookAtTranslation = Viewport->GetCamera()->TargetTranslation;
    const FVector3 CameraTranslation = Viewport->GetCamera()->GetTranslation();

    const FVector3 CameraNormal = (CameraTranslation - LookAtTranslation).Normalized();
    const FVector3 WorldNormal = FTriangle::GetSurfaceNormal(V0, V1, V2).Normalized();

    const float FacingRatio = Math::Max(0.0f, Math::Dot(WorldNormal, CameraNormal));

    // Project the world-space points to screen-space
    FVector3 ScreenPoints[3];
    bool bTriangleOnScreen = false;
    bTriangleOnScreen |= Viewport->ProjectWorldToScreen(V0, ScreenPoints[0]);
    bTriangleOnScreen |= Viewport->ProjectWorldToScreen(V1, ScreenPoints[1]);
    bTriangleOnScreen |= Viewport->ProjectWorldToScreen(V2, ScreenPoints[2]);

    // If none of the points are on the screen, the triangle is not in the frame, so exit
    // drawing this triangle
    if (!bTriangleOnScreen)
    {
        return;
    }

#if DRAW_SHADED
    // Reverse the order to CCW if the order is CW
    const EWindingOrder WindingOrder = FTriangle::GetVertexOrder(ScreenPoints[0], ScreenPoints[1], ScreenPoints[2]);
    switch (WindingOrder)
    {
    case EWindingOrder::CW :
        std::swap(ScreenPoints[0], ScreenPoints[1]);
        break;
    case EWindingOrder::CCW :
        break;
    case EWindingOrder::CL :
        return;
    }

    // Get the bounding box of the 2d triangle
    FRect BB = FRect::MakeBoundingBox(ScreenPoints[0], ScreenPoints[1], ScreenPoints[2]);
    BB.Clamp({0, 0, static_cast<float>(GetWidth()), static_cast<float>(GetHeight())});

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

#if DEPTH_TEST
            const float CurrentDepth = static_cast<float>(GetDepthBuffer()->GetPixel(X, Y));
            float NewDepth = FTriangle::GetDepth(P, V0, V1, V2);
            if (NewDepth > CurrentDepth)
            {
                // continue;
            }
            GetDepthBuffer()->SetPixel(X, Y, NewDepth);
#endif
            uint8 R = Math::Pow(FacingRatio, 2.2f) * 255; // Convert to SRGB
            // Set the color buffer to this new color
            GetColorBuffer()->SetPixel(X, Y, PColor::FromRgba(R, R, R));
        }
    }
#endif
    
#if DRAW_WIREFRAME
    DrawLine({ScreenPoints[0].X, ScreenPoints[0].Y}, {ScreenPoints[1].X, ScreenPoints[1].Y}, WireColor);
    DrawLine({ScreenPoints[1].X, ScreenPoints[1].Y}, {ScreenPoints[2].X, ScreenPoints[2].Y}, WireColor);
    DrawLine({ScreenPoints[2].X, ScreenPoints[2].Y}, {ScreenPoints[0].X, ScreenPoints[0].Y}, WireColor);
#endif
}

// TODO: Rewrite to use a single array of vertices rather than looping through meshes/triangles
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
    GetDepthBuffer()->Fill(Viewport->GetCamera()->MaxZ);
}
