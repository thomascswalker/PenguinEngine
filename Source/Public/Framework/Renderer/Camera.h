#pragma once

#include "Math/MathFwd.h"
#include "Framework/Engine/Object.h"
#include "Math/Spherical.h"

constexpr int32 g_windowWidthClip = 16;
constexpr int32 g_windowHeightClip = 59;
constexpr int32 g_defaultViewportWidth = 640 + g_windowWidthClip;
constexpr int32 g_defaultViewportHeight = 480 + g_windowHeightClip;

constexpr float g_defaultFov = 54.3f;
constexpr float g_defaultMinz = 0.1f;
constexpr float g_defaultMaxz = 100.0f;

constexpr float g_defaultMinZoom = 10.0f;
const auto g_defaultCameraTranslation = vec3f(-36, 30, 34);

enum EViewportType
{
	Perspective,
	Othographic
};

struct PViewData
{
	int32 m_width = g_defaultViewportWidth;
	int32 m_height = g_defaultViewportHeight;
	float m_fov = g_defaultFov;
	float m_minZ = 1.0f;
	float m_maxZ = 10.0f;
	float m_minFov = 20.0f;
	float m_maxFov = 120.0f;
	float m_minZoom = 2.0f;
	float m_maxZoom = 100.0f;

	vec3f m_target = vec3f::zeroVector(); // Origin
	sphericalf m_spherical;
	sphericalf m_sphericalDelta;
	float m_minPolarAngle = 0.0f;
	float m_maxPolarAngle = PI;
	vec3f m_panOffset;

	mat4f m_projectionMatrix;
	mat4f m_viewMatrix;
	mat4f m_viewProjectionMatrix;
	mat4f m_invViewProjectionMatrix;

	vec3f m_direction;
	vec3f m_translation;
};

class Camera : public Object
{
public:
	int32 m_width = g_defaultViewportWidth;
	int32 m_height = g_defaultViewportHeight;
	float m_fov = g_defaultFov;
	float m_minZ = 1.0f;
	float m_maxZ = 10.0f;
	float m_minFov = 20.0f;
	float m_maxFov = 120.0f;
	float m_minZoom = 2.0f;
	float m_maxZoom = 100.0f;

	vec3f m_target = vec3f::zeroVector(); // Origin
	sphericalf m_spherical;
	sphericalf m_sphericalDelta;
	float m_minPolarAngle = 0.0f;
	float m_maxPolarAngle = PI;
	vec3f m_panOffset;

	mat4f m_projectionMatrix;
	mat4f m_viewMatrix;
	mat4f m_viewProjectionMatrix;
	mat4f m_invViewProjectionMatrix;

	/**
	 * Constructor for the Camera class.
	 * Initializes the camera by calling the Init() function.
	 */
	Camera()
	{
		init();
	}

	void init()
	{
		setTranslation(g_defaultCameraTranslation);
	}

	constexpr float getAspect() const { return static_cast<float>(m_width) / static_cast<float>(m_height); }
	void computeViewProjectionMatrix();
	void orbit(float dx, float dy);
	void pan(float dx, float dy);
	void zoom(float value);
	void setFov(float newFov);
	void setLookAt(const vec3f& newLookAt) { m_target = newLookAt; }

	void update(float deltaTime) override;

	/* General Math */
	// https://github.com/EpicGames/UnrealEngine/blob/c830445187784f1269f43b56f095493a27d5a636/Engine/Source/Runtime/Engine/Private/SceneView.cpp#L1431
	void deprojectScreenToWorld(const vec2f& screenPoint, vec3f& outWorldPosition,
	                            vec3f& outWorldDirection) const;

	PViewData getViewData() const
	{
		PViewData data;

		data.m_width = m_width;
		data.m_height = m_height;
		data.m_fov = m_fov;
		data.m_minZ = m_minZ;
		data.m_maxZ = m_maxZ;
		data.m_target = m_target;
		data.m_spherical = m_spherical;
		data.m_projectionMatrix = m_projectionMatrix;
		data.m_viewMatrix = m_viewMatrix;
		data.m_viewProjectionMatrix = m_viewProjectionMatrix;
		data.m_invViewProjectionMatrix = m_invViewProjectionMatrix;
		data.m_direction = getForwardVector();
		data.m_translation = getTranslation();

		return data;
	}
};

namespace Math
{
	static bool projectWorldToScreen(const vec3f& worldPosition, vec3f& screenPosition, const PViewData& viewData)
	{
		// Clip 
		const vec4f result = viewData.m_viewProjectionMatrix * vec4f(worldPosition, 1.0f);
		if (result.w > 0.0f)
		{
			// Apply perspective correction
			const vec3f clipPosition{
				result.x / result.w,
				result.y / result.w,
				result.z / result.w
			};

			// Normalized device coordinates
			const vec2f normalizedPosition{
				(clipPosition.x / 2.0f) + 0.5f,
				(clipPosition.y / 2.0f) + 0.5f,
			};

			// Apply the current render width and height
			screenPosition = vec3f{
				normalizedPosition.x * static_cast<float>(viewData.m_width),
				normalizedPosition.y * static_cast<float>(viewData.m_height),
				(clipPosition.z + 0.5f) * 0.5f
			};
			return true;
		}
		return false;
	}

	static bool deprojectScreenToWorld(const vec2f& screenPosition, const PViewData& viewData)
	{
		return true;
	}
}
