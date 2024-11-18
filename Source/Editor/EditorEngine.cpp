
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
	mainLayout->setOrientation(Horizontal);

	m_exitButton = std::make_shared<Button>("Exit");
	m_exitButton->setFixedHeight(20);
	m_exitButton->setVerticalResizeMode(Fixed);
	m_exitButton->onClicked.addRaw(m_application, &IApplication::exit);

	m_newWindowButton = std::make_shared<Button>("New Window");
	m_newWindowButton->setFixedHeight(20);
	m_newWindowButton->setVerticalResizeMode(Fixed);
	m_newWindowButton->onClicked.addRaw(this, &EditorEngine::createNewWindow);

	mainLayout->addWidget(m_exitButton.get());
	mainLayout->addWidget(m_newWindowButton.get());
}

void EditorEngine::createNewWindow()
{
	m_newWindow = m_application->createWindow(m_mainWindow, std::string("New Window"), vec2i(200,300), 0);
}
