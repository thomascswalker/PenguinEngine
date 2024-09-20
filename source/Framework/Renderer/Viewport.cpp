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
	m_camera->m_width  = inWidth;
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

void Viewport::formatDebugText()
{
	const Engine* engine              = Engine::getInstance();
	const IInputHandler* inputHandler = IInputHandler::getInstance();

	m_debugText = std::format(
		"Stats\n"
		"FPS: {}\n"
		"Size: {}\n",
		engine->getFps(),
		getSize().toString()
	);
}
