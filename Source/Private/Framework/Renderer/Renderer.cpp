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

void PRenderer::DrawTriangle(const PVertex& V0, const PVertex& V1, const PVertex& V2)
{
    PCamera* Camera = Viewport->GetCamera();
    CurrentShader->Init(Camera->GetViewData());

    if (!CurrentShader->ComputeVertexShader(V0, V1, V2))
    {
        return;
    }

    if (Settings.GetRenderFlag(ERenderFlag::Shaded))
    {
        CurrentShader->ViewMatrix = Camera->ViewMatrix;
        Scanline();
    }

    FVector3 S0 = CurrentShader->S0;
    FVector3 S1 = CurrentShader->S1;
    FVector3 S2 = CurrentShader->S2;
    if (Settings.GetRenderFlag(ERenderFlag::Wireframe))
    {
        DrawLine({S0.X, S0.Y}, {S1.X, S1.Y}, WireColor);
        DrawLine({S1.X, S1.Y}, {S2.X, S2.Y}, WireColor);
        DrawLine({S2.X, S2.Y}, {S0.X, S0.Y}, WireColor);
    }

    // Draw normal direction
    if (Settings.GetRenderFlag(ERenderFlag::Normals))
    {
        FVector3 TriangleCenter = (CurrentShader->V0.Position + CurrentShader->V1.Position + CurrentShader->V2.Position) / 3.0f;
        FVector3 TriangleNormal = CurrentShader->TriangleWorldNormal;

        FVector3 NormalStartScreen;
        FVector3 NormalEndScreen;
        Math::ProjectWorldToScreen(TriangleCenter, NormalStartScreen, Viewport->GetCamera()->GetViewData());
        Math::ProjectWorldToScreen(TriangleCenter + TriangleNormal, NormalEndScreen, Viewport->GetCamera()->GetViewData());
        DrawLine(
            NormalStartScreen, // Start
            NormalEndScreen,   // End
            FColor::Yellow()
        );
    }
}

// TODO: Rewrite to use a single array of vertices rather than looping through meshes/triangles
void PRenderer::DrawMesh(const PMesh* Mesh)
{
    CurrentShader = std::make_shared<DefaultShader>();
    for (const auto& Triangle : Mesh->Triangles)
    {
        PVertex V0;
        PVertex V1;
        PVertex V2;
        Mesh->ProcessTriangle(Triangle, &V0, &V1, &V2);
        DrawTriangle(V0, V1, V2);
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

    // Draw mouse cursor line from click origin
    IInputHandler* InputHandler = IInputHandler::GetInstance();
    if (InputHandler->IsMouseDown(EMouseButtonType::Left) && InputHandler->IsAltDown())
    {
        FVector3 A = InputHandler->GetClickPosition();
        if (A.X != 0.0f && A.Y != 0.0f)
        {
            FVector3 B = InputHandler->GetCurrentCursorPosition();
            DrawLine(A, B, FColor::Red());
        }
    }
}
void PRenderer::Scanline()
{
    FVector3 S0 = CurrentShader->S0;
    FVector3 S1 = CurrentShader->S1;
    FVector3 S2 = CurrentShader->S2;

    switch (Math::GetWindingOrder(S0, S1, S2))
    {
    case EWindingOrder::CCW : // Counter-clockwise, front-facing
        break;
    case EWindingOrder::CW : // Clockwise, back-facing; exit
    case EWindingOrder::CL : // Co-linear; exit
        return;
    }

    // Compute the bounds of just this triangle on the screen
    const FRect Bounds = CurrentShader->ScreenBounds;
    const int32 MinX = static_cast<int32>(Bounds.Min().X);
    const int32 MinY = static_cast<int32>(Bounds.Min().Y);
    const int32 MaxX = static_cast<int32>(Bounds.Max().X);
    const int32 MaxY = static_cast<int32>(Bounds.Max().Y);

    // Precompute the area of the screen triangle so we're not computing it every pixel
    const float Area = Math::Area2D(S0, S1, S2) * 2;

    // Loop through all pixels in the screen bounding box.
    for (int32 Y = MinY; Y <= MaxY; Y++)
    {
        for (int32 X = MinX; X <= MaxX; X++)
        {
            FVector3 Point(
                static_cast<float>(X) + 0.5f,
                static_cast<float>(Y) + 0.5f,
                0.0f
            );

            // Use Pineda's edge function to determine if the current pixel is within the triangle.
            float W0 = Math::EdgeFunction(S1, S2, Point);
            float W1 = Math::EdgeFunction(S2, S0, Point);
            float W2 = Math::EdgeFunction(S0, S1, Point);

            if (W0 < 0 || W1 < 0 || W2 < 0)
            {
                continue;
            }

            // From the edge vectors, extrapolate the barycentric coordinates for this pixel.
            W0 /= Area;
            W1 /= Area;
            W2 /= Area;

            FVector3 UVW;
            UVW.X = W0 * 1.0f + W0 * 0.0f + W0 * 0.0f ;
            UVW.Y = W1 * 0.0f + W1 * 1.0f + W1 * 0.0f ;
            UVW.Z = W2 * 0.0f + W2 * 0.0f + W2 * 1.0f ;

            if (Settings.GetRenderFlag(ERenderFlag::Depth))
            {
                const float Depth = Math::GetDepth(Point.ToType<float>(), S0, S1, S2, Area);

                // Compare the new depth to the current depth at this pixel. If the new depth is further than
                // the current depth, continue.
                const float CurrentDepth = GetDepthChannel()->GetPixel<float>(Point.X, Point.Y);
                if (Depth >= CurrentDepth)
                {
                    continue;
                }
                // If the new depth is closer than the current depth, set the current depth
                // at this pixel to the new depth we just got.
                GetDepthChannel()->SetPixel(Point.X, Point.Y, Depth);
            }
            CurrentShader->UVW = UVW;
            CurrentShader->PixelWorldPosition = CurrentShader->V0.Position * UVW.X + CurrentShader->V1.Position * UVW.Y + CurrentShader->V2.Position * UVW.Z;
            CurrentShader->PixelWorldNormal = CurrentShader->V0.Normal * UVW.X + CurrentShader->V1.Normal * UVW.Y + CurrentShader->V2.Normal * UVW.Z;
            CurrentShader->ComputePixelShader(Point.X, Point.Y);
            GetColorChannel()->SetPixel(Point.X, Point.Y, CurrentShader->OutColor);
        }
    }
}
