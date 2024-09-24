#pragma once

#include "Math/MathFwd.h"
#include "Engine/Object.h"
#include "Math/Spherical.h"

constexpr int32 g_windowWidthClip       = 16;
constexpr int32 g_windowHeightClip      = 59;
constexpr int32 g_defaultViewportWidth  = 640 + g_windowWidthClip;
constexpr int32 g_defaultViewportHeight = 480 + g_windowHeightClip;
constexpr int32 g_minWindowWidth        = 320;
constexpr int32 g_minWindowHeight       = 240;
constexpr int32 g_maxWindowWidth        = 1280;
constexpr int32 g_maxWindowHeight       = 720;

constexpr float g_defaultFov  = 54.3f;
constexpr float g_defaultMinz = 0.1f;
constexpr float g_defaultMaxz = 100.0f;

constexpr float g_defaultMinZoom      = 10.0f;
const auto g_defaultCameraTranslation = vec3f(-36, 30, 34);

enum EViewportType
{
	Perspective,
	Othographic
};

struct ViewData
{
	int32 width   = g_defaultViewportWidth;
	int32 height  = g_defaultViewportHeight;
	float fov     = g_defaultFov;
	float minZ    = 1.0f;
	float maxZ    = 10.0f;
	float minFov  = 20.0f;
	float maxFov  = 120.0f;
	float minZoom = 2.0f;
	float maxZoom = 100.0f;

	vec3f target = vec3f::zeroVector(); // Origin
	sphericalf spherical;
	sphericalf sphericalDelta;
	float minPolarAngle = 0.0f;
	float maxPolarAngle = PI;
	vec3f panOffset;

	mat4f projectionMatrix;
	mat4f viewMatrix;
	mat4f modelMatrix;
	mat4f viewProjectionMatrix;
	mat4f invViewProjectionMatrix;

	vec3f cameraDirection;
	vec3f cameraTranslation;
};

class Camera : public Object
{
public:
	ViewData m_viewData;
	int32 m_width   = g_defaultViewportWidth;
	int32 m_height  = g_defaultViewportHeight;
	float m_fov     = g_defaultFov;
	float m_minZ    = 1.0f;
	float m_maxZ    = 10.0f;
	float m_minFov  = 20.0f;
	float m_maxFov  = 120.0f;
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

	constexpr float getAspect() const
	{
		return static_cast<float>(m_width) / static_cast<float>(m_height);
	}

	void computeViewProjectionMatrix();
	void orbit(float dx, float dy);
	void pan(float dx, float dy);
	void zoom(float value);
	void setFov(float newFov);

	void setLookAt(const vec3f& newLookAt)
	{
		m_target = newLookAt;
	}

	void update(float deltaTime) override;

	/* General Math */
	// https://github.com/EpicGames/UnrealEngine/blob/c830445187784f1269f43b56f095493a27d5a636/Engine/Source/Runtime/Engine/Private/SceneView.cpp#L1431
	void deprojectScreenToWorld(const vec2f& screenPoint, vec3f& outWorldPosition,
	                            vec3f& outWorldDirection) const;

	ViewData* getViewData()
	{
		m_viewData.width                   = m_width;
		m_viewData.height                  = m_height;
		m_viewData.fov                     = m_fov;
		m_viewData.minZ                    = m_minZ;
		m_viewData.maxZ                    = m_maxZ;
		m_viewData.target                  = m_target;
		m_viewData.spherical               = m_spherical;
		m_viewData.projectionMatrix        = m_projectionMatrix;
		m_viewData.viewMatrix              = m_viewMatrix;
		m_viewData.viewProjectionMatrix    = m_viewProjectionMatrix;
		m_viewData.invViewProjectionMatrix = m_invViewProjectionMatrix;
		m_viewData.cameraDirection         = getForwardVector();
		m_viewData.cameraTranslation       = getTranslation();
		return &m_viewData;
	}
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
