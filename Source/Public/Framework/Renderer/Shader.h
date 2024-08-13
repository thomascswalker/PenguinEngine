#pragma once

#include "Camera.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "Framework/Engine/Mesh.h"

struct IShader
{
	virtual ~IShader() = default;
	int32 width, height;
	Vertex v0, v1, v2;
	vec3f s0, s1, s2;

	vec3f cameraPosition;
	vec3f cameraWorldDirection;

	vec3f triangleWorldNormal;
	vec3f triangleCameraNormal;

	vec3f pixelWorldPosition;
	float facingRatio;

	rectf screenBounds;
	mat4f mvp;
	mat4f viewMatrix;
	mat4f projectionMatrix;
	vec3f uvw;

	bool hasNormals = false;
	bool hasTexCoords = false;

	Color outColor = Color::magenta();

	PViewData viewData;

	void init(const PViewData& inViewData)
	{
		viewData = inViewData;
		mvp = inViewData.m_viewProjectionMatrix;
		cameraWorldDirection = inViewData.m_direction;
		cameraPosition = inViewData.m_translation;
		width = inViewData.m_width;
		height = inViewData.m_height;
	}

	// No default implementation
	virtual bool computeVertexShader(const Vertex& inV0, const Vertex& inV1, const Vertex& inV2) = 0;
	virtual void computePixelShader(float x, float y) = 0;
};

struct DefaultShader : IShader
{
	bool computeVertexShader(const Vertex& inV0, const Vertex& inV1, const Vertex& inV2) override
	{
		v0 = inV0;
		v1 = inV1;
		v2 = inV2;

		// Project the world-space points to screen-space
		bool triangleOnScreen = false;
		triangleOnScreen |= Math::projectWorldToScreen(v0.m_position, s0, viewData);
		triangleOnScreen |= Math::projectWorldToScreen(v1.m_position, s1, viewData);
		triangleOnScreen |= Math::projectWorldToScreen(v2.m_position, s2, viewData);

		// If the triangle is completely off screen, exit
		if (!triangleOnScreen)
		{
			return false;
		}

		// Reverse the order to CCW if the order is CW
		switch (Math::getVertexOrder(s0, s1, s2))
		{
		case EWindingOrder::CW: // Triangle is back-facing, exit
		case EWindingOrder::CL: // Triangle has zero area, exit
			return false;
		case EWindingOrder::CCW: // Triangle is front-facing, continue
			break;
		}

		// Get the bounding box of the 2d triangle clipped to the viewport
		screenBounds = rectf::makeBoundingBox(s0, s1, s2);

		// Grow the bounds by 1 pixel to account for gaps between pixels.
		screenBounds.grow(1.0f);

		// Clamp the bounds to the viewport
		const rectf viewportRect = {0, 0, static_cast<float>(width), static_cast<float>(height)};
		screenBounds.clamp(viewportRect);

		// Average each of the vertices' normals to get the triangle normal
		vec4f v01Normal;
		vecAddVec(v0.m_normal, v1.m_normal, v01Normal);

		vec4f v012Normal;
		vecAddVec(v01Normal, v2.m_normal, v012Normal);

		triangleWorldNormal = v012Normal * 0.33333333f;

		vecDotVec(-cameraWorldDirection, triangleWorldNormal, &facingRatio);
		if (facingRatio < 0.0f)
		{
			return false;
		}

		// Calculate the triangle normal relative to the camera
		vec4f tmp;
		vecCrossVec(triangleWorldNormal, cameraWorldDirection, tmp);
		triangleCameraNormal.x = tmp.x;
		triangleCameraNormal.y = tmp.y;
		triangleCameraNormal.z = tmp.z;

		return true;
	}

	void computePixelShader(float u, float v) override
	{
		// Calculate the weighted normal of the current point on this triangle. This uses the UVW
		// barycentric coordinates to weight each vertex normal of the triangle.
		const vec3f weightedWorldNormal = v0.m_normal * uvw.x + v1.m_normal * uvw.y + v2.m_normal * uvw.
			z;

		// Calculate the dot product of the triangle normal and camera direction
		vecDotVec(-cameraWorldDirection, weightedWorldNormal, &facingRatio);
		facingRatio = std::max(0.0f, facingRatio);                               // Floor to a min of 0
		const float clampedFacingRatio = std::min(facingRatio * 255.0f, 255.0f); // Clamp to a max of 255

		outColor.r = static_cast<uint8>(clampedFacingRatio);
		outColor.g = outColor.r;
		outColor.b = outColor.r;
	}
};
