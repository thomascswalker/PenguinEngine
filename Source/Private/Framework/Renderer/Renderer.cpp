// ReSharper disable CppClangTidyBugproneNarrowingConversions
// ReSharper disable CppClangTidyClangDiagnosticFloatConversion
// ReSharper disable CppClangTidyClangDiagnosticImplicitIntFloatConversion

#include "Framework/Renderer/Renderer.h"
#include "Framework/Engine/Engine.h"


#define DRAW_WIREFRAME 0
#define DRAW_SHADED 1
#define DEPTH_TEST 1

/* Renderer */
PRenderer::PRenderer(uint32 InWidth, uint32 InHeight)
{
    Viewport = std::make_shared<PViewport>(InWidth, InHeight);
    Grid = std::make_unique<FGrid>(8, 4.0f);

    AddChannel(EChannelType::Data, "Depth");
    AddChannel(EChannelType::Color, "Color");
}

void PRenderer::Resize(uint32 InWidth, uint32 InHeight) const
{
    Viewport->Resize(InWidth, InHeight);
    for (auto B : Channels | std::views::values)
    {
        B->Resize(InWidth, InHeight);
    }
}

bool PRenderer::ClipLine(FVector2* A, FVector2* B) const
{
    const int32 MinX = 0;
    const int32 MinY = 0;
    const int32 MaxX = GetWidth() - 1;
    const int32 MaxY = GetHeight() - 1;

    // Cohen-Sutherland line clipping algorithm
    // Compute region codes for both endpoints
    int32 Code1 = (A->X < MinX) << 3 | (A->X > MaxX) << 2 | (A->Y < MinY) << 1 | (A->Y > MaxY);
    int32 Code2 = (B->X < MinX) << 3 | (B->X > MaxX) << 2 | (B->Y < MinY) << 1 | (B->Y > MaxY);

    while (Code1 || Code2)
    {
        // If both endpoints are inside the viewport, exit loop
        if (!(Code1 | Code2))
        {
            break;
        }

        // If both endpoints are outside the viewport and on the same side, discard the line
        if (Code1 & Code2)
        {
            return false;
        }

        // Find the endpoint outside the viewport
        int32 Code = Code1 ? Code1 : Code2;
        int32 X, Y;

        // Find intersection point using the parametric equation of the line
        if (Code & 1)
        {
            // Top edge
            X = A->X + (B->X - A->X) * (MaxY - A->Y) / (B->Y - A->Y);
            Y = MaxY;
        }
        else if (Code & 2)
        {
            // Bottom edge
            X = A->X + (B->X - A->X) * (MinY - A->Y) / (B->Y - A->Y);
            Y = MinY;
        }
        else if (Code & 4)
        {
            // Right edge
            Y = A->Y + (B->Y - A->Y) * (MaxX - A->X) / (B->X - A->X);
            X = MaxX;
        }
        else
        {
            // Left edge
            Y = A->Y + (B->Y - A->Y) * (MinX - A->X) / (B->X - A->X);
            X = MinX;
        }

        // Update the endpoint
        if (Code == Code1)
        {
            A->X = X;
            A->Y = Y;
            Code1 = (A->X < MinX) << 3 | (A->X > MaxX) << 2 | (A->Y < MinY) << 1 | (A->Y > MaxY);
        }
        else
        {
            B->X = X;
            B->Y = Y;
            Code2 = (B->X < MinX) << 3 | (B->X > MaxX) << 2 | (B->Y < MinY) << 1 | (B->Y > MaxY);
        }
    }

    return true;
}

bool PRenderer::ClipLine(FLine* Line) const
{
    return ClipLine(&Line->A, &Line->B);
}

void PRenderer::DrawLine(const FVector3& InA, const FVector3& InB, const PColor& Color) const
{
    FVector2 A(InA.X, InA.Y); // NOLINT
    FVector2 B(InB.X, InB.Y); // NOLINT

    // Clip the screen points within the viewport. If the line points are outside the viewport entirely
    // then just return.
    if (!ClipLine(&A, &B))
    {
        return;
    }
    /* TODO: This is a temporary fix to prevent lines getting clipped and setting one of the points to [0,0]. */
    if (A == 0.0f || B == 0.0f)
    {
        return;
    }

    bool bIsSteep = false;
    if (Math::Abs(A.X - B.X) < Math::Abs(A.Y - B.Y))
    {
        A = FVector2(A.Y, A.X);
        B = FVector2(B.Y, B.X);
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
            GetColorChannel()->SetPixel(Y, X, Color);
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
            GetColorChannel()->SetPixel(X, Y, Color);
            ErrorCount += DeltaError;
            if (ErrorCount > DeltaX)
            {
                Y += (B.Y > A.Y ? 1 : -1);
                ErrorCount -= DeltaX * 2;
            }
        }
    }
}

void PRenderer::DrawLine(const FLine3d& Line, const PColor& Color) const
{
    DrawLine(Line.A, Line.B, Color);
}


void PRenderer::DrawTriangle(float* Data) const
{
    DrawTriangle(
        {Data[0], Data[1], Data[2]},
        {Data[3], Data[4], Data[5]},
        {Data[6], Data[7], Data[8]}
    );
}


void PRenderer::DrawTriangle(const FVector3& V0, const FVector3& V1, const FVector3& V2) const
{
    // Screen points
    FVector3 S0, S1, S2;

    // Project the world-space points to screen-space
    bool bTriangleOnScreen = false;
    bTriangleOnScreen |= Viewport->ProjectWorldToScreen(V0, S0);
    bTriangleOnScreen |= Viewport->ProjectWorldToScreen(V1, S1);
    bTriangleOnScreen |= Viewport->ProjectWorldToScreen(V2, S2);

    // If none of the points are on the screen, the triangle is not in the frame, so exit
    // drawing this triangle
    if (!bTriangleOnScreen)
    {
        return;
    }

#if DRAW_SHADED
    // Reverse the order to CCW if the order is CW
    switch (Math::GetVertexOrder(S0, S1, S2))
    {
    case EWindingOrder::CW : // Triangle is back-facing, exit
        std::swap(S0, S1);
        break;
    case EWindingOrder::CCW :
        break;
    case EWindingOrder::CL : // Triangle has zero area, exit
        return;
    }

    // Get the current camera attributes
    const PCamera* Camera = Viewport->GetCamera();
    const FVector3 LookAtTranslation = Camera->TargetTranslation;
    const FVector3 CameraTranslation = Camera->GetTranslation();

    // Calculate the camera normal (direction) and the world normal of the triangle
    const FVector3 CameraNormal = (LookAtTranslation - CameraTranslation).Normalized();
    const FVector3 WorldNormal = Math::GetSurfaceNormal(V0, V1, V2);

    // Calculate the Camera to Triangle ratio
    const float FacingRatio = Math::Dot(WorldNormal, CameraNormal);

    // If FacingRatio is below 0, the two normals are facing opposite directions, and the face
    // is facing away from the camera.
    if (FacingRatio < 0.0f)
    {
        return;
    }

    // Get the bounding box of the 2d triangle clipped to the viewport
    FRect TriangleRect = FRect::MakeBoundingBox(S0, S1, S2);
    const FRect ViewportRect = {0, 0, static_cast<float>(GetWidth()), static_cast<float>(GetHeight())};
    TriangleRect.Clamp(ViewportRect);

    // Loop through all pixels in the screen bounding box.
    int32 MinX = static_cast<int32>(TriangleRect.Min().X);
    int32 MinY = static_cast<int32>(TriangleRect.Min().Y);
    int32 MaxX = static_cast<int32>(TriangleRect.Max().X);
    int32 MaxY = static_cast<int32>(TriangleRect.Max().Y);

    float Area = Math::Area(S0, S1, S2);
    
    for (int32 Y = MinY; Y < MaxY; Y++)
    {
        for (int32 X = MinX; X < MaxX; X++)
        {
            FVector3 P(static_cast<float>(X), static_cast<float>(Y), 0.0f);

            // Calculate barycentric coordinates at this pixel in the triangle. If this fails,
            // the pixel is not within the triangle.
            FVector3 UVW;
            if (!Math::GetBarycentric(P, S0, S1, S2, UVW))
            {
                continue;
            }

#if DEPTH_TEST
            // Calculate new depth
            float W0 = Math::Area(S1, S2, P);
            float W1 = Math::Area(S2, S0, P);
            float W2 = Math::Area(S0, S1, P);

            if (W0 < 0.0f && W1 < 0.0f && W2 < 0.0f)
            {
                continue;
            }

            W0 /= Area;
            W1 /= Area;
            W2 /= Area;
            
            const float NewDepth = 1.0f / (W0 * S0.Z + W1 * S1.Z + W2 * S2.Z);

            // Compare the new depth to the current depth at this pixel. If the new depth is further than
            // the current depth, continue.
            const float CurrentDepth = GetDepthChannel()->GetPixel<float>(X, Y);
            if (NewDepth >= CurrentDepth)
            {
                continue;
            }

            // If the new depth is closer than the current depth, set the current depth
            // at this pixel to the new depth we just got.
            GetDepthChannel()->SetPixel(X, Y, NewDepth);
#else

#endif // Depth test
            uint8 R = FacingRatio * 255.0f;
            GetColorChannel()->SetPixel(X, Y, PColor::FromRgba(R, R, R));
        }
    }
#endif // Draw shaded

#if DRAW_WIREFRAME
    DrawLine({S0.X, S0.Y}, {S1.X, S1.Y}, WireColor);
    DrawLine({S1.X, S1.Y}, {S2.X, S2.Y}, WireColor);
    DrawLine({S2.X, S2.Y}, {S0.X, S0.Y}, WireColor);
#endif
}

// TODO: Rewrite to use a single array of vertices rather than looping through meshes/triangles
void PRenderer::DrawMesh(const PMesh* Mesh) const
{
    for (uint32 Index = 0; Index < Mesh->GetTriCount(); Index++)
    {
        const uint32 StartIndex = Index * 3;

        const uint32 Index0 = Mesh->Indices[StartIndex];
        const uint32 Index1 = Mesh->Indices[StartIndex + 1];
        const uint32 Index2 = Mesh->Indices[StartIndex + 2];

        DrawTriangle(Mesh->Positions[Index0], Mesh->Positions[Index1], Mesh->Positions[Index2]);
    }
}


void PRenderer::DrawGrid() const
{
    for (const FLine3d& Line : Grid->Lines)
    {
        // Project the world-space points to screen-space
        FVector3 S0, S1;
        bool bLineOnScreen = false;
        bLineOnScreen |= Viewport->ProjectWorldToScreen(Line.A, S0);
        bLineOnScreen |= Viewport->ProjectWorldToScreen(Line.B, S1);

        // If neither of the points are on the screen, return
        if (!bLineOnScreen)
        {
            return;
        }

        DrawLine(S0, S1, GridColor);
    }
}

void PRenderer::Render() const
{
    // Recalculate the view-projection matrix of the camera
    Viewport->UpdateViewProjectionMatrix();

    // Reset all buffers to their default values (namely Z to Inf)
    ClearChannels();

    // Draw the world grid prior to drawing any geometry
    DrawGrid();

    // Draw each mesh
    const PEngine* Engine = PEngine::GetInstance();
    for (const auto& Mesh : Engine->GetMeshes())
    {
        DrawMesh(Mesh.get());
    }
}

void PRenderer::ClearChannels() const
{
    // Set all channels to 0
    for (const auto& [Key, Channel] : Channels)
    {
        // Ignore the depth channel, we'll handle that later
        if (Key == "Depth") // NOLINT
        {
            continue;
        }
        Channel->Clear();
    }

    // Fill the depth buffer with the Max Z-depth
    GetDepthChannel()->Fill(FLT_MAX);
}
