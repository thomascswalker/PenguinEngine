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
constexpr float g_defaultMaxz = 1000.0f;

constexpr float g_defaultMinZoom      = 1.0f;
constexpr float g_defaultMaxZoom      = 1000.0f;
constexpr float g_defaultZoom         = 36.0f;
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
	sphericalf spherical;
	sphericalf sphericalDelta;
	float minPolarAngle = 0.0f;
	float maxPolarAngle = g_pi;
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
	float m_minZ    = g_defaultMinz;
	float m_maxZ    = g_defaultMaxz;
	float m_minFov  = 20.0f;
	float m_maxFov  = 120.0f;
	float m_zoom    = g_defaultZoom;
	float m_minZoom = g_defaultMinZoom;
	float m_maxZoom = g_defaultMaxZoom;

	vec3f m_target = vec3f::zeroVector(); // Origin
	sphericalf m_spherical;
	sphericalf m_sphericalDelta;
	float m_minPolarAngle = 0.0f;
	float m_maxPolarAngle = g_pi;
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
	void update(float deltaTime) override;
	void setFov(float newFov);
	float getTargetDistance() const;

	void setLookAt(const vec3f& newLookAt)
	{
		m_target = newLookAt;
	}

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

	static vec3f clip(const vec4f& input, int32 width, int32 height)
	{
		// Apply perspective correction
		const vec3f clipPosition{
			input.x / input.w,
			input.y / input.w,
			input.z / input.w
		};

		// Normalized device coordinates
		const vec2f normalizedPosition{
			(clipPosition.x / 2.0f) + 0.5f,
			(clipPosition.y / 2.0f) + 0.5f,
		};

		// Apply the current render width and height
		return vec3f{
			normalizedPosition.x * static_cast<float>(width),
			normalizedPosition.y * static_cast<float>(height),
			(clipPosition.z + 0.5f) * 0.5f
		};
	}
}
