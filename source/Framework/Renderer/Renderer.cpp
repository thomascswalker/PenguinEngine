﻿// ReSharper disable CppClangTidyBugproneNarrowingConversions
// ReSharper disable CppClangTidyClangDiagnosticFloatConversion
// ReSharper disable CppClangTidyClangDiagnosticImplicitIntFloatConversion

#include <thread>

#include "Framework/Renderer/Renderer.h"
#include "Framework/Engine/Engine.h"
#include "Framework/Renderer/Shader.h"
#include "Framework/Renderer/Texture.h"
#include "Framework/Importers/TextureImporter.h"

Renderer::Renderer(uint32 inWidth, uint32 inHeight)
{
	m_viewport = std::make_shared<Viewport>(inWidth, inHeight);
	m_grid     = std::make_unique<Grid>(8, 4.0f);

	// Color and depth buffer storage
	m_colorTexture = std::make_shared<Texture>(vec2i(inWidth, inHeight));
	m_depthTexture = std::make_shared<Texture>(vec2i(inWidth, inHeight));

	// Set default render flags
	m_settings = RenderSettings();

	// Default shader
	m_currentShader = std::make_shared<DefaultShader>();
	m_threadCount   = std::thread::hardware_concurrency();
}

void Renderer::resize(const uint32 inWidth, const uint32 inHeight) const
{
	m_viewport->resize(inWidth, inHeight);
	m_colorTexture->resize(vec2i(inWidth, inHeight));
	m_depthTexture->resize(vec2i(inWidth, inHeight));
}

void Renderer::createTiles()
{
	int32 id         = 0;
	int32 tileCountX = getWidth() / g_defaultTileSize;
	int32 tileCountY = getHeight() / g_defaultTileSize;

	for (int32 x = 0; x < tileCountX; x++)
	{
		for (int32 y = 0; y < tileCountY; y++)
		{
			m_tiles.append(Tile(vec2f(g_defaultTileSize * x, g_defaultTileSize * y), // Min
			                    vec2f(g_defaultTileSize * x + g_defaultTileSize,     // MaxX
			                          g_defaultTileSize * y + g_defaultTileSize),    // MaxY
			                    id));                                                // Id
			id++;
		}
	}
}

void Renderer::draw() const
{
	// Recalculate the view-projection matrix of the camera
	m_viewport->getCamera()->computeViewProjectionMatrix();

	// Reset all buffers to their default values (namely z to Inf)
	m_colorTexture->fill(Color::black());
	m_depthTexture->fill(10000.0f);

	// Draw the world grid prior to drawing any geometry
	drawGrid();

	// Draw each mesh
	const Engine* engine = Engine::getInstance();
	for (const auto& mesh : g_meshes)
	{
		m_currentShader->hasNormals   = mesh->hasNormals();
		m_currentShader->hasTexCoords = mesh->hasTexCoords();
		drawMesh(mesh.get());
	}
}

void Renderer::drawMesh(Mesh* mesh) const
{
	for (const auto& triangle : *mesh->getTriangles())
	{
		drawTriangle(triangle.v0, triangle.v1, triangle.v2);
	}
}

void Renderer::drawTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2) const
{
	// Update the current shader to use the camera's most recent parameters
	const Camera* camera = m_viewport->getCamera();
	m_currentShader->init(camera->getViewData());

	// Compute the vertex shader given the input vertices.
	m_currentShader->preComputeVertexShader();
	if (!m_currentShader->computeVertexShader(v0, v1, v2))
	{
		return;
	}

	// Render shaded
	if (m_settings.getRenderFlag(Shaded))
	{
		// m_currentShader->viewMatrix = camera->m_viewMatrix;
		scanline();
	}

	// Render wireframe
	vec3f s0 = m_currentShader->s0;
	vec3f s1 = m_currentShader->s1;
	vec3f s2 = m_currentShader->s2;
	if (m_settings.getRenderFlag(Wireframe))
	{
		drawLine({s0.x, s0.y}, {s1.x, s1.y}, m_wireColor);
		drawLine({s1.x, s1.y}, {s2.x, s2.y}, m_wireColor);
		drawLine({s2.x, s2.y}, {s0.x, s0.y}, m_wireColor);
	}

	// Render normal direction
	if (m_settings.getRenderFlag(Normals))
	{
		// Get the center of the triangle
		vec3f triangleCenter = (m_currentShader->v0.position
				+ m_currentShader->v1.position
				+ m_currentShader->v2.position)
			/ 3.0f;

		// Get the computed triangle normal (average of the three normals)
		vec3f triangleNormal = m_currentShader->triangleWorldNormal;

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
			normalEndScreen,   // End
			Color::yellow());  // Color
	}
}

void Renderer::scanline() const
{
	const vec3f s0 = m_currentShader->s0;
	const vec3f s1 = m_currentShader->s1;
	const vec3f s2 = m_currentShader->s2;

	// Compute the bounds of just this triangle on the screen
	const int32 width  = getWidth();
	const int32 height = getHeight();
	const rectf bounds = m_currentShader->screenBounds;

	const vec2f boundsMin = bounds.min();
	const vec2f boundsMax = bounds.max();
	const int32 minX      = std::max(static_cast<int32>(boundsMin.x), 0);
	const int32 maxX      = std::min(static_cast<int32>(boundsMax.x), width - 1);
	const int32 minY      = std::max(static_cast<int32>(boundsMin.y), 0);
	const int32 maxY      = std::min(static_cast<int32>(boundsMax.y), height - 1);

	// Pre-compute the area of the screen triangle so we're not computing it every pixel
	const float area        = Math::area2D(s0, s1, s2) * 2.0f;
	const float oneOverArea = 1.0f / area;

	// Prior to the loop computing each pixel in the triangle, get the render settings
	const bool renderDepth = m_settings.getRenderFlag(Depth);

	const float depth0 = Math::getDepth(s0, s0, s1, s2, area);
	const float depth1 = Math::getDepth(s1, s0, s1, s2, area);
	const float depth2 = Math::getDepth(s2, s0, s1, s2, area);

	// Loop through all pixels in the screen bounding box.
	for (int32 y = minY; y <= maxY; y++)
	{
		for (int32 x = minX; x <= maxX; x++)
		{
			vec3f point(x, y, 0);

			// Use Pineda's edge function to determine if the current pixel is within the triangle.
			float w0 = Math::edgeFunction(s1.x, s1.y, s2.x, s2.y, point.x, point.y);
			float w1 = Math::edgeFunction(s2.x, s2.y, s0.x, s0.y, point.x, point.y);
			float w2 = Math::edgeFunction(s0.x, s0.y, s1.x, s1.y, point.x, point.y);

			if (w0 <= 0.0f || w1 <= 0.0f || w2 <= 0.0f)
			{
				continue;
			}

			// From the edge vectors, extrapolate the barycentric coordinates for this pixel.
			w0 *= oneOverArea;
			w1 *= oneOverArea;
			w2 *= oneOverArea;

			vec3f bary;
			bary.x                = w0;
			bary.y                = w1;
			bary.z                = w2;
			m_currentShader->bary = bary;

			if (renderDepth)
			{
				// Interpolate depth given the barycentric coordinates
				float z = bary.x * depth0 + bary.y * depth1 + bary.z * depth2;

				// Compare the new depth to the current depth at this pixel. If the new depth is further than
				// the current depth, continue.
				float oldZ = m_depthTexture->getPixelAsFloat(x, y);
				if (z > oldZ)
				{
					continue;
				}
				// If the new depth is closer than the current depth, set the current depth
				// at this pixel to the new depth we just got.
				m_depthTexture->setPixelFromFloat(x, y, z);
			}

			m_currentShader->pixelWorldPosition = m_currentShader->v0.position * bary.x + m_currentShader->v1.position *
				bary.y + m_currentShader->v2.position * bary.z;

			// Compute the UV coordinates of the current pixel
			m_currentShader->computeUv();

			// Compute the final color for this pixel
			m_currentShader->preComputePixelShader();
			m_currentShader->computePixelShader(point.x, point.y);

			// Set the current pixel in memory to the computed color
			m_colorTexture->setPixelFromColor(x, y, m_currentShader->outColor);
		}
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
	int32 code1 = (a->x < minX) << 3 | (a->x > maxX) << 2 | (a->y < minY) << 1 | (a->y > maxY);
	int32 code2 = (b->x < minX) << 3 | (b->x > maxX) << 2 | (b->y < minY) << 1 | (b->y > maxY);

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
			x = a->x + (b->x - a->x) * (maxY - a->y) / (b->y - a->y);
			y = maxY;
		}
		else if (code & 2)
		{
			// Bottom edge
			x = a->x + (b->x - a->x) * (minY - a->y) / (b->y - a->y);
			y = minY;
		}
		else if (code & 4)
		{
			// Right edge
			y = a->y + (b->y - a->y) * (maxX - a->x) / (b->x - a->x);
			x = maxX;
		}
		else
		{
			// Left edge
			y = a->y + (b->y - a->y) * (minX - a->x) / (b->x - a->x);
			x = minX;
		}

		// Update the endpoint
		if (code == code1)
		{
			a->x  = x;
			a->y  = y;
			code1 = (a->x < minX) << 3 | (a->x > maxX) << 2 | (a->y < minY) << 1 | (a->y > maxY);
		}
		else
		{
			b->x  = x;
			b->y  = y;
			code2 = (b->x < minX) << 3 | (b->x > maxX) << 2 | (b->y < minY) << 1 | (b->y > maxY);
		}
	}

	return true;
}

bool Renderer::clipLine(linef* line) const
{
	return clipLine(&line->a, &line->b);
}

void Renderer::drawLine(const vec3f& inA, const vec3f& inB, const Color& color) const
{
	vec2f a(inA.x, inA.y);
	vec2f b(inB.x, inB.y);

	// Clip the screen points within the viewport. If the line points are outside the viewport entirely
	// then just return.
	if (!clipLine(&a, &b))
	{
		return;
	}
	/* TODO: This is a temporary fix to prevent m_lines getting clipped and setting one of the points to [0,0]. */
	if (a == 0.0f || b == 0.0f)
	{
		return;
	}

	bool isSteep = false;
	if (std::abs(a.x - b.x) < std::abs(a.y - b.y))
	{
		a       = vec2f(a.y, a.x);
		b       = vec2f(b.y, b.x);
		isSteep = true;
	}

	if (a.x > b.x)
	{
		std::swap(a, b);
	}

	const int32 deltaX     = b.x - a.x;
	const int32 deltaY     = b.y - a.y;
	const int32 deltaError = std::abs(deltaY) * 2;
	int32 errorCount       = 0;

	// https://github.com/ssloy/tinyrenderer/issues/28
	int32 y = a.y;

	if (isSteep)
	{
		for (int32 x = a.x; x < b.x; ++x)
		{
			m_colorTexture->setPixelFromColor(y, x, color);
			errorCount += deltaError;
			if (errorCount > deltaX)
			{
				y += (b.y > a.y ? 1 : -1);
				errorCount -= deltaX * 2;
			}
		}
	}
	else
	{
		for (int32 x = a.x; x < b.x; ++x)
		{
			m_colorTexture->setPixelFromColor(x, y, color);
			errorCount += deltaError;
			if (errorCount > deltaX)
			{
				y += (b.y > a.y ? 1 : -1);
				errorCount -= deltaX * 2;
			}
		}
	}
}

void Renderer::drawLine(const line3d& line, const Color& color) const
{
	drawLine(line.a, line.b, color);
}

void Renderer::drawGrid() const
{
	for (const line3d& line : m_grid->getLines())
	{
		// Project the world-space points to screen-space
		vec3f s0, s1;
		bool lineOnScreen = false;
		lineOnScreen |= m_viewport->projectWorldToScreen(line.a, s0);
		lineOnScreen |= m_viewport->projectWorldToScreen(line.b, s1);

		// If neither of the points are on the screen, return
		if (!lineOnScreen)
		{
			return;
		}

		drawLine(s0, s1, m_gridColor);
	}
}
