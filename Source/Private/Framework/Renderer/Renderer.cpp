#include <algorithm>
#include "Framework/Renderer/Renderer.h"
#include "Framework/Engine/Engine.h"


#define DRAW_WIREFRAME 0
#define DRAW_SHADED 1
#define DEPTH_TEST 1
#define SCANLINE 0

/* Renderer */
PRenderer::PRenderer(uint32 InWidth, uint32 InHeight)
{
    Viewport = std::make_shared<PViewport>(InWidth, InHeight);
    Grid = std::make_unique<FGrid>(8, 4.0f);

    AddBuffer(EBufferType::Data, "Depth");
    AddBuffer(EBufferType::Color, "Color");
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

void PRenderer::DrawLine(const FLine3d& Line, const PColor& Color) const
{
    DrawLine(Line.A, Line.B, Color);
}

void PRenderer::DrawTriangle(const FVector3& V0, const FVector3& V1, const FVector3& V2) const
{
    // Screen points
    int32 Width = GetWidth();
    int32 Height = GetHeight();
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
    case EWindingOrder::CW :
        std::swap(S0, S1);
        break;
    case EWindingOrder::CCW :
        break;
    case EWindingOrder::CL :
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
    const float FacingRatio = Math::Dot(CameraNormal, WorldNormal);

    // If FacingRatio is above 0, the two normals are facing opposite directions, and the face
    // is facing away from the camera.
    if (FacingRatio > 0.0f)
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

#if SCANLINE
    std::vector<IVector2> Points;
    Math::GetLine(S0, S1, Points, Width, Height);
    Math::GetLine(S1, S2, Points, Width, Height);
    Math::GetLine(S2, S0, Points, Width, Height);

    // Clip points to screen boundaries
    for (auto& Point : Points)
    {
        Point.X = Math::Max(0, Math::Min(Point.X, Width - 1));
        Point.Y = Math::Max(0, Math::Min(Point.Y, Height - 1));
    }

    // Filling the triangle
    for (int Y = MinY; Y < MaxY; ++Y)
    {
        std::vector<int> Intersections;
        for (size_t Index = 0; Index < Points.size(); ++Index)
        {
            int NextIndex = (Index + 1) % Points.size();
            int X0 = Points[Index].X;
            int Y0 = Points[Index].Y;
            int X1 = Points[NextIndex].X;
            int Y1 = Points[NextIndex].Y;

            // Check if the scanline intersects with the edge
            if ((Y0 <= Y && Y1 > Y) || (Y1 <= Y && Y0 > Y))
            {
                int XIntersection = (X0 + (static_cast<double>(Y - Y0) / static_cast<double>(Y1 - Y0)) * (X1 - X0));
                Intersections.push_back(XIntersection);
            }
        }

        // Sort intersections to get the pairs of x coordinates for the scanline
        std::ranges::sort(Intersections);
        for (size_t Index = 0; Index < Intersections.size(); Index += 2)
        {
            int32 X0 = Intersections[Index];
            int32 X1 = Intersections[Index + 1];
            for (int32 X = X0; X < X1; ++X)
            {
#if DEPTH_TEST
                // Calculate new depth
                FVector3 UVW = Math::GetBarycentric(FVector3{static_cast<float>(X), static_cast<float>(Y)}, S0, S1, S2, UVW);
                const float NewDepth = 1.0f / (UVW.X * S0.Z + UVW.Y * S1.Z + UVW.Z * S2.Z);

                // Compare the new depth to the current depth at this pixel. If the new depth is further than
                // the current depth, continue.
                const float CurrentDepth = static_cast<float>(GetDepthBuffer()->GetPixel(X, Y));
                if (NewDepth > CurrentDepth)
                {
                    continue;
                }

                // If the new depth is closer than the current depth, set the current depth
                // at this pixel to the new depth we just got.
                GetDepthBuffer()->SetPixel(X, Y, NewDepth);
                const float RemappedDepth = Math::Remap(NewDepth, 1.0f, 10.0f, 0.0f, 1.0f);
                uint8 R = static_cast<uint8>(RemappedDepth * 255.0f);
#else
                uint8 R = static_cast<uint8>(Math::Abs(FacingRatio) * 255.0f);
#endif // Depth test
                GetColorBuffer()->SetPixel(X, Y, PColor::FromRgba(R, 0, 0));
            }
        }
    }

#else // Normal barycentric
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
            const float NewDepth = 1.0f / (UVW.X * S0.Z + UVW.Y * S1.Z + UVW.Z * S2.Z);

            // Compare the new depth to the current depth at this pixel. If the new depth is further than
            // the current depth, continue.
            const float CurrentDepth = static_cast<float>(GetDepthBuffer()->GetPixel(X, Y));
            if (NewDepth >= CurrentDepth)
            {
                continue;
            }

            // If the new depth is closer than the current depth, set the current depth
            // at this pixel to the new depth we just got.
            GetDepthBuffer()->SetPixel(X, Y, NewDepth);
            const float RemappedDepth = Math::Remap(NewDepth, 0.0f, 2.0f, 0.0f, 1.0f);
            const uint8 R = static_cast<uint8>(RemappedDepth * 255.0f);
#else
                const uint8 R = static_cast<uint8>(Math::Abs(FacingRatio) * 255.0f);
#endif // Depth test
            GetColorBuffer()->SetPixel(X, Y, PColor::FromRgba(R, 0, 0));
        }
    }
#endif // Scanline
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
    ClearBuffers();

    // Draw the world grid prior to drawing any geometry
    DrawGrid();

    // Draw each mesh
    const PEngine* Engine = PEngine::GetInstance();
    for (const auto& Mesh : Engine->GetMeshes())
    {
        DrawMesh(Mesh.get());
    }
}

void PRenderer::ClearBuffers() const
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
    GetDepthBuffer()->Fill(FLT_MAX); // Viewport->GetCamera()->MaxZ
}
