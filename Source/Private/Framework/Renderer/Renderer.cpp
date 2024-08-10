// ReSharper disable CppClangTidyBugproneNarrowingConversions
// ReSharper disable CppClangTidyClangDiagnosticFloatConversion
// ReSharper disable CppClangTidyClangDiagnosticImplicitIntFloatConversion

#include "Framework/Renderer/Renderer.h"
#include "Framework/Engine/Engine.h"
#include "Framework/Renderer/Shader.h"

/* Renderer */

PRenderer::PRenderer(uint32 inWidth, uint32 inHeight)
{
	m_viewport = std::make_shared<PViewport>(inWidth, inHeight);
	m_grid = std::make_unique<FGrid>(8, 4.0f);

	addChannel(EChannelType::Data, "Depth");
	addChannel(EChannelType::Color, "Color");

	// Set default render flags
	m_settings = Renderer::PRenderSettings();

	// Default shader
	m_currentShader = std::make_shared<DefaultShader>();
}

void PRenderer::resize(const uint32 inWidth, const uint32 inHeight) const
{
	m_viewport->Resize(inWidth, inHeight);
	for (const auto& channel : m_channels | std::views::values)
	{
		channel->resize(inWidth, inHeight);
	}
}

bool PRenderer::clipLine(FVector2* a, FVector2* b) const
{
	const int32 minX = 0;
	const int32 minY = 0;
	const int32 maxX = getWidth() - 1;
	const int32 maxY = getHeight() - 1;

	// Cohen-Sutherland line clipping algorithm
	// Compute region codes for both endpoints
	int32 code1 = (a->X < minX) << 3 | (a->X > maxX) << 2 | (a->Y < minY) << 1 | (a->Y > maxY);
	int32 code2 = (b->X < minX) << 3 | (b->X > maxX) << 2 | (b->Y < minY) << 1 | (b->Y > maxY);

	while (code1 || code2)
	{
		// If both endpoints are inside the viewport, exit loop
		if (!(code1 | code2))
		{
			break;
		}

		// If both endpoints are outside the viewport and on the same side, discard the line
		if (code1 & code2)
		{
			return false;
		}

		// Find the endpoint outside the viewport
		const int32 code = code1 ? code1 : code2;
		int32 x, y;

		// Find intersection point using the parametric equation of the line
		if (code & 1)
		{
			// Top edge
			x = a->X + (b->X - a->X) * (maxY - a->Y) / (b->Y - a->Y);
			y = maxY;
		}
		else if (code & 2)
		{
			// Bottom edge
			x = a->X + (b->X - a->X) * (minY - a->Y) / (b->Y - a->Y);
			y = minY;
		}
		else if (code & 4)
		{
			// Right edge
			y = a->Y + (b->Y - a->Y) * (maxX - a->X) / (b->X - a->X);
			x = maxX;
		}
		else
		{
			// Left edge
			y = a->Y + (b->Y - a->Y) * (minX - a->X) / (b->X - a->X);
			x = minX;
		}

		// Update the endpoint
		if (code == code1)
		{
			a->X = x;
			a->Y = y;
			code1 = (a->X < minX) << 3 | (a->X > maxX) << 2 | (a->Y < minY) << 1 | (a->Y > maxY);
		}
		else
		{
			b->X = x;
			b->Y = y;
			code2 = (b->X < minX) << 3 | (b->X > maxX) << 2 | (b->Y < minY) << 1 | (b->Y > maxY);
		}
	}

	return true;
}

bool PRenderer::clipLine(FLine* line) const
{
	return clipLine(&line->A, &line->B);
}

void PRenderer::drawLine(const FVector3& inA, const FVector3& inB, const FColor& color) const
{
	FVector2 a(inA.X, inA.Y);
	FVector2 b(inB.X, inB.Y);

	// Clip the screen points within the viewport. If the line points are outside the viewport entirely
	// then just return.
	if (!clipLine(&a, &b))
	{
		return;
	}
	/* TODO: This is a temporary fix to prevent lines getting clipped and setting one of the points to [0,0]. */
	if (a == 0.0f || b == 0.0f)
	{
		return;
	}

	bool bIsSteep = false;
	if (Math::Abs(a.X - b.X) < Math::Abs(a.Y - b.Y))
	{
		a = FVector2(a.Y, a.X);
		b = FVector2(b.Y, b.X);
		bIsSteep = true;
	}

	if (a.X > b.X)
	{
		std::swap(a, b);
	}

	const int32 deltaX = b.X - a.X;
	const int32 deltaY = b.Y - a.Y;
	const int32 deltaError = Math::Abs(deltaY) * 2;
	int32 errorCount = 0;

	// https://github.com/ssloy/tinyrenderer/issues/28
	int32 y = a.Y;

	if (bIsSteep)
	{
		for (int32 x = a.X; x < b.X; ++x)
		{
			getColorChannel()->setPixel(y, x, color);
			errorCount += deltaError;
			if (errorCount > deltaX)
			{
				y += (b.Y > a.Y ? 1 : -1);
				errorCount -= deltaX * 2;
			}
		}
	}
	else
	{
		for (int32 x = a.X; x < b.X; ++x)
		{
			getColorChannel()->setPixel(x, y, color);
			errorCount += deltaError;
			if (errorCount > deltaX)
			{
				y += (b.Y > a.Y ? 1 : -1);
				errorCount -= deltaX * 2;
			}
		}
	}
}

void PRenderer::drawLine(const FLine3d& line, const FColor& color) const
{
	drawLine(line.A, line.B, color);
}

void PRenderer::drawTriangle(const PVertex& v0, const PVertex& v1, const PVertex& v2)
{
	const PCamera* camera = m_viewport->GetCamera();
	m_currentShader->Init(camera->getViewData());

	if (!m_currentShader->ComputeVertexShader(v0, v1, v2))
	{
		return;
	}

	if (m_settings.GetRenderFlag(ERenderFlag::Shaded))
	{
		m_currentShader->ViewMatrix = camera->m_viewMatrix;
		scanline();
	}

	FVector3 s0 = m_currentShader->S0;
	FVector3 s1 = m_currentShader->S1;
	FVector3 s2 = m_currentShader->S2;
	if (m_settings.GetRenderFlag(ERenderFlag::Wireframe))
	{
		drawLine({s0.X, s0.Y}, {s1.X, s1.Y}, m_wireColor);
		drawLine({s1.X, s1.Y}, {s2.X, s2.Y}, m_wireColor);
		drawLine({s2.X, s2.Y}, {s0.X, s0.Y}, m_wireColor);
	}

	// Draw normal direction
	if (m_settings.GetRenderFlag(ERenderFlag::Normals))
	{
		// Get the center of the triangle
		FVector3 triangleCenter = (m_currentShader->V0.Position + m_currentShader->V1.Position + m_currentShader->V2.
				Position)
			/ 3.0f;

		// Get the computed triangle normal (average of the three normals)
		FVector3 triangleNormal = m_currentShader->TriangleWorldNormal;

		FVector3 normalStartScreen;
		FVector3 normalEndScreen;

		// Compute two screen-space points:
		// 1. The center of the triangle
		// 2. 1 unit out from the center of the triangle, in the direction the triangle is facing
		Math::projectWorldToScreen(triangleCenter, normalStartScreen, m_viewport->GetCamera()->getViewData());
		Math::projectWorldToScreen(triangleCenter + triangleNormal, normalEndScreen,
		                           m_viewport->GetCamera()->getViewData());

		// Draw the line between the two points
		drawLine(
			normalStartScreen, // Start
			normalEndScreen, // End
			FColor::Yellow()
		);
	}
}

// TODO: Rewrite to use a single array of vertices rather than looping through meshes/triangles
void PRenderer::drawMesh(const PMesh* mesh)
{
	for (const auto& triangle : mesh->Triangles)
	{
		drawTriangle(triangle.V0, triangle.V1, triangle.V2);
	}
}

void PRenderer::drawGrid() const
{
	for (const FLine3d& line : m_grid->Lines)
	{
		// Project the world-space points to screen-space
		FVector3 s0, s1;
		bool lineOnScreen = false;
		lineOnScreen |= m_viewport->ProjectWorldToScreen(line.A, s0);
		lineOnScreen |= m_viewport->ProjectWorldToScreen(line.B, s1);

		// If neither of the points are on the screen, return
		if (!lineOnScreen)
		{
			return;
		}

		drawLine(s0, s1, m_gridColor);
	}
}

void PRenderer::draw()
{
	// Recalculate the view-projection matrix of the camera
	m_viewport->GetCamera()->computeViewProjectionMatrix();

	// Reset all buffers to their default values (namely Z to Inf)
	clearChannels();

	// Draw the world grid prior to drawing any geometry
	drawGrid();

	// Draw each mesh
	const PEngine* engine = PEngine::getInstance();
	for (const auto& mesh : engine->getMeshes())
	{
		drawMesh(mesh.get());
	}

	// Draw mouse cursor line from click origin
	const IInputHandler* inputHandler = IInputHandler::GetInstance();
	if (inputHandler->IsMouseDown(EMouseButtonType::Left) && inputHandler->IsAltDown())
	{
		const FVector3 a = inputHandler->GetClickPosition();
		if (a.X != 0.0f && a.Y != 0.0f)
		{
			const FVector3 b = inputHandler->GetCurrentCursorPosition();
			drawLine(a, b, FColor::Red());
		}
	}
}

void PRenderer::scanline()
{
	FVector3 S0 = m_currentShader->S0;
	FVector3 S1 = m_currentShader->S1;
	FVector3 S2 = m_currentShader->S2;

	// Compute the bounds of just this triangle on the screen
	int32 Width = getWidth();
	int32 Height = getHeight();
	const FRect Bounds = m_currentShader->ScreenBounds;
	const int32 MinX = Math::Max(static_cast<int32>(Bounds.Min().X), 0);
	const int32 MaxX = Math::Min(static_cast<int32>(Bounds.Max().X), Width - 1);
	const int32 MinY = Math::Max(static_cast<int32>(Bounds.Min().Y), 0);
	const int32 MaxY = Math::Min(static_cast<int32>(Bounds.Max().Y), Height - 1);

	// Precompute the area of the screen triangle so we're not computing it every pixel
	const float Area = Math::Area2D(S0, S1, S2) * 2.0f;
	const float OneOverArea = 1.0f / Area;

	int32 InitialOffset = MinY * Width;

	std::shared_ptr<PChannel> DepthChannel = getDepthChannel();
	float* DepthMemory = static_cast<float*>(DepthChannel->m_memory) + InitialOffset; // float, 32-bytes

	std::shared_ptr<PChannel> ColorChannel = getColorChannel();
	int32* ColorMemory = static_cast<int32*>(ColorChannel->m_memory) + InitialOffset; // int32, 32-bytes

	// Prior to the loop computing each pixel in the triangle, get the render settings
	const bool bRenderDepth = m_settings.GetRenderFlag(ERenderFlag::Depth);

	const float Depth0 = Math::GetDepth(S0, S0, S1, S2, Area);
	const float Depth1 = Math::GetDepth(S1, S0, S1, S2, Area);
	const float Depth2 = Math::GetDepth(S2, S0, S1, S2, Area);

	// Loop through all pixels in the screen bounding box.
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
			m_currentShader->UVW = UVW;

			if (bRenderDepth)
			{
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
			}

			// Compute world position
			FVector3 UVWX(UVW.X);
			FVector3 UVWY(UVW.Y);
			FVector3 UVWZ(UVW.Z);

			m_currentShader->PixelWorldPosition = m_currentShader->V0.Position * UVW.X + m_currentShader->V1.Position *
				UVW.Y
				+ m_currentShader->V2.Position * UVW.Z;

			// Compute the final color for this pixel
			m_currentShader->ComputePixelShader(Point.X, Point.Y);

			// Set the current pixel in memory to the computed color
			*ColorPixel = m_currentShader->OutColor;
		}
		DepthMemory += Width;
		ColorMemory += Width;
	}
}
