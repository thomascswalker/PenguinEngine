#include "Engine/Engine.h"

#include "Application.h"
#include "ObjectManager.h"
#include "Actors/StaticMeshActor.h"

#include "Engine/Timer.h"
#include "Importers/MeshImporter.h"
#include "Importers/TextureImporter.h"
#include "Input/InputHandler.h"
#include "Platforms/Generic/PlatformInterface.h"
#include "Engine/Mesh.h"
#include "Renderer/UI/Widget.h"
#include "Renderer/Font.h"

using namespace WidgetManager;

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

	m_viewport = std::make_shared<Viewport>(inWidth, inHeight);
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
		input->m_onMouseLeftDown.addRaw(this, &Engine::onLeftMouseDown);
		input->m_onMouseLeftUp.addRaw(this, &Engine::onLeftMouseUp);
		input->m_onMouseMiddleUp.addRaw(this, &Engine::onMiddleMouseUp);
		input->m_onMouseMoved.addRaw(this, &Engine::onMouseMoved);
	}

	// Load fonts
	g_fontDatabase->init();
	LOG_INFO("Initialized font database.")

	// Construct UI
	constructUI();
	LOG_INFO("Constructed UI.")

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
	m_deltaTime = std::chrono::duration_cast<DurationMs>(endTime - m_startTime).count();
	m_startTime = PTimer::now();

	// Update camera movement
	if (const IInputHandler* input = IInputHandler::getInstance())
	{
		// Update camera position
		Camera*		camera = getViewportCamera();
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
	}

	// Tick every object
	auto tickables = g_objectManager.getTickables();
	for (auto tickable : tickables)
	{
		tickable->update(m_deltaTime);
	}

	// Update UI Widgets
	Widget* root = WidgetManager::g_rootWidget;
	WidgetManager::layoutWidget(root, vec2i{ m_viewport->getWidth(), m_viewport->getHeight() }, recti{ { 0, 0 }, m_viewport->getSize() });
}

void Engine::constructUI()
{
	auto canvas = constructWidget<Canvas>("Root");
	canvas->setLayoutMode(ELayoutMode::Vertical);
	canvas->setWidth(m_viewport->getWidth());
	canvas->setHeight(m_viewport->getHeight());
	g_rootWidget = canvas;

	auto menuPanel = constructWidget<Panel>("Menu");
	menuPanel->setLayoutMode(ELayoutMode::Horizontal);
	menuPanel->setVerticalResizeMode(EResizeMode::Fixed);
	menuPanel->setHorizontalResizeMode(EResizeMode::Expanding);
	menuPanel->setFixedHeight(40);
	canvas->addChild(menuPanel);

	auto exitButton = constructWidget<Button>("Exit");
	exitButton->m_onClicked.addRaw(this, &Engine::exit);
	exitButton->setText("X");
	exitButton->setTextColor(UIColors::White);
	exitButton->setHorizontalResizeMode(EResizeMode::Fixed);
	exitButton->setFixedWidth(50);
	menuPanel->addChild(exitButton);

	auto viewportCanvas = constructWidget<Canvas>("Viewport");
	viewportCanvas->setVerticalResizeMode(EResizeMode::Expanding);
	canvas->addChild(viewportCanvas);

	//auto toolPanel = constructWidget<Panel>("Tools");
	//toolPanel->setLayoutMode(ELayoutMode::Vertical);
	//toolPanel->setHorizontalResizeMode(EResizeMode::Fixed);
	//toolPanel->setFixedWidth(140);
	//viewportCanvas->addChild(toolPanel);

	//auto meshButton = constructWidget<Button>("ImportMesh");
	//meshButton->m_onClicked.addRaw(this, &Engine::loadMesh);
	////meshButton->setText("Load Model");
	//meshButton->setVerticalResizeMode(EResizeMode::Fixed);
	//toolPanel->addChild(meshButton);

	//auto texButton = constructWidget<Button>("ImportTexture");
	//texButton->m_onClicked.addRaw(this, &Engine::loadTexture);
	////texButton->setText("Load Texture");
	//texButton->setVerticalResizeMode(EResizeMode::Fixed);
	//toolPanel->addChild(texButton);

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
		default:
			break;
	}
}

void Engine::onLeftMouseDown(MouseData& mouse) const
{
	WidgetManager::updateWidgets(mouse);
}

void Engine::onLeftMouseUp(MouseData& mouse) const
{
	WidgetManager::updateWidgets(mouse);

	Camera* camera = getViewportCamera();
	camera->m_deltaRotation.phi = 0.0f;
	camera->m_deltaRotation.theta = 0.0f;
}

void Engine::onMiddleMouseUp(MouseData& mouse) const
{
	Camera* camera = getViewportCamera();
	camera->m_deltaTranslation = 0;
}

void Engine::onMouseMoved(MouseData& mouse) const
{
	WidgetManager::updateWidgets(mouse);
}

void Engine::onMouseMiddleScrolled(MouseData& mouse) const
{
	Camera* camera = getViewportCamera();
	camera->setFov(camera->m_fov + (mouse.middleDelta));
}

void Engine::loadMesh() const
{
	Application* app = Application::getInstance();
	IPlatform*	 platform = app->getPlatform();
	std::string	 fileName;
	if (platform->getFileDialog(fileName, "obj"))
	{
		// Load model
		auto mesh = new Mesh();
		ObjImporter::import(fileName, mesh);
		mesh->processTriangles();
		g_meshes.push_back(std::move(mesh));
		LOG_INFO("Loaded model {}.", fileName)

		if (StaticMeshActor* staticMeshActor = g_objectManager.createObject<StaticMeshActor>())
		{
			// Set the mesh in the actor
			staticMeshActor->setMesh(g_meshes.back());
			// Bind the mesh to the render pipeline
			m_viewport->getRHI()->addRenderable(staticMeshActor);
		}
		else
		{
			LOG_ERROR("Failed to construct StaticMeshActor.")
		}
	}
}

void Engine::loadTexture() const
{
	Application* app = Application::getInstance();
	IPlatform*	 platform = app->getPlatform();
	std::string	 fileName;
	if (platform->getFileDialog(fileName, "png"))
	{
		// Load texture
		g_textures.clear();
		const auto texture = std::make_shared<Texture>();
		TextureImporter::import(fileName, texture.get(), ETextureFileFormat::Rgba);
		texture->flipVertical();
		g_textures.emplace_back(texture);
		LOG_INFO("Loaded texture {}.", fileName)

		m_viewport->getRHI()->addTexture(texture.get());
	}
}
