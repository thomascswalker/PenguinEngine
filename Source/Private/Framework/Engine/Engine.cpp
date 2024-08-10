#include <Framework/Engine/Engine.h>
#include <Framework/Engine/Mesh.h>
#include "Framework/Engine/Timer.h"
#include "Framework/Input/InputHandler.h"
#include "Framework/Application.h"
#include "Framework/Importers/MeshImporter.h"

#include "Framework/Platforms/PlatformInterface.h"

PEngine* PEngine::m_instance = getInstance();

PEngine* PEngine::getInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new PEngine();
	}
	return m_instance;
}


bool PEngine::startup(uint32 inWidth, uint32 inHeight)
{
	LOG_INFO("Starting up engine.")

	m_renderer = std::make_shared<PRenderer>(inWidth, inHeight);
	m_isRunning = true;

	// Track starting time
	m_startTime = PTimer::Now();

	// Bind input events
	if (IInputHandler* input = PWin32InputHandler::GetInstance())
	{
		// Keyboard
		input->KeyPressed.AddRaw(this, &PEngine::onKeyPressed);

		// Mouse
		input->MouseMiddleScrolled.AddRaw(this, &PEngine::onMouseMiddleScrolled);
		input->MouseLeftUp.AddRaw(this, &PEngine::onLeftMouseUp);
		input->MouseMiddleUp.AddRaw(this, &PEngine::onMiddleMouseUp);

		// Menu
		input->MenuActionPressed.AddRaw(this, &PEngine::onMenuActionPressed);
	}

	LOG_INFO("Renderer constructed.")
	return true;
}

bool PEngine::shutdown()
{
	LOG_INFO("Shutting down engine.")
	m_isRunning = false;
	return true;
}

void PEngine::tick()
{
	const TimePoint endTime = PTimer::Now();
	m_deltaTime = std::chrono::duration_cast<DurationMs>(endTime - m_startTime).count();
	m_startTime = PTimer::Now();

	// Update camera movement
	if (const IInputHandler* input = PWin32InputHandler::GetInstance())
	{
		// Update camera position
		PCamera* camera = getViewportCamera();
		const FVector2 deltaMouseCursor = input->GetDeltaCursorPosition();

		// Orbit
		if (input->IsMouseDown(EMouseButtonType::Left) && input->IsAltDown())
		{
			camera->orbit(deltaMouseCursor.X, deltaMouseCursor.Y);
		}

		// Pan
		if (input->IsMouseDown(EMouseButtonType::Middle) && input->IsAltDown())
		{
			camera->pan(deltaMouseCursor.X, deltaMouseCursor.Y);
		}

		// Zoom
		if (input->IsMouseDown(EMouseButtonType::Right) && input->IsAltDown())
		{
			camera->zoom(deltaMouseCursor.Y);
		}
	}

	// Tick every object
	getViewportCamera()->Update(m_deltaTime);

	// Format debug text
	getViewport()->formatDebugText();
}

void PEngine::openFile(const std::string& fileName)
{
	m_meshes.clear();
	const auto objMesh = std::make_shared<PMesh>();
	ObjImporter::Import(fileName, objMesh.get());
	objMesh->ProcessTriangles();
	m_meshes.push_back(objMesh);
}

void PEngine::onKeyPressed(const EKey keyCode) const
{
	switch (keyCode)
	{
	case EKey::T:
		{
			getViewport()->ToggleShowDebugText();
			break;
		}
	case EKey::F:
		{
			getViewport()->ResetView();
			break;
		}
	case EKey::F1:
		{
			m_renderer->m_settings.ToggleRenderFlag(ERenderFlag::Wireframe);
			break;
		}
	case EKey::F2:
		{
			m_renderer->m_settings.ToggleRenderFlag(ERenderFlag::Shaded);
			break;
		}
	case EKey::F3:
		{
			m_renderer->m_settings.ToggleRenderFlag(ERenderFlag::Depth);
			break;
		}
	case EKey::F4:
		{
			m_renderer->m_settings.ToggleRenderFlag(ERenderFlag::Normals);
			break;
		}
	default:
		break;
	}
}

void PEngine::onLeftMouseUp(const FVector2& cursorPosition) const
{
	PCamera* camera = getViewportCamera();
	camera->m_sphericalDelta.Phi = 0.0f;
	camera->m_sphericalDelta.Theta = 0.0f;
}

void PEngine::onMiddleMouseUp(const FVector2& cursorPosition) const
{
	PCamera* camera = getViewportCamera();
	camera->m_panOffset = 0;
}

void PEngine::onMenuActionPressed(const EMenuAction actionId)
{
	const PApplication* app = PApplication::getInstance();
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
		                             m_renderer->m_settings.ToggleRenderFlag(ERenderFlag::Wireframe));
		break;
	case EMenuAction::Shaded:
		platform->setMenuItemChecked(EMenuAction::Shaded, m_renderer->m_settings.ToggleRenderFlag(ERenderFlag::Shaded));
		break;
	case EMenuAction::Depth:
		platform->setMenuItemChecked(EMenuAction::Depth, m_renderer->m_settings.ToggleRenderFlag(ERenderFlag::Depth));
		break;
	case EMenuAction::Normals:
		platform->setMenuItemChecked(EMenuAction::Normals,
		                             m_renderer->m_settings.ToggleRenderFlag(ERenderFlag::Normals));
	}
}

void PEngine::onOpenPressed()
{
	PApplication* app = PApplication::getInstance();
	IPlatform* platform = app->getPlatform();
	std::string fileName;
	if (platform->getFileDialog(fileName))
	{
		// Load model
		openFile(fileName);
	}
}

void PEngine::onMouseMiddleScrolled(const float delta) const
{
	PCamera* camera = getViewportCamera();
	camera->setFov(camera->m_fov + (delta));
}
