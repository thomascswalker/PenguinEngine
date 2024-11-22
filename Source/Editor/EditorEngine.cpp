
#include "EditorEngine.h"
#include "Renderer/UI/Widget.h"
#include "Editor.h"

Engine* EditorEngine::create()
{
	g_editor = new EditorEngine();
	return g_editor;
}

bool EditorEngine::initialize(IApplication* app)
{
	// Initialize the editor
	m_application = app;
	constructUI();
	return true;
}

bool EditorEngine::shutdown()
{
	return true;
}

void EditorEngine::tick(float deltaTime) {}

void EditorEngine::constructUI()
{
	createMainWindow();
}

void EditorEngine::createMainWindow()
{
	// Create the main window
	WindowDescription mainWindowDesc;
	mainWindowDesc.width = 640;
	mainWindowDesc.height = 480;
	mainWindowDesc.title = "Penguin Engine";
	mainWindowDesc.backgroundColor = Color::gray();
	m_application->createWindow(mainWindowDesc, nullptr);
	m_mainWindow = m_application->getMainWindow();

	// Create a new canvas and layout, setting the parent widget of the layout
	// to the canvas we're making
	auto	mainCanvas = m_mainWindow->getCanvas();
	Layout* mainLayout = mainCanvas->getLayout();
	mainLayout->setOrientation(Vertical);

	m_mainMenu = std::make_shared<Panel>();
	m_mainMenu->getLayout()->setOrientation(Horizontal);
	m_mainMenu->setVerticalResizeMode(Fixed);
	m_mainMenu->setFixedHeight(25);
	mainCanvas->addChild(m_mainMenu);

	m_exitButton = std::make_shared<Button>("Exit");
	m_exitButton->setFixedHeight(20);
	m_exitButton->setFixedWidth(100);
	m_exitButton->setHorizontalResizeMode(Fixed);
	m_exitButton->setVerticalResizeMode(Fixed);
	m_exitButton->onClicked.addRaw(m_application, &IApplication::exit);
	m_mainMenu->addChild(m_exitButton);

	m_newWindowButton = std::make_shared<Button>("New Window");
	m_newWindowButton->setFixedHeight(20);
	m_newWindowButton->setFixedWidth(100);
	m_newWindowButton->setHorizontalResizeMode(Fixed);
	m_newWindowButton->setVerticalResizeMode(Fixed);
	m_newWindowButton->onClicked.addRaw(this, &EditorEngine::createNewWindow);
	m_mainMenu->addChild(m_newWindowButton);

	m_tempButton = std::make_shared<Button>("Expanding");
	m_tempButton->setFixedHeight(20);
	m_tempButton->setVerticalResizeMode(Fixed);
	m_mainMenu->addChild(m_tempButton);

	m_viewportCanvas = std::make_shared<Canvas>();
	mainCanvas->addChild(m_viewportCanvas);

	m_viewportWidget = std::make_shared<ViewportWidget>();
	//m_viewportCanvas->addChild(m_viewportWidget);
}

void EditorEngine::createNewWindow()
{
	m_newWindow = m_application->createWindow(m_mainWindow, std::string("New Window"), vec2i(200,300), 0);
}
