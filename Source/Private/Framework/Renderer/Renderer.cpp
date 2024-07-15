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
        // Get the center of the triangle
        FVector3 TriangleCenter = (CurrentShader->V0.Position + CurrentShader->V1.Position + CurrentShader->V2.Position) / 3.0f;

        // Get the computed triangle normal (average of the three normals)
        FVector3 TriangleNormal = CurrentShader->TriangleWorldNormal;

        FVector3 NormalStartScreen;
        FVector3 NormalEndScreen;

        // Compute two screen-space points:
        // 1. The center of the triangle
        // 2. 1 unit out from the center of the triangle, in the direction the triangle is facing
        Math::ProjectWorldToScreen(TriangleCenter, NormalStartScreen, Viewport->GetCamera()->GetViewData());
        Math::ProjectWorldToScreen(TriangleCenter + TriangleNormal, NormalEndScreen, Viewport->GetCamera()->GetViewData());

        // Draw the line between the two points
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
    int32 Width = GetWidth();
    int32 Height = GetHeight();
    const FRect Bounds = CurrentShader->ScreenBounds;
    int32 MinX = Math::Max(static_cast<int32>(Bounds.Min().X), 0);
    int32 MaxX = Math::Min(static_cast<int32>(Bounds.Max().X), Width - 1);
    const int32 MinY = Math::Max(static_cast<int32>(Bounds.Min().Y), 0);
    const int32 MaxY = Math::Min(static_cast<int32>(Bounds.Max().Y), Height - 1);

    // Precompute the area of the screen triangle so we're not computing it every pixel
    const float Area = Math::Area2D(S0, S1, S2) * 2.0f;
    const float OneOverArea = 1.0f / Area;

    int32 InitialOffset = MinY * Width;

    std::shared_ptr<PChannel> DepthChannel = GetDepthChannel();
    float* DepthMemory = static_cast<float*>(DepthChannel->Memory) + InitialOffset; // float, 32-bytes

    std::shared_ptr<PChannel> ColorChannel = GetColorChannel();
    int32* ColorMemory = static_cast<int32*>(ColorChannel->Memory) + InitialOffset; // int32, 32-bytes

    // Prior to the loop computing each pixel in the triangle, get the render settings
    const bool bRenderDepth = Settings.GetRenderFlag(ERenderFlag::Depth);
    float W0, W1, W2;

    const FVector3 UVW0(1, 0, 0);
    const FVector3 UVW1(0, 1, 0);
    const FVector3 UVW2(0, 0, 1);

    const FLine Line01(FVector2(S0.X, S0.Y), FVector2(S1.X, S1.Y));
    const FLine Line12(FVector2(S1.X, S1.Y), FVector2(S2.X, S2.Y));
    const FLine Line20(FVector2(S2.X, S2.Y), FVector2(S0.X, S0.Y));

    const float Depth0 = Math::GetDepth(S0, S0, S1, S2, Area);
    const float Depth1 = Math::GetDepth(S1, S0, S1, S2, Area);
    const float Depth2 = Math::GetDepth(S2, S0, S1, S2, Area);


    // Loop through all pixels in the screen bounding box.
    for (int32 Y = MinY; Y <= MaxY; Y++)
    {
        FVector2 LeftMost(MinX, Y);
        FVector2 RightMost(MaxX, Y);

        FLine RowLine(LeftMost, RightMost);

        for (int32 X = MinX; X <= MaxX; X++)
        {
            FVector3 Point(X, Y, 0);
            FVector3 UVW;
            float* DepthPixel = DepthMemory + X;
            int32* ColorPixel = ColorMemory + X;

            // Use Pineda's edge function to determine if the current pixel is within the triangle.
            W0 = Math::EdgeFunction(S1.X, S1.Y, S2.X, S2.Y, Point.X, Point.Y);
            W1 = Math::EdgeFunction(S2.X, S2.Y, S0.X, S0.Y, Point.X, Point.Y);
            W2 = Math::EdgeFunction(S0.X, S0.Y, S1.X, S1.Y, Point.X, Point.Y);
            
            if (W0 <= 0 || W1 <= 0 || W2 <= 0)
            {
                continue;
            }

            // From the edge vectors, extrapolate the barycentric coordinates for this pixel.
            W0 *= OneOverArea;
            W1 *= OneOverArea;
            W2 *= OneOverArea;

            UVW.X = W0;
            UVW.Y = W1;
            UVW.Z = W2;
            CurrentShader->UVW = UVW;
            
            if (bRenderDepth)
            {
                float NewDepth = UVW.X * Depth0 + UVW.Y * Depth1 + UVW.Z * Depth2;

                // Compare the new depth to the current depth at this pixel. If the new depth is further than
                // the current depth, continue.
                float CurrentDepth = *DepthPixel;
                if (NewDepth > CurrentDepth)
                {
                    continue;
                }
                // If the new depth is closer than the current depth, set the current depth
                // at this pixel to the new depth we just got.
                *DepthPixel = Depth;
            }

            // Compute world position
            CurrentShader->PixelWorldPosition = CurrentShader->V0.Position * UVW.X + CurrentShader->V1.Position * UVW.Y + CurrentShader->V2.Position * UVW.Z;
            
            // Compute world normal
            CurrentShader->PixelWorldNormal = CurrentShader->V0.Normal * UVW.X + CurrentShader->V1.Normal * UVW.Y + CurrentShader->V2.Normal * UVW.Z;
            
            // Compute the final color for this pixel
            CurrentShader->ComputePixelShader(Point.X, Point.Y);

            // Set the current pixel in memory to the computed color
            *ColorPixel = CurrentShader->OutColor.ToInt32();
        }
        DepthMemory += Width;
        ColorMemory += Width;
    }
}
