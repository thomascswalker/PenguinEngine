#pragma once

#include "Camera.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "Framework/Engine/Mesh.h"

struct IShader
{
	virtual ~IShader() = default;
	int32 m_width, m_height;
	Vertex m_v0, m_v1, m_v2;
	vec3f m_s0, m_s1, m_s2;

	vec3f m_cameraPosition;
	vec3f m_cameraWorldDirection;

	vec3f m_triangleWorldNormal;
	vec3f m_triangleCameraNormal;

	vec3f m_pixelWorldPosition;
	float m_facingRatio;

	rectf m_screenBounds;
	mat4f m_mvp;
	mat4f m_viewMatrix;
	mat4f m_projectionMatrix;
	vec3f m_uvw;

	bool m_hasNormals = false;
	bool m_hasTexCoords = false;

	int32 m_outColor = Color::magenta().toInt32();

	PViewData m_viewData;

	void init(const PViewData& inViewData)
	{
		m_viewData = inViewData;
		m_mvp = inViewData.m_viewProjectionMatrix;
		m_cameraWorldDirection = inViewData.m_direction;
		m_cameraPosition = inViewData.m_translation;
		m_width = inViewData.m_width;
		m_height = inViewData.m_height;
	}

	virtual bool computeVertexShader(const Vertex& inV0, const Vertex& inV1, const Vertex& inV2)
	{
		m_v0 = inV0;
		m_v1 = inV1;
		m_v2 = inV2;

		// Project the world-space points to screen-space
		bool bTriangleOnScreen = false;
		bTriangleOnScreen |= Math::projectWorldToScreen(m_v0.m_position, m_s0, m_viewData);
		bTriangleOnScreen |= Math::projectWorldToScreen(m_v1.m_position, m_s1, m_viewData);
		bTriangleOnScreen |= Math::projectWorldToScreen(m_v2.m_position, m_s2, m_viewData);

		// If the triangle is completely off screen, exit
		if (!bTriangleOnScreen)
		{
			return false;
		}

		// Reverse the order to CCW if the order is CW
		switch (Math::GetVertexOrder(m_s0, m_s1, m_s2))
		{
		case EWindingOrder::CW: // Triangle is back-facing, exit
		case EWindingOrder::CL: // Triangle has zero area, exit
			return false;
		case EWindingOrder::CCW: // Triangle is front-facing, continue
			break;
		}

		// Get the bounding box of the 2d triangle clipped to the viewport
		m_screenBounds = rectf::MakeBoundingBox(m_s0, m_s1, m_s2);

		// Grow the bounds by 1 pixel to account for gaps between pixels.
		m_screenBounds.Grow(1.0f);

		// Clamp the bounds to the viewport
		const rectf viewportRect = {0, 0, static_cast<float>(m_width), static_cast<float>(m_height)};
		m_screenBounds.Clamp(viewportRect);

		// Average each of the vertices' normals to get the triangle normal
		vec4f v01Normal;
		vecAddVec(m_v0.m_normal, m_v1.m_normal, v01Normal);

		vec4f v012Normal;
		vecAddVec(v01Normal, m_v2.m_normal, v012Normal);

		m_triangleWorldNormal = v012Normal * 0.33333333f;

		vecDotVec(-m_cameraWorldDirection, m_triangleWorldNormal, &m_facingRatio);
		if (m_facingRatio < 0.0f)
		{
			return false;
		}

		// Calculate the triangle normal relative to the camera
		vec4f tmp;
		vecCrossVec(m_triangleWorldNormal, m_cameraWorldDirection, tmp);
		m_triangleCameraNormal.X = tmp.X;
		m_triangleCameraNormal.Y = tmp.Y;
		m_triangleCameraNormal.Z = tmp.Z;

		return true;
	}

	// No default implementation
	virtual void computePixelShader(float x, float y) = 0;
};

struct DefaultShader : IShader
{
	void computePixelShader(float u, float v) override
	{
		// Calculate the weighted normal of the current point on this triangle. This uses the UVW
		// barycentric coordinates to weight each vertex normal of the triangle.
		const vec3f weightedWorldNormal = m_v0.m_normal * m_uvw.X + m_v1.m_normal * m_uvw.Y + m_v2.m_normal * m_uvw.
			Z;

		// Calculate the dot product of the triangle normal and camera direction
		vecDotVec(-m_cameraWorldDirection, weightedWorldNormal, &m_facingRatio);
		m_facingRatio = Math::Max(0.0f, m_facingRatio); // Floor to a min of 0
		const float clampedFacingRatio = Math::Min(m_facingRatio * 255.0f, 255.0f); // Clamp to a max of 255

		uint8 r = static_cast<uint8>(clampedFacingRatio);
		uint8 g = r;
		uint8 b = r;

		m_outColor = (r << 16) | (g << 8) | b | 0;
	}
};
