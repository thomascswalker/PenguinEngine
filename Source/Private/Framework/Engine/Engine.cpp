#include <Framework/Engine/Engine.h>
#include <Framework/Engine/Mesh.h>
#include "Framework/Engine/Timer.h"
#include "Framework/Input/InputHandler.h"
#include "Framework/Application.h"
#include "Framework/Importers/MeshImporter.h"

#include "Framework/Platforms/PlatformInterface.h"

Engine* Engine::m_instance = getInstance();

Engine* Engine::getInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new Engine();
	}
	return m_instance;
}


bool Engine::startup(uint32 inWidth, uint32 inHeight)
{
	LOG_INFO("Starting up engine.")

	m_renderer = std::make_shared<Renderer>(inWidth, inHeight);
	m_isRunning = true;

	// Track starting time
	m_startTime = PTimer::now();

	// Bind input events
	if (IInputHandler* input = Win32InputHandler::getInstance())
	{
		// Keyboard
		input->m_keyPressed.AddRaw(this, &Engine::onKeyPressed);

		// Mouse
		input->m_onMouseMiddleScrolled.AddRaw(this, &Engine::onMouseMiddleScrolled);
		input->m_onMouseLeftUp.AddRaw(this, &Engine::onLeftMouseUp);
		input->m_onMouseMiddleUp.AddRaw(this, &Engine::onMiddleMouseUp);

		// Menu
		input->m_menuActionPressed.AddRaw(this, &Engine::onMenuActionPressed);
	}

	LOG_INFO("Renderer constructed.")
	return true;
}

bool Engine::shutdown()
{
	LOG_INFO("Shutting down engine.")
	m_isRunning = false;
	return true;
}

void Engine::tick()
{
	const timePoint endTime = PTimer::now();
	m_deltaTime = std::chrono::duration_cast<durationMs>(endTime - m_startTime).count();
	m_startTime = PTimer::now();

	// Update camera movement
	if (const IInputHandler* input = Win32InputHandler::getInstance())
	{
		// Update camera position
		Camera* camera = getViewportCamera();
		const vec2f deltaMouseCursor = input->getDeltaCursorPosition();

		// Orbit
		if (input->isMouseDown(EMouseButtonType::Left) && input->isAltDown())
		{
			camera->orbit(deltaMouseCursor.X, deltaMouseCursor.Y);
		}

		// Pan
		if (input->isMouseDown(EMouseButtonType::Middle) && input->isAltDown())
		{
			camera->pan(deltaMouseCursor.X, deltaMouseCursor.Y);
		}

		// Zoom
		if (input->isMouseDown(EMouseButtonType::Right) && input->isAltDown())
		{
			camera->zoom(deltaMouseCursor.Y);
		}
	}

	// Tick every object
	getViewportCamera()->update(m_deltaTime);

	// Format debug text
	getViewport()->formatDebugText();
}

void Engine::openFile(const std::string& fileName)
{
	m_meshes.clear();
	const auto objMesh = std::make_shared<Mesh>();
	ObjImporter::import(fileName, objMesh.get());
	objMesh->processTriangles();
	m_meshes.push_back(objMesh);
}

void Engine::onKeyPressed(const EKey keyCode) const
{
	switch (keyCode)
	{
	case EKey::T:
		{
			getViewport()->toggleShowDebugText();
			break;
		}
	case EKey::F:
		{
			getViewport()->resetView();
			break;
		}
	case EKey::F1:
		{
			m_renderer->m_settings.toggleRenderFlag(ERenderFlag::Wireframe);
			break;
		}
	case EKey::F2:
		{
			m_renderer->m_settings.toggleRenderFlag(ERenderFlag::Shaded);
			break;
		}
	case EKey::F3:
		{
			m_renderer->m_settings.toggleRenderFlag(ERenderFlag::Depth);
			break;
		}
	case EKey::F4:
		{
			m_renderer->m_settings.toggleRenderFlag(ERenderFlag::Normals);
			break;
		}
	default:
		break;
	}
}

void Engine::onLeftMouseUp(const vec2f& cursorPosition) const
{
	Camera* camera = getViewportCamera();
	camera->m_sphericalDelta.Phi = 0.0f;
	camera->m_sphericalDelta.Theta = 0.0f;
}

void Engine::onMiddleMouseUp(const vec2f& cursorPosition) const
{
	Camera* camera = getViewportCamera();
	camera->m_panOffset = 0;
}

void Engine::onMenuActionPressed(const EMenuAction actionId)
{
	const Application* app = Application::getInstance();
	IPlatform* platform = app->getPlatform();
	switch (actionId)
	{
	case EMenuAction::Open:
		onOpenPressed();
		break;
	case EMenuAction::Quit:
		m_isRunning = false;
		break;
	case EMenuAction::Wireframe:
		platform->setMenuItemChecked(EMenuAction::Wireframe,
		                             m_renderer->m_settings.toggleRenderFlag(ERenderFlag::Wireframe));
		break;
	case EMenuAction::Shaded:
		platform->setMenuItemChecked(EMenuAction::Shaded, m_renderer->m_settings.toggleRenderFlag(ERenderFlag::Shaded));
		break;
	case EMenuAction::Depth:
		platform->setMenuItemChecked(EMenuAction::Depth, m_renderer->m_settings.toggleRenderFlag(ERenderFlag::Depth));
		break;
	case EMenuAction::Normals:
		platform->setMenuItemChecked(EMenuAction::Normals,
		                             m_renderer->m_settings.toggleRenderFlag(ERenderFlag::Normals));
	}
}

void Engine::onOpenPressed()
{
	Application* app = Application::getInstance();
	IPlatform* platform = app->getPlatform();
	std::string fileName;
	if (platform->getFileDialog(fileName))
	{
		// Load model
		openFile(fileName);
	}
}

void Engine::onMouseMiddleScrolled(const float delta) const
{
	Camera* camera = getViewportCamera();
	camera->setFov(camera->m_fov + (delta));
}
