#include "Engine/Engine.h"
#include "Platforms/Generic/GenericApplication.h"

bool Engine::initialize(IApplication* app)
{
	return true;
}

bool Engine::shutdown()
{
	return true;
}

void Engine::tick(float deltaTime) {}

void Engine::onKeyPressed(const EKey keyCode) const {}

void Engine::onLeftMouseDown(MouseData& mouse) const {}

void Engine::onLeftMouseUp(MouseData& mouse) const {}

void Engine::onMiddleMouseUp(MouseData& mouse) const {}

void Engine::onMouseMoved(MouseData& mouse) const {}

void Engine::onMouseMiddleScrolled(MouseData& mouse) const {}
