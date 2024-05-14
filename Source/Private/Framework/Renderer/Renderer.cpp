// ReSharper disable CppClangTidyBugproneNarrowingConversions
// ReSharper disable CppClangTidyClangDiagnosticFloatConversion
// ReSharper disable CppClangTidyClangDiagnosticImplicitIntFloatConversion

#include "Framework/Renderer/Renderer.h"
#include "Framework/Engine/Engine.h"
#include "Framework/Renderer/Shader.h"

/* Renderer */

PRenderer::PRenderer(uint32 InWidth, uint32 InHeight)
{
    Viewport = std::make_shared<PViewport>(InWidth, InHeight);
    Grid = std::make_unique<FGrid>(8, 4.0f);

    AddChannel(EChannelType::Data, "Depth");
    AddChannel(EChannelType::Color, "Color");

    // Set default render flags
    Settings = Renderer::PRenderSettings();
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

void PRenderer::DrawLine(const FVector3& InA, const FVector3& InB, const FColor& Color) const
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

void PRenderer::DrawLine(const FLine3d& Line, const FColor& Color) const
{
    DrawLine(Line.A, Line.B, Color);
}

void PRenderer::DrawTriangle(const FVector3& V0, const FVector3& V1, const FVector3& V2)
{
    auto Camera = Viewport->GetCamera();
    const FVector3 CameraNormal = (Camera->LookAt - Camera->GetTranslation()).Normalized();
    CurrentShader = new DefaultShader();
    CurrentShader->Init(
        Camera->ViewProjectionMatrix,
        V0, V1, V2,
        CameraNormal,
        Camera->GetTranslation(),
        Camera->Width, Camera->Height
    );
    
    if (!CurrentShader->ComputeVertexShader())
    {
        return;
    }

    if (Settings.GetRenderFlag(ERenderFlags::Shaded))
    {
        ScanlineFast();
    }
}

// TODO: Rewrite to use a single array of vertices rather than looping through meshes/triangles
void PRenderer::DrawMesh(const PMesh* Mesh)
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

void PRenderer::Draw()
{
    // Recalculate the view-projection matrix of the camera
    Viewport->GetCamera()->ComputeViewProjectionMatrix();

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
void PRenderer::Scanline()
{
    auto S0 = CurrentShader->S0;
    auto S1 = CurrentShader->S1;
    auto S2 = CurrentShader->S2;
    auto Bounds = CurrentShader->ScreenBounds;
    
    // Loop through all pixels in the screen bounding box.
    int32 MinX = static_cast<int32>(Bounds.Min().X);
    int32 MinY = static_cast<int32>(Bounds.Min().Y);
    int32 MaxX = static_cast<int32>(Bounds.Max().X);
    int32 MaxY = static_cast<int32>(Bounds.Max().Y);

    for (int32 Y = MinY; Y <= MaxY; Y++)
    {
        for (int32 X = MinX; X <= MaxX; X++)
        {
            FVector3 P(
                static_cast<float>(X) + 0.5f,
                static_cast<float>(Y) + 0.5f,
                0.0f
            );

            // Calculate barycentric coordinates at this pixel in the triangle. If this fails,
            // the pixel is not within the triangle.
            FVector3 UVW;
            if (!Math::GetBarycentric(P, S0, S1, S2, UVW))
            {
                continue;
            }
            
            if (Settings.GetRenderFlag(ERenderFlags::Depth))
            {
                const float NewDepth = UVW.Z;

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
            }
            CurrentShader->ComputePixelShader();
            GetColorChannel()->SetPixel(X, Y, CurrentShader->OutColor);
        }
    }
}

// https://fgiesen.wordpress.com/2013/02/10/optimizing-the-basic-rasterizer/
void PRenderer::ScanlineFast()
{
    auto V0 = CurrentShader->S0;
    auto V1 = CurrentShader->S1;
    auto V2 = CurrentShader->S2;
    auto Bounds = CurrentShader->ScreenBounds;
    
    int32 A01 = V0.Y - V1.Y;
    int32 A12 = V1.Y - V2.Y;
    int32 A20 = V2.Y - V0.Y;
    int32 B01 = V1.X - V0.X;
    int32 B12 = V2.X - V1.X;
    int32 B20 = V0.X - V2.X;

    IVector3 Point = FVector3(Bounds.Min().X, Bounds.Min().Y, 0).ToType<int32>();
    int32 C0 = Math::Orient2D(V1.ToType<int32>(), V2.ToType<int32>(), Point);
    int32 C1 = Math::Orient2D(V2.ToType<int32>(), V0.ToType<int32>(), Point);
    int32 C2 = Math::Orient2D(V0.ToType<int32>(), V1.ToType<int32>(), Point);

    int32 MinX = static_cast<int32>(Bounds.Min().X);
    int32 MinY = static_cast<int32>(Bounds.Min().Y);
    int32 MaxX = static_cast<int32>(Bounds.Max().X);
    int32 MaxY = static_cast<int32>(Bounds.Max().Y);
    
    for (Point.Y = MinY; Point.Y <= MaxY; Point.Y++)
    {
        int32 W0 = C0;
        int32 W1 = C1;
        int32 W2 = C2;

        for (Point.X = MinX; Point.X <= MaxX; Point.X++)
        {
            if (W0 >= 0 && W1 >= 0 && W2 >= 0)
            {
                // Inside triangle
                if (Settings.GetRenderFlag(ERenderFlags::Depth))
                {
                    const float NewDepth = V0.Z;

                    // Compare the new depth to the current depth at this pixel. If the new depth is further than
                    // the current depth, continue.
                    const float CurrentDepth = GetDepthChannel()->GetPixel<float>(Point.X, Point.Y);
                    if (NewDepth >= CurrentDepth)
                    {
                        continue;
                    }

                    // If the new depth is closer than the current depth, set the current depth
                    // at this pixel to the new depth we just got.
                    GetDepthChannel()->SetPixel(Point.X, Point.Y, NewDepth);
                }
                CurrentShader->ComputePixelShader();
                GetColorChannel()->SetPixel(Point.X, Point.Y, CurrentShader->OutColor);
            }

            W0 += A12;
            W1 += A20;
            W2 += A01;
        }

        C0 += B12;
        C1 += B20;
        C2 += B01;
    }

    if (Settings.GetRenderFlag(ERenderFlags::Wireframe))
    {
        DrawLine({V0.X, V0.Y}, {V1.X, V1.Y}, WireColor);
        DrawLine({V1.X, V1.Y}, {V2.X, V2.Y}, WireColor);
        DrawLine({V2.X, V2.Y}, {V0.X, V0.Y}, WireColor);
    }
}
