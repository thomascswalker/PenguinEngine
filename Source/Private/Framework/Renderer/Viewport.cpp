#include "Framework/Renderer/Viewport.h"
#include "Framework/Engine/Engine.h"
#include "Framework/Input/InputHandler.h"
#include "Math/MathCommon.h"
#include "Math/Vector.h"

Viewport::Viewport(const uint32 inWidth, const uint32 inHeight)
{
	m_camera = std::make_shared<Camera>();
	resize(inWidth, inHeight);
}

void Viewport::resize(const uint32 inWidth, const uint32 inHeight) const
{
	m_camera->m_width = inWidth;
	m_camera->m_height = inHeight;
}

vec2f Viewport::getSize() const
{
	return {static_cast<float>(m_camera->m_width), static_cast<float>(m_camera->m_height)};
}

void Viewport::resetView() const
{
	m_camera->setTranslation(g_defaultCameraTranslation);
	m_camera->setRotation(rotf());

	m_camera->m_target = vec3f::zeroVector();
	m_camera->computeViewProjectionMatrix();
}

/**
 * \brief Projects the specified `WorldPosition` into the in/out `ScreenPosition` using the specified `ViewProjectionMatrix`.
 * \param worldPosition The world position of the point to be projected.
 * \param screenPosition The out screen position.
 * \return True if the position could be projected, false otherwise.
 */
bool Viewport::projectWorldToScreen(const vec3f& worldPosition, vec3f& screenPosition) const
{
	// Clip space
	mat4f model;
	mat4f mvp = m_camera->m_viewProjectionMatrix * model;
	vec4f result = mvp * vec4f(worldPosition.x, worldPosition.y, worldPosition.z, 1.0f);
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
			normalizedPosition.x * static_cast<float>(m_camera->m_width),
			normalizedPosition.y * static_cast<float>(m_camera->m_height),
			(result.z + 1.0f) * 0.5f
		};
		return true;
	}

	return false;
}

bool Viewport::projectScreenToWorld(const vec2f& screenPosition, float depth, vec3f& worldPosition) const
{
	return true;
}

void Viewport::formatDebugText()
{
	const Engine* engine = Engine::getInstance();
	const IInputHandler* inputHandler = IInputHandler::getInstance();

	m_debugText = std::format(
		"Stats\n"
		"FPS: {}\n"
		"Size: {}\n",
		engine->getFps(),
		getSize().toString()
	);
}
