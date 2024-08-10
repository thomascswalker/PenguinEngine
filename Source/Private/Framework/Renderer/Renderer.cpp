// ReSharper disable CppClangTidyBugproneNarrowingConversions
// ReSharper disable CppClangTidyClangDiagnosticFloatConversion
// ReSharper disable CppClangTidyClangDiagnosticImplicitIntFloatConversion

#include "Framework/Renderer/Renderer.h"
#include "Framework/Engine/Engine.h"
#include "Framework/Renderer/Shader.h"

/* Renderer */

Renderer::Renderer(uint32 inWidth, uint32 inHeight)
{
	m_viewport = std::make_shared<Viewport>(inWidth, inHeight);
	m_grid = std::make_unique<FGrid>(8, 4.0f);

	addChannel(EChannelType::Data, "Depth");
	addChannel(EChannelType::Color, "Color");

	// Set default render flags
	m_settings = RenderSettings();

	// Default shader
	m_currentShader = std::make_shared<DefaultShader>();
}

void Renderer::resize(const uint32 inWidth, const uint32 inHeight) const
{
	m_viewport->resize(inWidth, inHeight);
	for (const auto& channel : m_channels | std::views::values)
	{
		channel->resize(inWidth, inHeight);
	}
}

bool Renderer::clipLine(vec2f* a, vec2f* b) const
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

bool Renderer::clipLine(linef* line) const
{
	return clipLine(&line->m_a, &line->m_b);
}

void Renderer::drawLine(const vec3f& inA, const vec3f& inB, const Color& color) const
{
	vec2f a(inA.X, inA.Y);
	vec2f b(inB.X, inB.Y);

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
		a = vec2f(a.Y, a.X);
		b = vec2f(b.Y, b.X);
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

void Renderer::drawLine(const line3d& line, const Color& color) const
{
	drawLine(line.m_a, line.m_b, color);
}

void Renderer::drawTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
{
	const Camera* camera = m_viewport->getCamera();
	m_currentShader->init(camera->getViewData());

	if (!m_currentShader->computeVertexShader(v0, v1, v2))
	{
		return;
	}

	if (m_settings.getRenderFlag(ERenderFlag::Shaded))
	{
		m_currentShader->m_viewMatrix = camera->m_viewMatrix;
		scanline();
	}

	vec3f s0 = m_currentShader->m_s0;
	vec3f s1 = m_currentShader->m_s1;
	vec3f s2 = m_currentShader->m_s2;
	if (m_settings.getRenderFlag(ERenderFlag::Wireframe))
	{
		drawLine({s0.X, s0.Y}, {s1.X, s1.Y}, m_wireColor);
		drawLine({s1.X, s1.Y}, {s2.X, s2.Y}, m_wireColor);
		drawLine({s2.X, s2.Y}, {s0.X, s0.Y}, m_wireColor);
	}

	// Draw normal direction
	if (m_settings.getRenderFlag(ERenderFlag::Normals))
	{
		// Get the center of the triangle
		vec3f triangleCenter = (m_currentShader->m_v0.m_position + m_currentShader->m_v1.m_position + m_currentShader
				->m_v2
				.
				m_position)
			/ 3.0f;

		// Get the computed triangle normal (average of the three normals)
		vec3f triangleNormal = m_currentShader->m_triangleWorldNormal;

		vec3f normalStartScreen;
		vec3f normalEndScreen;

		// Compute two screen-space points:
		// 1. The center of the triangle
		// 2. 1 unit out from the center of the triangle, in the direction the triangle is facing
		Math::projectWorldToScreen(triangleCenter, normalStartScreen, m_viewport->getCamera()->getViewData());
		Math::projectWorldToScreen(triangleCenter + triangleNormal, normalEndScreen,
		                           m_viewport->getCamera()->getViewData());

		// Draw the line between the two points
		drawLine(
			normalStartScreen, // Start
			normalEndScreen, // End
			Color::yellow()
		);
	}
}

// TODO: Rewrite to use a single array of vertices rather than looping through meshes/triangles
void Renderer::drawMesh(const Mesh* mesh)
{
	for (const auto& triangle : mesh->m_triangles)
	{
		drawTriangle(triangle.m_v0, triangle.m_v1, triangle.m_v2);
	}
}

void Renderer::drawGrid() const
{
	for (const line3d& line : m_grid->m_lines)
	{
		// Project the world-space points to screen-space
		vec3f s0, s1;
		bool lineOnScreen = false;
		lineOnScreen |= m_viewport->projectWorldToScreen(line.m_a, s0);
		lineOnScreen |= m_viewport->projectWorldToScreen(line.m_b, s1);

		// If neither of the points are on the screen, return
		if (!lineOnScreen)
		{
			return;
		}

		drawLine(s0, s1, m_gridColor);
	}
}

void Renderer::draw()
{
	// Recalculate the view-projection matrix of the camera
	m_viewport->getCamera()->computeViewProjectionMatrix();

	// Reset all buffers to their default values (namely Z to Inf)
	clearChannels();

	// Draw the world grid prior to drawing any geometry
	drawGrid();

	// Draw each mesh
	const Engine* engine = Engine::getInstance();
	for (const auto& mesh : engine->getMeshes())
	{
		drawMesh(mesh.get());
	}

	// Draw mouse cursor line from click origin
	const IInputHandler* inputHandler = IInputHandler::getInstance();
	if (inputHandler->isMouseDown(EMouseButtonType::Left) && inputHandler->isAltDown())
	{
		const vec3f a = inputHandler->getClickPosition();
		if (a.X != 0.0f && a.Y != 0.0f)
		{
			const vec3f b = inputHandler->getCurrentCursorPosition();
			drawLine(a, b, Color::red());
		}
	}
}

void Renderer::scanline() const
{
	const vec3f s0 = m_currentShader->m_s0;
	const vec3f s1 = m_currentShader->m_s1;
	const vec3f s2 = m_currentShader->m_s2;

	// Compute the bounds of just this triangle on the screen
	const int32 width = getWidth();
	const int32 height = getHeight();
	const rectf bounds = m_currentShader->m_screenBounds;
	const int32 minX = Math::Max(static_cast<int32>(bounds.Min().X), 0);
	const int32 maxX = Math::Min(static_cast<int32>(bounds.Max().X), width - 1);
	const int32 minY = Math::Max(static_cast<int32>(bounds.Min().Y), 0);
	const int32 maxY = Math::Min(static_cast<int32>(bounds.Max().Y), height - 1);

	// Precompute the area of the screen triangle so we're not computing it every pixel
	const float area = Math::Area2D(s0, s1, s2) * 2.0f;
	const float oneOverArea = 1.0f / area;

	const int32 initialOffset = minY * width;

	std::shared_ptr<PChannel> depthChannel = getDepthChannel();
	float* depthMemory = static_cast<float*>(depthChannel->m_memory) + initialOffset; // float, 32-bytes

	std::shared_ptr<PChannel> colorChannel = getColorChannel();
	int32* colorMemory = static_cast<int32*>(colorChannel->m_memory) + initialOffset; // int32, 32-bytes

	// Prior to the loop computing each pixel in the triangle, get the render settings
	const bool bRenderDepth = m_settings.getRenderFlag(ERenderFlag::Depth);

	const float depth0 = Math::GetDepth(s0, s0, s1, s2, area);
	const float depth1 = Math::GetDepth(s1, s0, s1, s2, area);
	const float depth2 = Math::GetDepth(s2, s0, s1, s2, area);

	// Loop through all pixels in the screen bounding box.
	for (int32 y = minY; y <= maxY; y++)
	{
		for (int32 x = minX; x <= maxX; x++)
		{
			vec3f point(x, y, 0);

			// Use Pineda's edge function to determine if the current pixel is within the triangle.
			float w0 = Math::EdgeFunction(s1.X, s1.Y, s2.X, s2.Y, point.X, point.Y);
			float w1 = Math::EdgeFunction(s2.X, s2.Y, s0.X, s0.Y, point.X, point.Y);
			float w2 = Math::EdgeFunction(s0.X, s0.Y, s1.X, s1.Y, point.X, point.Y);

			if (w0 <= 0.0f || w1 <= 0.0f || w2 <= 0.0f)
			{
				continue;
			}

			vec3f uvw;
			float* depthPixel = depthMemory + x;
			int32* colorPixel = colorMemory + x;

			// From the edge vectors, extrapolate the barycentric coordinates for this pixel.
			w0 *= oneOverArea;
			w1 *= oneOverArea;
			w2 *= oneOverArea;

			uvw.X = w0;
			uvw.Y = w1;
			uvw.Z = w2;
			m_currentShader->m_uvw = uvw;

			if (bRenderDepth)
			{
				// Interpolate depth given UVW
				float newDepth = uvw.X * depth0 + uvw.Y * depth1 + uvw.Z * depth2;

				// Compare the new depth to the current depth at this pixel. If the new depth is further than
				// the current depth, continue.
				float currentDepth = *depthPixel;
				if (newDepth > currentDepth)
				{
					continue;
				}
				// If the new depth is closer than the current depth, set the current depth
				// at this pixel to the new depth we just got.
				*depthPixel = newDepth;
			}

			m_currentShader->m_pixelWorldPosition = m_currentShader->m_v0.m_position * uvw.X + m_currentShader->m_v1.
				m_position *
				uvw.Y
				+ m_currentShader->m_v2.m_position * uvw.Z;

			// Compute the final color for this pixel
			m_currentShader->computePixelShader(point.X, point.Y);

			// Set the current pixel in memory to the computed color
			*colorPixel = m_currentShader->m_outColor;
		}
		depthMemory += width;
		colorMemory += width;
	}
}
