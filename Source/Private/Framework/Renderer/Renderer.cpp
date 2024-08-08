#include "Framework/Renderer/Renderer.h"

#include "Framework/Core/PThreadPool.h"
#include "Framework/Engine/Engine.h"
#include "Framework/Renderer/Shader.h"

/* Renderer */

PRenderer::PRenderer(uint32 InWidth, uint32 InHeight)
{
	Viewport = std::make_shared<PViewport>(InWidth, InHeight);
	Width = &Viewport->Camera->Width;
	Height = &Viewport->Camera->Height;
	Grid = std::make_unique<FGrid>(8, 4.0f);

	AddChannel(EChannelType::Data, "Depth");
	AddChannel(EChannelType::Color, "Color");

	// Set default render flags
	Settings = Renderer::PRenderSettings();

	// Default shader
	CurrentShader = std::make_shared<DefaultShader>();
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
	FVector2 A(InA.X, InA.Y);	 // NOLINT
	FVector2 B(InB.X, InB.Y);	 // NOLINT

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
		DrawLine(NormalStartScreen,	   // Start
			NormalEndScreen,		   // End
			FColor::Yellow());
	}
}

// TODO: Rewrite to use a single array of vertices rather than looping through meshes/triangles
void PRenderer::DrawMesh(PMesh* Mesh)
{
	for (const auto& T : Mesh->Triangles)
	{
		DrawTriangle(T.V0, T.V1, T.V2);
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
	const IInputHandler* InputHandler = IInputHandler::GetInstance();
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

void PRenderer::RasterizeTriangle(const PTriangleRenderData& RenderData)
{
	FVector3 S0 = RenderData.ScreenPoints[0];
	FVector3 S1 = RenderData.ScreenPoints[1];
	FVector3 S2 = RenderData.ScreenPoints[2];
	float OneOverArea = RenderData.OneOverArea;
	float Depth0 = RenderData.DepthPoints[0];
	float Depth1 = RenderData.DepthPoints[1];
	float Depth2 = RenderData.DepthPoints[2];
	int32 ScreenWidth = *Width;

	float* DepthMemory = RenderData.DepthMemory;
	int32* ColorMemory = RenderData.ColorMemory;

	const FRect Bounds = CurrentShader->ScreenBounds;
	const int32 MinX = Math::Max(static_cast<int32>(Bounds.Min().X), 0);
	const int32 MaxX = Math::Min(static_cast<int32>(Bounds.Max().X), *Width - 1);
	const int32 MinY = Math::Max(static_cast<int32>(Bounds.Min().Y), 0);
	const int32 MaxY = Math::Min(static_cast<int32>(Bounds.Max().Y), *Height - 1);

	for (int32 Y = MinY; Y <= MaxY; Y++)
	{
		for (int32 X = MinX; X <= MaxX; X++)
		{
			FVector3 Point(X, Y, 0);

			// Use Pineda's edge function to determine if the current pixel is within the triangle.
			float W0 = Math::EdgeFunction(S1.X, S1.Y, S2.X, S2.Y, Point.X, Point.Y);
			float W1 = Math::EdgeFunction(S2.X, S2.Y, S0.X, S0.Y, Point.X, Point.Y);
			float W2 = Math::EdgeFunction(S0.X, S0.Y, S1.X, S1.Y, Point.X, Point.Y);

			if (W0 <= 0.0f || W1 <= 0.0f || W2 <= 0.0f)
			{
				continue;
			}

			FVector3 UVW;
			float* DepthPixel = DepthMemory + X;
			int32* ColorPixel = ColorMemory + X;

			// From the edge vectors, extrapolate the barycentric coordinates for this pixel.
			W0 *= OneOverArea;
			W1 *= OneOverArea;
			W2 *= OneOverArea;

			UVW.X = W0;
			UVW.Y = W1;
			UVW.Z = W2;
			CurrentShader->UVW = UVW;

			// Interpolate depth given UVW
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
			*DepthPixel = NewDepth;

			// Compute world position
			CurrentShader->PixelWorldPosition =
				CurrentShader->V0.Position * UVW.X + CurrentShader->V1.Position * UVW.Y + CurrentShader->V2.Position * UVW.Z;

			// Compute the final color for this pixel
			CurrentShader->ComputePixelShader(Point.X, Point.Y);

			// Set the current pixel in memory to the computed color
			*ColorPixel = CurrentShader->OutColor.ToInt32();
		}
		DepthMemory += ScreenWidth;
		ColorMemory += ScreenWidth;
	}

	ThreadsComplete++;
}
void PRenderer::Scanline()
{
	FVector3 S0 = CurrentShader->S0;
	FVector3 S1 = CurrentShader->S1;
	FVector3 S2 = CurrentShader->S2;

	// Compute the bounds of just this triangle on the screen
	int32 ScreenWidth = *Width;
	int32 ScreenHeight = *Height;

	int32 BucketRowCount = ScreenWidth / BucketSize;
	int32 BucketColumnCount = ScreenHeight / BucketSize;

	FRect ScreenBounds = CurrentShader->ScreenBounds;

	// Precompute the area of the screen triangle so we're not computing it every pixel
	const float Area = Math::Area2D(S0, S1, S2) * 2.0f;
	const float OneOverArea = 1.0f / Area;

	// Offset in memory to the top-left corner of this triangle's screen bounding box
	int32 InitialOffset = static_cast<int32>(ScreenBounds.Y) * ScreenWidth;

	std::shared_ptr<PChannel> DepthChannel = GetDepthChannel();
	float* DepthMemory = static_cast<float*>(DepthChannel->Memory) + InitialOffset;	   // float, 32-bytes

	std::shared_ptr<PChannel> ColorChannel = GetColorChannel();
	int32* ColorMemory = static_cast<int32*>(ColorChannel->Memory) + InitialOffset;	   // int32, 32-bytes

	const float Depth0 = Math::GetDepth(S0, S0, S1, S2, Area);
	const float Depth1 = Math::GetDepth(S1, S0, S1, S2, Area);
	const float Depth2 = Math::GetDepth(S2, S0, S1, S2, Area);

	// TODO: Make a thread pool
	FVector3 ScreenPoints[3] = {S0, S1, S2};
	float DepthPoints[3] = {Depth0, Depth1, Depth2};
	// for (int32 ThreadIndex = 0; ThreadIndex < ThreadCount; ThreadIndex++)
	// {
	// 	// Construct the data struct to easily pass all necessary variables to the rasterization function
	PTriangleRenderData RenderData(DepthMemory, ColorMemory, ScreenPoints, DepthPoints, ScreenBounds, OneOverArea);

	if (!Settings.IsMultithreaded())
	{
		// Single threaded
		PRenderer::RasterizeTriangle(RenderData);
	}
	else
	{
		// Multithreaded

		// Create a new thread
		auto Thread = std::thread(&PRenderer::RasterizeTriangle, this, RenderData);
		//
		// // Join this thread once it's complete
		Thread.join();
	}
	// }
}

void PRenderer::ScanlineFast()
{
	FVector3 S0 = CurrentShader->S0;
	FVector3 S1 = CurrentShader->S1;
	FVector3 S2 = CurrentShader->S2;

	FVector2 UV0, UV1, UV2;

	// Sort by Y value
	if (S1.Y < S0.Y)
	{
		std::swap(S0, S1);
		std::swap(UV0, UV1);
	}
	if (S2.Y < S0.Y)
	{
		std::swap(S0, S2);
		std::swap(UV0, UV2);
	}
	if (S2.Y < S1.Y)
	{
		std::swap(S1, S2);
		std::swap(UV1, UV2);
	}

	int32 X0 = S0.X;
	int32 X1 = S1.X;
	int32 X2 = S2.X;
	int32 Y0 = S0.Y;
	int32 Y1 = S1.Y;
	int32 Y2 = S2.Y;

	float U0 = UV0.X;
	float U1 = UV1.X;
	float U2 = UV2.X;
	float V0 = UV0.Y;
	float V1 = UV1.Y;
	float V2 = UV2.Y;

	// Determine two edges
	int32 DY0 = S1.Y - S0.Y;
	int32 DX0 = S1.X - S0.X;
	float DV0 = V1 - V0;
	float DU0 = U1 - U0;

	int32 DY1 = S2.Y - S0.Y;
	int32 DX1 = S2.X - S0.X;
	float DV1 = V2 - V0;
	float DU1 = U2 - U0;

	float TexU, TexV;
	// Precompute the area of the screen triangle so we're not computing it every pixel
	const float Area = Math::Area2D(S0, S1, S2) * 2.0f;
	const float OneOverArea = 1.0f / Area;

	// Delta X Step
	float DAXS = 0;
	float DBXS = 0;
	float DU0S = 0;
	float DV0S = 0;
	float DU1S = 0;
	float DV1S = 0;

	if (DY0)
	{
		DAXS = DX0 / (float) std::abs(DY0);
	}
	if (DY1)
	{
		DBXS = DX1 / (float) std::abs(DY1);
	}

	if (DY0)
	{
		DU0S = DU0 / (float) std::abs(DY0);
	}
	if (DY0)
	{
		DV0S = DV0 / (float) std::abs(DY0);
	}

	if (DY1)
	{
		DU1S = DU1 / (float) std::abs(DY1);
	}
	if (DY1)
	{
		DV1S = DV1 / (float) std::abs(DY1);
	}

	std::shared_ptr<PChannel> ColorChannel = GetColorChannel();

	// If our line exists vertically
	if (DY0)
	{
		for (int32 Y = Y0; Y <= Y1; Y++)
		{
			int32 AX = X0 + (float) (Y - Y0) * DAXS;
			int32 BX = X0 + (float) (Y - Y0) * DBXS;

			float TexStartU = U0 + (float) (Y - Y0) * DU0S;
			float TexStartV = V0 + (float) (Y - Y0) * DV0S;
			float TexEndU = U0 + (float) (Y - Y0) * DU1S;
			float TexEndV = V0 + (float) (Y - Y0) * DV1S;

			if (AX > BX)
			{
				std::swap(AX, BX);
				std::swap(TexStartU, TexEndU);
				std::swap(TexStartV, TexEndV);
			}

			TexU = TexStartU;
			TexV = TexStartV;

			float LineStep = 1.0f / (float) (BX - AX);	  // bx - ax are the number of x pixels that make up the scanline
			float CurrentStep = 0.0f;					  // How far across the scanline we are

			for (int32 X = AX; X < BX; X++)
			{
				TexU = (1.0f - CurrentStep) * TexStartU + CurrentStep * TexEndU;
				TexV = (1.0f - CurrentStep) * TexStartV + CurrentStep * TexEndV;

				// Use Pineda's edge function to determine if the current pixel is within the triangle.
				float W0 = Math::EdgeFunction(X1, Y1, X2, Y2, X, Y);
				float W1 = Math::EdgeFunction(X2, Y2, X0, Y0, X, Y);
				float W2 = Math::EdgeFunction(X0, Y0, X1, Y1, X, Y);

				CurrentStep += LineStep;

				W0 *= OneOverArea;
				W1 *= OneOverArea;
				W2 *= OneOverArea;

				FVector3 UVW;
				UVW.X = W0;
				UVW.Y = W1;
				UVW.Z = W2;

				FColor Color = FColor::FromRgba(W0 * 255, W1 * 255, W2 * 255);

				// Draw this pixel
				ColorChannel->SetPixel(X, Y, Color.ToInt32());
			}
		}

		DY0 = Y2 - Y1;
		DX0 = X2 - X1;

		if (DY0)
		{
			DAXS = DX0 / (float) std::abs(DY0);
		}
		if (DY1)
		{
			DBXS = DX1 / (float) std::abs(DY1);
		}

		for (int32 Y = Y1; Y <= Y2; Y++)
		{
			int32 AX = X1 + (float) (Y - Y1) * DAXS;
			int32 BX = X0 + (float) (Y - Y0) * DBXS;

			float TexStartU = U1 + (float) (Y - Y1) * DU0S;
			float TexStartV = V1 + (float) (Y - Y1) * DV0S;
			float TexEndU = U0 + (float) (Y - Y0) * DU1S;
			float TexEndV = V0 + (float) (Y - Y0) * DV1S;

			if (AX > BX)
			{
				std::swap(AX, BX);
				std::swap(TexStartU, TexEndU);
				std::swap(TexStartV, TexEndV);
			}

			float LineStep = 1.0f / (float) (BX - AX);	  // bx - ax are the number of x pixels that make up the scanline
			float CurrentStep = 0.0f;					  // How far across the scanline we are

			for (int32 X = AX; X < BX; X++)
			{
				// Use Pineda's edge function to determine if the current pixel is within the triangle.
				float W0 = Math::EdgeFunction(X1, Y1, X2, Y2, X, Y);
				float W1 = Math::EdgeFunction(X2, Y2, X0, Y0, X, Y);
				float W2 = Math::EdgeFunction(X0, Y0, X1, Y1, X, Y);

				CurrentStep += LineStep;

				W0 *= OneOverArea;
				W1 *= OneOverArea;
				W2 *= OneOverArea;

				FVector3 UVW;
				UVW.X = W0;
				UVW.Y = W1;
				UVW.Z = W2;

				FColor Color = FColor::FromRgba(W0 * 255, W1 * 255, W2 * 255);

				// Draw this pixel
				ColorChannel->SetPixel(X, Y, Color.ToInt32());
			}
		}
	}
}
