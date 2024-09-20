#include "Scanline.h"

#include "Math/Clipping.h"

void ScanlineRenderPipeline::startup()
{
	m_currentShader = std::make_shared<DefaultShader>();
	m_colorTexture  = std::make_shared<Texture>();
	m_depthTexture  = std::make_shared<Texture>();
	m_threadCount   = (int32)std::thread::hardware_concurrency();
}

void ScanlineRenderPipeline::preDraw()
{
	// Reset all buffers to their default values (namely z to Inf)
	m_colorTexture->fill(Color::black());
	m_depthTexture->fill(10000.0f);

	if (TextureManager::count() > 0)
	{
		m_currentShader->texture = TextureManager::getTexture(0);
	}
}

void ScanlineRenderPipeline::draw()
{
	for (const auto& mesh : g_meshes)
	{
		m_currentShader->hasNormals   = mesh->hasNormals();
		m_currentShader->hasTexCoords = mesh->hasTexCoords();

		for (const auto& tri : *mesh->getTriangles())
		{
			drawTriangle(tri.v0, tri.v1, tri.v2);
		}
	}
}

void ScanlineRenderPipeline::postDraw() {}

void ScanlineRenderPipeline::drawGrid(Grid* grid)
{
	if (grid == nullptr)
	{
		return;
	}

	for (const line3d& line : grid->getLines())
	{
		// Project the world-space points to screen-space
		vec3f s0, s1;
		bool lineOnScreen = false;
		lineOnScreen |= Math::projectWorldToScreen(line.a, s0, *m_viewData);
		lineOnScreen |= Math::projectWorldToScreen(line.b, s1, *m_viewData);

		// If neither of the points are on the screen, return
		if (!lineOnScreen)
		{
			return;
		}

		drawLine(s0, s1, Color::gray());
	}
}

void ScanlineRenderPipeline::drawTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
{
	// Update the current shader to use the camera's most recent parameters
	m_currentShader->init(*m_viewData);

	// Compute the vertex shader given the input vertices.
	m_currentShader->preComputeVertexShader();
	if (!m_currentShader->computeVertexShader(v0, v1, v2))
	{
		return;
	}

	// Render shaded
	if (m_renderSettings->getRenderFlag(Shaded))
	{
		// m_currentShader->viewMatrix = camera->m_viewMatrix;
		drawScanline();
	}

	// Render wireframe
	vec3f s0 = m_currentShader->s0;
	vec3f s1 = m_currentShader->s1;
	vec3f s2 = m_currentShader->s2;
	if (m_renderSettings->getRenderFlag(Wireframe))
	{
		Color wireColor = Color::red();
		drawLine({s0.x, s0.y}, {s1.x, s1.y}, wireColor);
		drawLine({s1.x, s1.y}, {s2.x, s2.y}, wireColor);
		drawLine({s2.x, s2.y}, {s0.x, s0.y}, wireColor);
	}

	// Render normal direction
	if (m_renderSettings->getRenderFlag(Normals))
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
		Math::projectWorldToScreen(triangleCenter, normalStartScreen, *m_viewData);
		Math::projectWorldToScreen(triangleCenter + triangleNormal, normalEndScreen,
		                           *m_viewData);

		// Draw the line between the two points
		auto normalColor = Color::yellow();
		drawLine(
			normalStartScreen, // Start
			normalEndScreen,   // End
			normalColor);      // Color
	}
}

void ScanlineRenderPipeline::drawScanline() const
{
	const vec3f s0 = m_currentShader->s0;
	const vec3f s1 = m_currentShader->s1;
	const vec3f s2 = m_currentShader->s2;

	// Compute the bounds of just this triangle on the screen
	const int32 width  = m_width;
	const int32 height = m_height;
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
	const bool renderDepth = m_renderSettings->getRenderFlag(Depth);

	const float depth0 = Math::getDepth(s0, s0, s1, s2, area);
	const float depth1 = Math::getDepth(s1, s0, s1, s2, area);
	const float depth2 = Math::getDepth(s2, s0, s1, s2, area);

	// Loop through all pixels in the screen bounding box.
	for (int32 y = minY; y <= maxY; y++)
	{
		for (int32 x = minX; x <= maxX; x++)
		{
			vec3f point((float)x, (float)y, 0);

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

			m_currentShader->pixelWorldPosition = m_currentShader->v0.position * bary.x + m_currentShader->v1.
				position * bary.y + m_currentShader->v2.position * bary.z;

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

void ScanlineRenderPipeline::drawLine(const vec3f& inA, const vec3f& inB, const Color& color)
{
	vec2i a((int32)inA.x, (int32)inA.y);
	vec2i b((int32)inB.x, (int32)inB.y);

	// Clip the screen points within the viewport. If the line points are outside the viewport entirely
	// then just return.
	if (!clipLine(&a, &b, vec2i{m_width, m_height}))
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
		a       = vec2i(a.y, a.x);
		b       = vec2i(b.y, b.x);
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
			if (y < 0)
			{
				continue;
			}
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
			if (y < 0)
			{
				continue;
			}
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

void ScanlineRenderPipeline::resize(int32 width, int32 height)
{
	m_width  = width;
	m_height = height;
	m_colorTexture->resize({width, height});
	m_depthTexture->resize({width, height});
}

uint8* ScanlineRenderPipeline::getFrameData()
{
	return m_colorTexture->getMemory<uint8>();
}

void ScanlineRenderPipeline::setViewData(ViewData* newViewData)
{
	m_viewData = newViewData;
}

void ScanlineRenderPipeline::setRenderSettings(RenderSettings* newRenderSettings)
{
	m_renderSettings = newRenderSettings;
}
