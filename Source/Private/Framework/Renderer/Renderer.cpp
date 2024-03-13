#include <algorithm>
#include "Framework/Renderer/Renderer.h"
#include "Framework/Engine/Engine.h"


#define DRAW_WIREFRAME 1
#define DRAW_SHADED 1
#define DEPTH_TEST 1
#define TWO_SIDED 0

/* Renderer */
PRenderer::PRenderer(uint32 InWidth, uint32 InHeight)
{
    Viewport = std::make_shared<PViewport>(InWidth, InHeight);
    Grid = std::make_unique<PGrid>(10, 1.0f);

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

bool PRenderer::ClipLine(const FVector2& InA, const FVector2& InB, FVector2& OutA, FVector2& OutB) const
{
    float Width = GetWidth();
    float Height = GetHeight();

    float X1 = InA.X;
    float Y1 = InA.Y;
    float X2 = InB.X;
    float Y2 = InB.Y;

    float X[2] = {X1, X2};
    float Y[2] = {Y1, Y2};
    for (int32 Index = 0; Index < 2; Index++)
    {
        if (X[Index] < 0.0f)
        {
            X[Index] = 0.0f;
            Y[Index] = (Y2 - Y1) / (X2 - X1) * (0.0f - X1) + Y1;
        }
        else if (X[Index] > Width)
        {
            X[Index] = Width;
            Y[Index] = (Y2 - Y1) / (X2 - X1) * (Width - X1) + Y1;
        }

        if (Y[Index] < 0.0f)
        {
            Y[Index] = 0.0f;
            X[Index] = (X2 - X1) / (Y2 - Y1) * (0.0f - Y1) + X1;
        }
        else if (Y[Index] > Height)
        {
            Y[Index] = Height;
            X[Index] = (X2 - X1) / (Y2 - Y1) * (Height - Y1) + X1;
        }
    }

    OutA.X = X[0];
    OutA.Y = Y[0];
    OutB.X = X[1];
    OutB.Y = Y[1];

    return true;
}
void PRenderer::DrawLine(const FVector2& InA, const FVector2& InB, const PColor& Color) const
{
    FVector2 OutA;
    FVector2 OutB;
    ClipLine(InA, InB, OutA, OutB);
    
    IVector2 A(OutA.X, OutA.Y); // NOLINT
    IVector2 B(OutB.X, OutB.Y); // NOLINT

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

    const FVector3 LookAtTranslation = Viewport->GetCamera()->TargetTranslation;
    const FVector3 CameraTranslation = Viewport->GetCamera()->GetTranslation();

    const FVector3 CameraNormal = (LookAtTranslation - CameraTranslation).Normalized();
    const FVector3 WorldNormal = FTriangle::GetSurfaceNormal(V0, V1, V2);

    // Facing ratio in radians (-1 to 1)
    float FacingRatio = Math::Dot(WorldNormal, CameraNormal);

    // If the triangle is not facing the camera, then return
    // if (FacingRatio <= -0.5f || FacingRatio >= 0.5f)
    // {
    //     return;
    // }

#if TWO_SIDED
    FacingRatio = Math::Abs(Math::Min(FacingRatio, 1.0f));
#else
    FacingRatio = Math::Max(0.0f, Math::Min(FacingRatio, 1.0f));
#endif

    // Get the bounding box of the 2d triangle
    FRect BB = FRect::MakeBoundingBox(ScreenPoints[0], ScreenPoints[1], ScreenPoints[2]);
    BB.Clamp({0, 0, static_cast<float>(GetWidth()), static_cast<float>(GetHeight())});

    for (int32 Y = BB.Min().Y; Y < BB.Max().Y; Y++)
    {
        for (int32 X = BB.Min().X; X < BB.Max().X; X++)
        {
            FVector3 P = FVector3(static_cast<float>(X), static_cast<float>(Y), 0.0f);
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
                continue;
            }
            GetDepthBuffer()->SetPixel(X, Y, NewDepth);
#endif
            uint8 R = Math::Abs(FacingRatio) * 255; // Convert to SRGB
            uint8 G = 128; // Convert to SRGB

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


void PRenderer::DrawGrid() const
{
    for (const PLine3& Line : Grid->Lines)
    {
        // Project the world-space points to screen-space
        FVector3 ScreenPoints[2];
        bool bLineOnScreen = false;
        bLineOnScreen |= Viewport->ProjectWorldToScreen(Line.A, ScreenPoints[0]);
        bLineOnScreen |= Viewport->ProjectWorldToScreen(Line.B, ScreenPoints[1]);
        if (!bLineOnScreen)
        {
            return;
        }

        DrawLine(FVector2{ScreenPoints[0].X, ScreenPoints[0].Y}, FVector2{ScreenPoints[1].X, ScreenPoints[1].Y}, GridColor);
    }
}

void PRenderer::Render() const
{
    Viewport->UpdateViewProjectionMatrix();
    ClearBuffers();

    DrawGrid();

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
