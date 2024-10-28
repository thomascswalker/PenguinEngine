#pragma once

#include "Engine/Actors/Actor.h"
#include "Math/MathFwd.h"
#include "Math/Spherical.h"
#include "Platforms/Generic/PlatformMemory.h"


constexpr float g_defaultFov  = 54.3f;
constexpr float g_defaultMinz = 0.1f;
constexpr float g_defaultMaxz = 1000.0f;

constexpr float g_defaultMinZoom = 1.0f;
constexpr float g_defaultMaxZoom = 1000.0f;
constexpr float g_defaultZoom    = 36.0f;

constexpr float g_defaultPhi   = 1.02609062f;
constexpr float g_defaultTheta = -0.813961744f;

constexpr float g_defaultPanSpeed   = 0.001f;
constexpr float g_defaultOrbitSpeed = 0.001f;
constexpr float g_defaultZoomSpeed  = 0.01f;

const auto g_defaultCameraTranslation = vec3f(-36, 30, 34);

enum class EViewportType : uint8
{
	Perspective,
	Othographic
};

struct ViewData
{
	int32 width   = g_defaultViewportWidth;
	int32 height  = g_defaultViewportHeight;
	float fov     = g_defaultFov;
	float minZ    = g_defaultMinz;
	float maxZ    = g_defaultMaxz;
	float minFov  = 20.0f;
	float maxFov  = 120.0f;
	float minZoom = g_defaultMinZoom;
	float maxZoom = g_defaultMaxZoom;

	vec3f target = vec3f::zeroVector(); // Origin

	mat4f modelMatrix;
	mat4f viewProjectionMatrix;
	mat4f modelViewProjectionMatrix;

	vec3f cameraDirection;
	vec3f cameraTranslation;
};

class Camera : public Actor
{
public:
	ViewData m_viewData;
	int32 m_width   = g_defaultViewportWidth;
	int32 m_height  = g_defaultViewportHeight;
	float m_fov     = g_defaultFov;
	float m_minZ    = g_defaultMinz;
	float m_maxZ    = g_defaultMaxz;
	float m_minFov  = 20.0f;
	float m_maxFov  = 120.0f;
	float m_zoom    = g_defaultZoom;
	float m_minZoom = g_defaultMinZoom;
	float m_maxZoom = g_defaultMaxZoom;

	vec3f m_targetTranslation = vec3f::zeroVector(); // Origin
	sphericalf m_spherical;
	sphericalf m_deltaRotation;
	float m_minPolarAngle = 0.0f;
	float m_maxPolarAngle = g_pi;
	vec3f m_deltaTranslation;

	mat4f m_projectionMatrix;
	mat4f m_viewMatrix;
	mat4f m_viewProjectionMatrix;
	mat4f m_invViewProjectionMatrix;

	float m_panSpeed   = g_defaultPanSpeed;
	float m_orbitSpeed = g_defaultOrbitSpeed;
	float m_zoomSpeed  = g_defaultZoomSpeed;

	/**
	 * Constructor for the Camera class.
	 */
	Camera();
	void update(float deltaTime) override;

	[[nodiscard]] constexpr float getAspect() const;
	void computeViewProjectionMatrix();
	void orbit(float dx, float dy);
	void pan(float dx, float dy);
	void zoom(float value);
	void setFov(float newFov);
	[[nodiscard]] float getTargetDistance() const;
	void setLookAt(const vec3f& newLookAt);
	ViewData* getViewData();
	void setDefault();
};

namespace Math
{
	static bool projectWorldToScreen(const vec3f& worldPosition, vec3f& screenPosition, const ViewData& viewData)
	{
		// Clip 
		const vec4f result = viewData.viewProjectionMatrix * vec4f(worldPosition, 1.0f);
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
				normalizedPosition.x * static_cast<float>(viewData.width),
				normalizedPosition.y * static_cast<float>(viewData.height),
				(clipPosition.z + 0.5f) * 0.5f
			};
			return true;
		}
		return false;
	}

	static bool deprojectScreenToWorld(const vec2f& screenPosition, const ViewData& viewData)
	{
		return true;
	}
}
