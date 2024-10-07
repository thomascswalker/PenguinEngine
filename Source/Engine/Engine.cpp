#include "Engine/Engine.h"

#include "Application.h"
#include "ObjectManager.h"
#include "StaticMeshActor.h"

#include "Engine/Timer.h"
#include "Importers/MeshImporter.h"
#include "Importers/TextureImporter.h"
#include "Input/InputHandler.h"
#include "Platforms/Generic/PlatformInterface.h"
#include "Engine/Mesh.h"

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

	m_viewport  = std::make_shared<Viewport>(inWidth, inHeight);
	m_isRunning = true;

	// Track starting time
	m_startTime = PTimer::now();

	// Bind input events
	if (IInputHandler* input = IInputHandler::getInstance())
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
	const TimePoint endTime = PTimer::now();
	m_deltaTime             = std::chrono::duration_cast<DurationMs>(endTime - m_startTime).count();
	m_startTime             = PTimer::now();

	// Update camera movement
	if (const IInputHandler* input = IInputHandler::getInstance())
	{
		// Update camera position
		Camera* camera               = getViewportCamera();
		const vec2f deltaMouseCursor = input->getDeltaCursorPosition();

		// Orbit
		if (input->isMouseDown(EMouseButtonType::Left) && input->isAltDown())
		{
			camera->orbit(deltaMouseCursor.x * m_deltaTime, deltaMouseCursor.y * m_deltaTime);
		}

		// Pan
		if (input->isMouseDown(EMouseButtonType::Middle) && input->isAltDown())
		{
			camera->pan(deltaMouseCursor.x * m_deltaTime, deltaMouseCursor.y * m_deltaTime);
		}

		// Zoom
		if (input->isMouseDown(EMouseButtonType::Right) && input->isAltDown())
		{
			camera->zoom(deltaMouseCursor.y * m_deltaTime);
		}

		m_viewport->updateSceneCamera();
	}

	// Tick every object
	auto tickables = g_objectManager.getTickables();
	for (auto tickable : tickables)
	{
		tickable->update(m_deltaTime);
	}

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
			m_viewport->m_settings.toggleRenderFlag(Wireframe);
			break;
		}
	case EKey::F2:
		{
			m_viewport->m_settings.toggleRenderFlag(Shaded);
			break;
		}
	case EKey::F3:
		{
			m_viewport->m_settings.toggleRenderFlag(Depth);
			break;
		}
	case EKey::F4:
		{
			m_viewport->m_settings.toggleRenderFlag(Normals);
			break;
		}
	default: break;
	}
}

void Engine::onLeftMouseUp(const vec2f& cursorPosition) const
{
	Camera* camera                = getViewportCamera();
	camera->m_deltaRotation.phi   = 0.0f;
	camera->m_deltaRotation.theta = 0.0f;
}

void Engine::onMiddleMouseUp(const vec2f& cursorPosition) const
{
	Camera* camera             = getViewportCamera();
	camera->m_deltaTranslation = 0;
}

void Engine::onMenuActionPressed(const EMenuAction actionId)
{
	const Application* app = Application::getInstance();
	IPlatform* platform    = app->getPlatform();
	switch (actionId)
	{
	case EMenuAction::LoadModel:
		{
			loadMesh();
			break;
		}
	case EMenuAction::LoadTexture:
		{
			loadTexture();
			break;
		}
	case EMenuAction::Quit:
		{
			m_isRunning = false;
			break;
		}
	case EMenuAction::Wireframe:
		{
			platform->setMenuItemChecked(EMenuAction::Wireframe, m_viewport->m_settings.toggleRenderFlag(Wireframe));
			break;
		}
	case EMenuAction::Shaded:
		{
			platform->setMenuItemChecked(EMenuAction::Shaded, m_viewport->m_settings.toggleRenderFlag(Shaded));
			break;
		}
	case EMenuAction::Depth:
		{
			platform->setMenuItemChecked(EMenuAction::Depth, m_viewport->m_settings.toggleRenderFlag(Depth));
			break;
		}
	case EMenuAction::Normals:
		{
			platform->setMenuItemChecked(EMenuAction::Normals, m_viewport->m_settings.toggleRenderFlag(Normals));
			break;
		}
	case EMenuAction::VertexNormals:
		{
			break;
		}
	}
}

void Engine::loadMesh() const
{
	Application* app    = Application::getInstance();
	IPlatform* platform = app->getPlatform();
	std::string fileName;
	if (platform->getFileDialog(fileName, "obj"))
	{
		// Load model
		Mesh* mesh = new Mesh();
		ObjImporter::import(fileName, mesh);
		mesh->processTriangles();
		g_meshes.push_back(std::move(mesh));
		LOG_INFO("Loaded model {}.", fileName)

		if (StaticMeshActor* staticMeshActor = g_objectManager.createObject<StaticMeshActor>())
		{
			staticMeshActor->setMesh(g_meshes.back());
		}
		else
		{
			LOG_ERROR("Failed to construct StaticMeshActor.")
		}

		auto renderables = g_objectManager.getRenderables();
		m_viewport->updateSceneGeometry(renderables);
	}
}

void Engine::loadTexture() const
{
	Application* app    = Application::getInstance();
	IPlatform* platform = app->getPlatform();
	std::string fileName;
	if (platform->getFileDialog(fileName, "png"))
	{
		// Load texture
		g_textures.clear();
		const auto texture = std::make_shared<Texture>();
		TextureImporter::import(fileName, texture.get(), ETextureFileFormat::Rgba);
		texture->flipVertical();
		g_textures.emplace_back(texture);
		LOG_INFO("Loaded texture {}.", fileName)
	}
}

void Engine::onMouseMiddleScrolled(const float delta) const
{
	Camera* camera = getViewportCamera();
	camera->setFov(camera->m_fov + (delta));
}
