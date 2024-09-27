#pragma once

#include <memory>

#include "RenderPipeline.h"

#include "Engine/Mesh.h"
#include "Renderer/Camera.h"
#include "Renderer/Grid.h"
#include "Renderer/Settings.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"

struct ScanlineShaderData
{
	ViewData viewData;
	Texture* texture = nullptr;
	Color baseColor  = Color::white();
	Color outColor   = Color::white();
	vec3f cameraWorldDirection;
	vec3f triangleWorldNormal;
	vec3f triangleCameraNormal;
	float facingRatio = 0.0f;
	int32 x;
	int32 y;
	Vertex v0;
	Vertex v1;
	Vertex v2;
	vec3f s0;
	vec3f s1;
	vec3f s2;
	vec2f uv;
	vec3f bary;
	rectf screenBounds;
	vec3f pixelWorldPosition;
	bool hasNormals   = false;
	bool hasTexCoords = false;

	ScanlineShaderData() = default;
};

struct FastsScanlineShaderData
{
	ViewData viewData;
	Texture* texture = nullptr;
	Color baseColor  = Color::white();
	Color outColor   = Color::white();
	vecm cameraWorldDirection;
	vecm triangleWorldNormal;
	vecm triangleCameraNormal;
	float facingRatio = 0.0f;
	int32 x;
	int32 y;
	Vertex v0;
	Vertex v1;
	Vertex v2;
	vecm s0;
	vecm s1;
	vecm s2;
	vecm uv;
	vecm bary;
	rectf screenBounds;
	vecm pixelWorldPosition;
	bool hasNormals   = false;
	bool hasTexCoords = false;

	FastsScanlineShaderData() = default;
};

class ScanlineVertexShader : public VertexShader
{
public:
	ScanlineVertexShader() = default;

	bool process(const ViewData* viewData, ScanlineShaderData* shaderData) const
	{
		// Project the world-space points to screen-space
		bool triangleOnScreen = false;
		triangleOnScreen |= Math::projectWorldToScreen(shaderData->v0.position, shaderData->s0, *viewData);
		triangleOnScreen |= Math::projectWorldToScreen(shaderData->v1.position, shaderData->s1, *viewData);
		triangleOnScreen |= Math::projectWorldToScreen(shaderData->v2.position, shaderData->s2, *viewData);

		// If the triangle is completely off screen, exit
		if (!triangleOnScreen)
		{
			return false;
		}

		// Reverse the order to CCW if the order is CW
		switch (Math::getVertexOrder(shaderData->s0, shaderData->s1, shaderData->s2))
		{
		case EWindingOrder::CW: // Triangle is back-facing, exit
		case EWindingOrder::CL: // Triangle has zero area, exit
			return false;
		case EWindingOrder::CCW: // Triangle is front-facing, continue
			break;
		}

		// Get the bounding box of the 2d triangle clipped to the viewport
		shaderData->screenBounds = rectf::makeBoundingBox(shaderData->s0, shaderData->s1, shaderData->s2);

		// Grow the bounds by 1 pixel to account for gaps between pixels.
		shaderData->screenBounds.grow(1.0f);

		// Clamp the bounds to the viewport
		const rectf viewportRect = {
			0, 0, static_cast<float>(viewData->width), static_cast<float>(viewData->height)
		};
		shaderData->screenBounds.clamp(viewportRect);

		if (shaderData->hasNormals)
		{
			// Average each of the vertices' normals to get the triangle normal
			vec4f v01Normal;
			vecAddVec(shaderData->v0.normal, shaderData->v1.normal, v01Normal);

			vec4f v012Normal;
			vecAddVec(v01Normal, shaderData->v2.normal, v012Normal);

			shaderData->triangleWorldNormal = v012Normal * 0.33333333f;

			vecDotVec(-viewData->cameraDirection, shaderData->triangleWorldNormal, &shaderData->facingRatio);
			if (shaderData->facingRatio < 0.0f)
			{
				return false;
			}

			// Calculate the triangle normal relative to the camera
			vec4f tmp;
			vecCrossVec(shaderData->triangleWorldNormal, viewData->cameraDirection, tmp);

			shaderData->triangleCameraNormal.x = tmp.x;
			shaderData->triangleCameraNormal.y = tmp.y;
			shaderData->triangleCameraNormal.z = tmp.z;
		}

		return true;
	}
};

class ScanlinePixelShader : public PixelShader
{
public:
	ScanlinePixelShader() = default;

	bool process(const ViewData* viewData, ScanlineShaderData* shaderData) const
	{
		float weightedFacingRatio = 1.0f;

		if (shaderData->hasNormals)
		{
#ifndef PENG_SSE
			// Calculate the weighted normal of the current point on this triangle. This uses the UVW
			// barycentric coordinates to weight each vertex normal of the triangle.
			vec3f weightedWorldNormal = shaderData->v0.normal * shaderData->bary.x + shaderData->v1.normal * shaderData->bary.y + shaderData->v2.normal * shaderData->bary.z;

			// Calculate the dot product of the triangle normal and inverse camera direction

			vecDotVec(-viewData->cameraDirection, weightedWorldNormal, &weightedFacingRatio);
			
#else
			// Convert vectors to __m128
			vecm vn0(shaderData->v0.normal);
			vecm vn1(shaderData->v1.normal);
			vecm vn2(shaderData->v2.normal);
			vecm bx(vec3f(shaderData->bary.x));
			vecm by(vec3f(shaderData->bary.y));
			vecm bz(vec3f(shaderData->bary.z));
			vecm cd(-viewData->cameraDirection);

			// Multiply each normal by a vec3 of each component of the Barycentric coords
			__m128 a = _mm_mul_ps(vn0.m, bx.m);
			__m128 b = _mm_mul_ps(vn1.m, by.m);
			__m128 c = _mm_mul_ps(vn2.m, bz.m);

			// Add all components together
			__m128 weightedWorldNormal = _mm_add_ps(_mm_add_ps(a, b), c);

			// Dot product of negative camera direction and the weighted world normal
			weightedFacingRatio = _mm_cvtss_f32(_mm_dp_ps(cd.m, weightedWorldNormal, 0b11101111));
			// Ignore W component of out
#endif
			// Clamp to 0..1
			weightedFacingRatio = std::clamp(weightedFacingRatio, 0.0f, 1.0f);
		}

		// If a texture is loaded in the current shader, get the current color from the current
		// UV coordinates of this pixel.
		if (shaderData->texture && shaderData->hasTexCoords)
		{
			// Compute the relative UV coordinates on the texture
			int32 x = shaderData->uv.x * (shaderData->texture->getWidth() - 1);
			int32 y = shaderData->uv.y * (shaderData->texture->getHeight() - 1);

			// Set the outColor to the pixel at [x,y] in the texture
			shaderData->outColor = shaderData->texture->getPixelAsColor(x, y);
		}
		else
		{
			shaderData->outColor = shaderData->baseColor;
		}

		// Multiply outColor by the facing ratio to give some shading
		shaderData->outColor.r *= weightedFacingRatio;
		shaderData->outColor.g *= weightedFacingRatio;
		shaderData->outColor.b *= weightedFacingRatio;

		return true;
	}
};

class ScanlineRenderPipeline : public IRenderPipeline
{
	std::shared_ptr<ScanlineVertexShader> m_vertexShader = nullptr;
	std::shared_ptr<ScanlinePixelShader> m_pixelShader   = nullptr;

	std::shared_ptr<Texture> m_frameBuffer = nullptr;
	std::shared_ptr<Texture> m_depthBuffer = nullptr;

	std::shared_ptr<ViewData> m_viewData             = nullptr;
	std::shared_ptr<ScanlineShaderData> m_shaderData = nullptr;

	Mesh* m_currentMesh                              = nullptr;
	Triangle* m_currentTriangle                      = nullptr;
	std::shared_ptr<RenderSettings> m_renderSettings = nullptr;

public:
	ScanlineRenderPipeline() = default;

	bool init(void* windowHandle) override;
	void beginDraw() override;
	void draw() override;
	void endDraw() override;
	void shutdown() override {}
	void resize(int32 width, int32 height) override;

	void drawGrid(Grid* grid) override;
	void drawTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2);
	void drawScanline() const;
	void drawLine(const vec3f& inA, const vec3f& inB, const Color& color) override;

	uint8* getFrameData() override;
	void setViewData(ViewData* newViewData) override;
	void setRenderSettings(RenderSettings* newRenderSettings) override;
	void setVertexData(float* data, size_t size, int32 count) override;
};
