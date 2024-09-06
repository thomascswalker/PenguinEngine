#include "Framework/Engine/Engine.h"

#include "Framework/Application.h"
#include "Framework/Engine/Timer.h"
#include "Framework/Importers/MeshImporter.h"
#include "Framework/Importers/TextureImporter.h"
#include "Framework/Input/InputHandler.h"
#include "Framework/Platforms/PlatformInterface.h"
#include "Framework/Engine/Mesh.h"

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
		LOG_INFO("Setting up input.")
		// Keyboard
		input->m_keyPressed.addRaw(this, &Engine::onKeyPressed);

		// Mouse
		input->m_onMouseMiddleScrolled.addRaw(this, &Engine::onMouseMiddleScrolled);
		input->m_onMouseLeftUp.addRaw(this, &Engine::onLeftMouseUp);
		input->m_onMouseMiddleUp.addRaw(this, &Engine::onMiddleMouseUp);

		// Menu
		input->m_menuActionPressed.addRaw(this, &Engine::onMenuActionPressed);
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
		Camera*		camera = getViewportCamera();
		const vec2f deltaMouseCursor = input->getDeltaCursorPosition();

		// Orbit
		if (input->isMouseDown(EMouseButtonType::Left) && input->isAltDown())
		{
			camera->orbit(deltaMouseCursor.x, deltaMouseCursor.y);
		}

		// Pan
		if (input->isMouseDown(EMouseButtonType::Middle) && input->isAltDown())
		{
			camera->pan(deltaMouseCursor.x, deltaMouseCursor.y);
		}

		// Zoom
		if (input->isMouseDown(EMouseButtonType::Right) && input->isAltDown())
		{
			camera->zoom(deltaMouseCursor.y);
		}
	}

	// Tick every object
	getViewportCamera()->update(m_deltaTime);

	// Format debug text
	getViewport()->formatDebugText();
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
			m_renderer->m_settings.toggleRenderFlag(Wireframe);
			break;
		}
		case EKey::F2:
		{
			m_renderer->m_settings.toggleRenderFlag(Shaded);
			break;
		}
		case EKey::F3:
		{
			m_renderer->m_settings.toggleRenderFlag(Depth);
			break;
		}
		case EKey::F4:
		{
			m_renderer->m_settings.toggleRenderFlag(Normals);
			break;
		}
		default:
			break;
	}
}

void Engine::onLeftMouseUp(const vec2f& cursorPosition) const
{
	Camera* camera = getViewportCamera();
	camera->m_sphericalDelta.phi = 0.0f;
	camera->m_sphericalDelta.theta = 0.0f;
}

void Engine::onMiddleMouseUp(const vec2f& cursorPosition) const
{
	Camera* camera = getViewportCamera();
	camera->m_panOffset = 0;
}

void Engine::onMenuActionPressed(const EMenuAction actionId)
{
	const Application* app = Application::getInstance();
	IPlatform*		   platform = app->getPlatform();
	switch (actionId)
	{
		case EMenuAction::LoadModel:
		{
			onLoadModelPressed();
			break;
		}
		case EMenuAction::LoadTexture:
		{
			onLoadTexturePressed();
			break;
		}
		case EMenuAction::Quit:
		{
			m_isRunning = false;
			break;
		}
		case EMenuAction::Wireframe:
		{
			platform->setMenuItemChecked(EMenuAction::Wireframe, m_renderer->m_settings.toggleRenderFlag(Wireframe));
			break;
		}
		case EMenuAction::Shaded:
		{
			platform->setMenuItemChecked(EMenuAction::Shaded, m_renderer->m_settings.toggleRenderFlag(Shaded));
			break;
		}
		case EMenuAction::Depth:
		{
			platform->setMenuItemChecked(EMenuAction::Depth, m_renderer->m_settings.toggleRenderFlag(Depth));
			break;
		}
		case EMenuAction::Normals:
		{
			platform->setMenuItemChecked(EMenuAction::Normals, m_renderer->m_settings.toggleRenderFlag(Normals));
			break;
		}
	}
}

void Engine::onLoadModelPressed()
{
	Application* app = Application::getInstance();
	IPlatform*	 platform = app->getPlatform();
	std::string	 fileName;
	if (platform->getFileDialog(fileName, "obj"))
	{
		// Load model
		g_meshes.clear();
		const auto mesh = std::make_shared<Mesh>();
		ObjImporter::import(fileName, mesh.get());
		mesh->processTriangles();
		g_meshes.push_back(mesh);
		LOG_INFO("Loaded model {}.", fileName);
	}
}

void Engine::onLoadTexturePressed()
{
	Application* app = Application::getInstance();
	IPlatform*	 platform = app->getPlatform();
	std::string	 fileName;
	if (platform->getFileDialog(fileName, "png"))
	{
		// Load texture
		g_textures.clear();
		const auto texture = std::make_shared<Texture>();
		TextureImporter::import(fileName, texture.get(), ETextureFileFormat::RGBA);
		texture->flipVertical();
		g_textures.emplace_back(texture);
		m_renderer->getShader()->texture = TextureManager::getTexture(0);
		LOG_INFO("Loaded texture {}.", fileName);
	}
}

void Engine::onMouseMiddleScrolled(const float delta) const
{
	Camera* camera = getViewportCamera();
	camera->setFov(camera->m_fov + (delta));
}
