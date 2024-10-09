#include "Scanline.h"

#include "Math/Clipping.h"

/** Vertex Shader **/

VertexOutput ScanlineVertexShader::process(const VertexInput& input)
{
	VertexOutput out;
	// Project world position to screen
	out.position = Math::vectorTransform(vec4f(input.position, 1.0f), input.mvp);
	// Transform object normal to world normal
	out.normal = Math::vectorTransform(input.normal, input.model);
	return out;
}

/** Pixel Shader **/

Color ScanlinePixelShader::process(const PixelData& input)
{
	Color out = Color::white();
	if (input.texture)
	{
		// Compute the relative UV coordinates on the texture
		int32 x = input.uv.x * (input.texture->getWidth() - 1);
		int32 y = input.uv.y * (input.texture->getHeight() - 1);

		// Set the outColor to the pixel at [x,y] in the texture
		out = input.texture->getPixelAsColor(x, y);
	}
	float facingRatio = (-input.cameraNormal).dot(input.worldNormal);
	facingRatio       = std::clamp(facingRatio, 0.0f, 1.0f);
	out *= facingRatio;

	return out;
}

/** Pipeline **/

bool ScanlineRenderPipeline::init(void* windowHandle)
{
	int32 width  = g_defaultViewportWidth;
	int32 height = g_defaultViewportHeight;

	m_frameBuffer = std::make_shared<Texture>(vec2i{width, height});
	m_depthBuffer = std::make_shared<Texture>(vec2i{width, height});

	m_vertexShader = std::make_shared<ScanlineVertexShader>();
	m_pixelShader  = std::make_shared<ScanlinePixelShader>();

	m_viewData         = std::make_shared<ViewData>();
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
		m_texturePtr = TextureManager::getTexture(0);
	}
}

void ScanlineRenderPipeline::draw()
{
	for (const MeshDescription& desc : m_meshDescriptions)
	{
		auto offset                           = desc.offset;
		auto size                             = desc.vertexCount;
		m_viewData->modelMatrix               = desc.transform->toMatrix();
		m_viewData->modelViewProjectionMatrix = m_viewData->modelMatrix * m_viewData->viewProjectionMatrix;

		// Draw each triangle in the vertex buffer
		for (int32 index = offset; index < offset + size; index += 3)
		{
			drawTriangle(m_vertexBuffer.data() + index);
		}
	}
}

void ScanlineRenderPipeline::bindMesh(IRenderable* renderable)
{
	// Convert the current renderable's geometry into a vertex buffer
	MeshDescription meshDesc{};

	meshDesc.offset = m_vertexBuffer.size();

	// Set the transform pointer
	Mesh* mesh         = renderable->getMesh();
	meshDesc.transform = renderable->getTransform();

	// Add each vertex to the vertex buffer and increment the vertex count by 3
	std::vector<Triangle>* triangles = mesh->getTriangles();
	for (Triangle& tri : *triangles)
	{
		m_vertexBuffer.emplace_back(tri.v0);
		m_vertexBuffer.emplace_back(tri.v1);
		m_vertexBuffer.emplace_back(tri.v2);
		meshDesc.vertexCount += 3;
	}

	// Add to mesh descriptions
	m_meshDescriptions.emplace_back(meshDesc);
}

void ScanlineRenderPipeline::endDraw() {}

bool ScanlineRenderPipeline::vertexStage()
{
	// Run the vertex shader for each vertex. This is assuming the output is a vec4f which is the
	// final projected vertex position on the screen. The W component of that vector needs to be
	// above 0 to be in the view frustum. Otherwise, it's off the screen.
	bool triangleOnScreen = false;

	VertexInput input;
	input.mvp   = m_viewData->modelViewProjectionMatrix;
	input.model = m_viewData->modelMatrix;
	for (int32 i = 0; i < 3; i++)
	{
		input.position = m_vertexBufferPtr[i].position;
		input.normal   = m_vertexBufferPtr[i].normal;

		auto output = ScanlineVertexShader::process(input);
		if (output.position.w > 0.0f)
		{
			m_screenPoints[i]  = Clipping::clip(output.position, m_viewData->width, m_viewData->height);
			m_screenNormals[i] = output.normal;
			triangleOnScreen   = true;
		}
	}

	// If none of the vertexes are on the screen, the triangle is off-screen and the vertex stage
	// has failed.
	if (!triangleOnScreen)
	{
		return false;
	}

	// Check back-facing
	auto normal     = (m_screenNormals[0] + m_screenNormals[1] + m_screenNormals[2]) / 3.0f;
	float dotNormal = (-m_viewData->cameraDirection).dot(normal);
	if (dotNormal > 0.0f)
	{
		return false;
	}

	// Check the order of the vertexes on the screen. If they are
	EWindingOrder order = Math::getVertexOrder(m_screenPoints[0], m_screenPoints[1], m_screenPoints[2]);
	switch (order)
	{
	case EWindingOrder::Clockwise: // Triangle is back-facing, exit
	case EWindingOrder::CoLinear:  // Triangle has zero area, exit
		return false;
	case EWindingOrder::CounterClockwise: // Triangle is front-facing, continue
		break;
	}

	return true;
}

void ScanlineRenderPipeline::rasterStage()
{
	// Clear pixel buffer prior to rasterization
	m_pixelBuffer.clear();

	Vertex v0 = m_vertexBufferPtr[0];
	Vertex v1 = m_vertexBufferPtr[1];
	Vertex v2 = m_vertexBufferPtr[2];

	vec3f s0 = m_screenPoints[0];
	vec3f s1 = m_screenPoints[1];
	vec3f s2 = m_screenPoints[2];

	// Compute the bounds of just this triangle on the screen
	int32 width  = m_viewData->width;
	int32 height = m_viewData->height;

	rectf bounds = rectf::makeBoundingBox(s0, s1, s2);

	vec2f boundsMin = bounds.min();
	vec2f boundsMax = bounds.max();
	int32 minX      = std::max(static_cast<int32>(boundsMin.x), 0);
	int32 maxX      = std::min(static_cast<int32>(boundsMax.x), width - 1);
	int32 minY      = std::max(static_cast<int32>(boundsMin.y), 0);
	int32 maxY      = std::min(static_cast<int32>(boundsMax.y), height - 1);

	// Pre-compute the area of the screen triangle so we're not computing it every pixel
	float area        = Math::area2D(s0, s1, s2) * 2.0f;
	float oneOverArea = 1.0f / area;

	// Loop through all pixels in the screen bounding box.
	for (int32 y = minY; y <= maxY; y++)
	{
		for (int32 x = minX; x <= maxX; x++)
		{
			vec3f point((float)x, (float)y, 0);

			// Use Pineda's edge function to determine if the current pixel is within the triangle.
			float w0 = Math::edgeFunction(s1, s2, point);
			float w1 = Math::edgeFunction(s2, s0, point);
			float w2 = Math::edgeFunction(s0, s1, point);

			if (w0 > 0.0f || w1 > 0.0f || w2 > 0.0f)
			{
				continue;
			}

			// From the edge vectors, extrapolate the barycentric coordinates for this pixel.
			w0 *= oneOverArea;
			w1 *= oneOverArea;
			w2 *= oneOverArea;

			vec3f bary;
			bary.x = w0;
			bary.y = w1;
			bary.z = w2;

			float z = 0;
			if (m_renderSettings->getRenderFlag(Depth))
			{
				// Interpolate depth given the barycentric coordinates
				z = bary.x * -s0.z + bary.y * -s1.z + bary.z * -s2.z;

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

			PixelData pixel;
			pixel.width    = m_viewData->width;
			pixel.height   = m_viewData->height;
			pixel.depth    = z;
			pixel.distance = bary;

			// Compute World Position of the current pixel
			pixel.position      = point; // local
			pixel.worldPosition = v0.position * bary.x + v1.position * bary.y + v2.position * bary.z;

			// Compute the UV coordinates of the current pixel
			pixel.uv = v0.texCoord * bary.x + v1.texCoord * bary.y + v2.texCoord * bary.z;

			// Compute the Normal direction of the current pixel
			pixel.worldNormal = m_screenNormals[0] * bary.x + m_screenNormals[1] * bary.y + m_screenNormals[2] * bary.z;
			pixel.cameraNormal = m_viewData->cameraDirection;

			// Set the texture
			pixel.texture = m_texturePtr;

			// Add to the fragment buffer
			m_pixelBuffer.emplace_back(pixel);
		}
	}
}

void ScanlineRenderPipeline::fragmentStage() const
{
	// Render each pixel
	for (const auto& pixel : m_pixelBuffer)
	{
		Color color = ScanlinePixelShader::process(pixel);
		m_frameBuffer->setPixelFromColor(pixel.position.x, pixel.position.y, color);
	}
}

void ScanlineRenderPipeline::drawTriangle(Vertex* vertex)
{
	// Set the vertex buffer pointer to the current vertex.
	m_vertexBufferPtr = vertex;

	// Run the vertex shader
	if (!vertexStage())
	{
		return;
	}

	if (m_renderSettings->getRenderFlag(Shaded))
	{
		// Rasterize the triangle
		rasterStage();

		// Run the pixel shader
		fragmentStage();
	}

	// Draw wireframe
	if (m_renderSettings->getRenderFlag(Wireframe))
	{
		drawWireframe();
	}

	if (m_renderSettings->getRenderFlag(Normals))
	{
		drawNormal();
	}
}

void ScanlineRenderPipeline::drawWireframe() const
{
	auto s0 = m_screenPoints[0];
	auto s1 = m_screenPoints[1];
	auto s2 = m_screenPoints[2];

	std::vector<vec2f> pixels{};
	computeLinePixels(s0, s1, pixels);
	computeLinePixels(s1, s2, pixels);
	computeLinePixels(s2, s0, pixels);
	for (const auto& p : pixels)
	{
		m_frameBuffer->setPixelFromColor(p.x, p.y, m_renderSettings->getWireColor());
	}
}

void ScanlineRenderPipeline::drawNormal()
{
	auto v0 = m_vertexBufferPtr[0];
	auto v1 = m_vertexBufferPtr[1];
	auto v2 = m_vertexBufferPtr[2];

	// Render normal direction
	if (m_renderSettings->getRenderFlag(Normals))
	{
		// Get the center of the triangle
		vec3f triangleCenter = (v0.position + v1.position + v2.position) / 3.0f;

		// Get the computed triangle normal (average of the three normals)
		vec3f triangleNormal = (v0.normal + v1.normal + v2.normal) / 3.0f;

		vec3f normalStartScreen;
		vec3f normalEndScreen;

		// Compute two screen-space points:
		// 1. The center of the triangle
		// 2. 1 unit out from the center of the triangle, in the direction the triangle is facing
		Math::projectWorldToScreen(triangleCenter, normalStartScreen, *m_viewData);
		Math::projectWorldToScreen(triangleCenter + triangleNormal, normalEndScreen, *m_viewData);

		// Draw the line between the two points
		auto normalColor = Color::yellow();
		drawLine(
			normalStartScreen, // Start
			normalEndScreen,   // End
			normalColor);      // Color
	}
}

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

void ScanlineRenderPipeline::drawLine(const vec3f& inA, const vec3f& inB, const Color& color)
{
	std::vector<vec2f> pixels;
	computeLinePixels(inA, inB, pixels);
	for (const auto& p : pixels)
	{
		m_frameBuffer->setPixelFromColor(p.x, p.y, color);
	}
}

void ScanlineRenderPipeline::computeLinePixels(const vec3f& inA, const vec3f& inB, std::vector<vec2f>& points) const
{
	vec2i a((int32)inA.x, (int32)inA.y);
	vec2i b((int32)inB.x, (int32)inB.y);

	// Clip the screen points within the viewport. If the line points are outside the viewport entirely
	// then just return.
	if (!Clipping::clipLine(&a, &b, vec2i{m_viewData->width, m_viewData->height}))
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
			points.push_back(vec2f(y, x));
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
			points.push_back(vec2f(x, y));
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
