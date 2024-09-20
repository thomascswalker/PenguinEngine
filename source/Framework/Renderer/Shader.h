#pragma once
#pragma warning(disable : 4244)

#include "Camera.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "Framework/Renderer/Texture.h"
#include "Framework/Engine/Mesh.h"

struct IShader
{
	virtual ~IShader() = default;
	int32 width, height;
	Vertex v0, v1, v2;
	vec3f s0, s1, s2;

	vec3f cameraPosition;
	vec3f cameraWorldDirection;

	vec3f pixelWorldPosition;
	float facingRatio;

	rectf screenBounds;
	mat4f mvp;
	mat4f viewMatrix;
	mat4f projectionMatrix;
	vec3f bary;
	vec2f uv;
	float z = 0.0f;

	bool hasNormals = false;
	vec3f triangleWorldNormal;
	vec3f triangleCameraNormal;

	bool hasTexCoords = false;
	Texture* texture  = nullptr;

	Color baseColor = Color::white();
	Color outColor  = Color::white();

	PViewData viewData;

	bool tiling = false;

	/**
	 * @brief Initializes this shader with the specified view data.
	 * @param inViewData The view data from the current camera in the viewport.
	 */
	void init(const PViewData& inViewData)
	{
		viewData             = inViewData;
		mvp                  = inViewData.m_viewProjectionMatrix;
		cameraWorldDirection = inViewData.m_direction;
		cameraPosition       = inViewData.m_translation;
		width                = inViewData.m_width;
		height               = inViewData.m_height;
	}

	/**
	 * @brief Compute the absolute UV position in 0..1.
	 */
	virtual void computeUv() = 0;

	virtual void preComputeVertexShader() {}

	/**
	 * @brief Computes the final pixel position of each vertex.
	 * @param inV0 The first vertex of the triangle.
	 * @param inV1 The second vertex of the triangle.
	 * @param inV2 The third vertex of the triangle.
	 * @return bool True if any or all vertices are projected onto the screen, false if none are projected (visible).
	 */
	virtual bool computeVertexShader(const Vertex& inV0, const Vertex& inV1, const Vertex& inV2) = 0;

	virtual void preComputePixelShader()
	{
		outColor = baseColor;
	}

	/**
	 * @brief Computes the final pixel color.
	 * @param x The screen X coordinate.
	 * @param y The screen Y coordinate.
	 */
	virtual void computePixelShader(float x, float y) = 0;
};

struct DefaultShader : IShader
{
	void computeUv() override
	{
		if (!hasTexCoords)
		{
			return;
		}

		// Use the barycentric coordinates to interpolate between all three vertex UV coordinates
		uv = v0.texCoord * bary.x + v1.texCoord * bary.y + v2.texCoord * bary.z;

		// TODO: Figure out how to fix perspective distortion
		// float w = 1.0f / z;
		// uv /= z;
	}

	bool computeVertexShader(const Vertex& inV0, const Vertex& inV1, const Vertex& inV2) override
	{
		// Store the input vertices
		v0 = inV0;
		v1 = inV1;
		v2 = inV2;

		// If we're not tiling, the triangle hasn't been projected yet. We need to project it and validate
		// that some or all of it is on screen.
		if (!tiling)
		{
			// Project the world-space points to screen-space
			bool triangleOnScreen = false;
			triangleOnScreen |= Math::projectWorldToScreen(v0.position, s0, viewData);
			triangleOnScreen |= Math::projectWorldToScreen(v1.position, s1, viewData);
			triangleOnScreen |= Math::projectWorldToScreen(v2.position, s2, viewData);

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
		}

		// Get the bounding box of the 2d triangle clipped to the viewport
		screenBounds = rectf::makeBoundingBox(s0, s1, s2);

		// Grow the bounds by 1 pixel to account for gaps between pixels.
		screenBounds.grow(1.0f);

		// Clamp the bounds to the viewport
		const rectf viewportRect = {0, 0, static_cast<float>(width), static_cast<float>(height)};
		screenBounds.clamp(viewportRect);

		// Determine if this triangle has normals by just comparing if they're all equal
		// to each other (false) or not (true).
		//hasNormals = false;
		//hasNormals &= v0.normal != v1.normal;
		//hasNormals &= v1.normal != v2.normal;
		//hasNormals &= v0.normal != v2.normal;
		if (hasNormals)
		{
			// Average each of the vertices' normals to get the triangle normal
			vec4f v01Normal;
			vecAddVec(v0.normal, v1.normal, v01Normal);

			vec4f v012Normal;
			vecAddVec(v01Normal, v2.normal, v012Normal);

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
		}

		return true;
	}

	void computePixelShader(float u, float v) override
	{
		vec3f weightedWorldNormal(1.0f);
		float weightedFacingRatio = 1.0f;

		if (hasNormals)
		{
			// Calculate the weighted normal of the current point on this triangle. This uses the UVW
			// barycentric coordinates to weight each vertex normal of the triangle.
			weightedWorldNormal = v0.normal * bary.x + v1.normal * bary.y + v2.normal * bary.z;

			// Calculate the dot product of the triangle normal and inverse camera direction

			vecDotVec(-cameraWorldDirection, weightedWorldNormal, &weightedFacingRatio);

			// Clamp to 0..1
			weightedFacingRatio = std::clamp(weightedFacingRatio, 0.0f, 1.0f);
		}

		// If a texture is loaded in the current shader, get the current color from the current
		// UV coordinates of this pixel.
		if (texture && hasTexCoords)
		{
			// Compute the relative UV coordinates on the texture
			int32 x = uv.x * (texture->getWidth() - 1);
			int32 y = uv.y * (texture->getHeight() - 1);

			// Set the outColor to the pixel at [x,y] in the texture
			outColor = texture->getPixelAsColor(x, y);
		}

		// Multiply outColor by the facing ratio to give some shading
		outColor.r *= weightedFacingRatio;
		outColor.g *= weightedFacingRatio;
		outColor.b *= weightedFacingRatio;
	}
};
