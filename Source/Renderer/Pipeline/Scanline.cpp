#include "Scanline.h"

#include "Math/Clipping.h"

bool ScanlineRenderPipeline::init(void* windowHandle)
{
	int32 width  = g_defaultViewportWidth;
	int32 height = g_defaultViewportHeight;

	m_frameBuffer = std::make_shared<Texture>(vec2i{width, height});
	m_depthBuffer = std::make_shared<Texture>(vec2i{width, height});

	m_vertexShader = std::make_shared<ScanlineVertexShader>();
	m_pixelShader  = std::make_shared<ScanlinePixelShader>();

	m_viewData         = std::make_shared<ViewData>();
	m_shaderData       = std::make_shared<ScanlineShaderData>();
	m_viewData->width  = width;
	m_viewData->height = height;

	return true;
}

void ScanlineRenderPipeline::beginDraw()
{
	// Reset all buffers to their default values (namely z to Inf)
	m_frameBuffer->fill(Color::black());
	m_depthBuffer->fill(10000.0f);

	if (TextureManager::count() > 0)
	{
		m_shaderData->texture = TextureManager::getTexture(0);
	}
}

void ScanlineRenderPipeline::draw()
{
	for (const auto& mesh : g_meshes)
	{
		m_shaderData->hasNormals   = mesh->hasNormals();
		m_shaderData->hasTexCoords = mesh->hasTexCoords();

		for (const auto& tri : *mesh->getTriangles())
		{
			drawTriangle(tri.v0, tri.v1, tri.v2);
		}
	}
}

void ScanlineRenderPipeline::endDraw() {}

void ScanlineRenderPipeline::drawGrid(Grid* grid)
{
	if (grid == nullptr)
	{
		return;
	}

	Color color = m_renderSettings->getGridColor();
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

		drawLine(s0, s1, color);
	}
}

void ScanlineRenderPipeline::drawTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
{
	// Compute the vertex shader given the input vertices.
	m_shaderData->v0 = v0;
	m_shaderData->v1 = v1;
	m_shaderData->v2 = v2;

	if (!m_vertexShader->process(m_viewData.get(), m_shaderData.get()))
	{
		return;
	}

	// Render shaded
	if (m_renderSettings->getRenderFlag(Shaded))
	{
		drawScanline();
	}

	// Render wireframe
	vec3f s0 = m_shaderData->s0;
	vec3f s1 = m_shaderData->s1;
	vec3f s2 = m_shaderData->s2;
	if (m_renderSettings->getRenderFlag(Wireframe))
	{
		Color wireColor = m_renderSettings->getWireColor();
		drawLine({s0.x, s0.y}, {s1.x, s1.y}, wireColor);
		drawLine({s1.x, s1.y}, {s2.x, s2.y}, wireColor);
		drawLine({s2.x, s2.y}, {s0.x, s0.y}, wireColor);
	}

	// Render normal direction
	if (m_renderSettings->getRenderFlag(Normals))
	{
		// Get the center of the triangle
		vec3f triangleCenter = (m_shaderData->v0.position
				+ m_shaderData->v1.position
				+ m_shaderData->v2.position)
			/ 3.0f;

		// Get the computed triangle normal (average of the three normals)
		vec3f triangleNormal = m_shaderData->triangleWorldNormal;

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
	const vec3f s0 = m_shaderData->s0;
	const vec3f s1 = m_shaderData->s1;
	const vec3f s2 = m_shaderData->s2;

	// Compute the bounds of just this triangle on the screen
	const int32 width  = m_viewData->width;
	const int32 height = m_viewData->height;
	const rectf bounds = m_shaderData->screenBounds;

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
			float w0 = EDGE_FUNCTION(s1.x, s1.y, s2.x, s2.y, point.x, point.y);
			float w1 = EDGE_FUNCTION(s2.x, s2.y, s0.x, s0.y, point.x, point.y);
			float w2 = EDGE_FUNCTION(s0.x, s0.y, s1.x, s1.y, point.x, point.y);

			if (w0 <= 0.0f || w1 <= 0.0f || w2 <= 0.0f)
			{
				continue;
			}

			// From the edge vectors, extrapolate the barycentric coordinates for this pixel.
			w0 *= oneOverArea;
			w1 *= oneOverArea;
			w2 *= oneOverArea;

			vec3f bary;
			bary.x             = w0;
			bary.y             = w1;
			bary.z             = w2;
			m_shaderData->bary = bary;

			if (renderDepth)
			{
				// Interpolate depth given the barycentric coordinates
				float z = bary.x * depth0 + bary.y * depth1 + bary.z * depth2;

				// Compare the new depth to the current depth at this pixel. If the new depth is further than
				// the current depth, continue.
				float oldZ = m_depthBuffer->getPixelAsFloat(x, y);
				if (z > oldZ)
				{
					continue;
				}
				// If the new depth is closer than the current depth, set the current depth
				// at this pixel to the new depth we just got.
				m_depthBuffer->setPixelFromFloat(x, y, z);
			}

			m_shaderData->pixelWorldPosition = m_shaderData->v0.position * bary.x + m_shaderData->v1.position * bary.y +
				m_shaderData->v2.position * bary.z;

			// Compute the UV coordinates of the current pixel
			if (m_shaderData->hasTexCoords)
			{
				// Use the barycentric coordinates to interpolate between all three vertex UV coordinates
				m_shaderData->uv = m_shaderData->v0.texCoord * bary.x + m_shaderData->v1.texCoord * bary.y +
					m_shaderData->v2.texCoord * bary.z;
			}

			// Compute the final color for this pixel
			m_shaderData->x = (int32)point.x;
			m_shaderData->y = (int32)point.y;
			m_pixelShader->process(m_viewData.get(), m_shaderData.get());

			// Set the current pixel in memory to the computed color
			m_frameBuffer->setPixelFromColor(x, y, m_shaderData->outColor);
		}
	}
}

void ScanlineRenderPipeline::drawLine(const vec3f& inA, const vec3f& inB, const Color& color)
{
	vec2i a((int32)inA.x, (int32)inA.y);
	vec2i b((int32)inB.x, (int32)inB.y);

	// Clip the screen points within the viewport. If the line points are outside the viewport entirely
	// then just return.
	if (!clipLine(&a, &b, vec2i{m_viewData->width, m_viewData->height}))
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
			m_frameBuffer->setPixelFromColor(y, x, color);
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
			m_frameBuffer->setPixelFromColor(x, y, color);
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
	if (width != m_viewData->width || height != m_viewData->height)
	{
		LOG_ERROR("Size mismatch with ScanlineRenderPipeline::resize() and m_viewData. Skipping resize.")
		return;
	}

	m_frameBuffer->resize({width, height});
	m_depthBuffer->resize({width, height});
}

uint8* ScanlineRenderPipeline::getFrameData()
{
	return m_frameBuffer->getMemory<uint8>();
}

void ScanlineRenderPipeline::setViewData(ViewData* newViewData)
{
	m_viewData = std::make_shared<ViewData>(*newViewData);
}

void ScanlineRenderPipeline::setRenderSettings(RenderSettings* newRenderSettings)
{
	m_renderSettings = std::make_shared<RenderSettings>(*newRenderSettings);
}

void ScanlineRenderPipeline::setVertexData(float* data, size_t size, int32 count) {}
