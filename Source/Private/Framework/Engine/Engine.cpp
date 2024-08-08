#include <Framework/Engine/Engine.h>
#include <Framework/Engine/Mesh.h>
#include "Framework/Engine/Timer.h"
#include "Framework/Input/InputHandler.h"
#include "Framework/Application.h"
#include "Framework/Importers/MeshImporter.h"

#include "Framework/Platforms/PlatformInterface.h"

PEngine* PEngine::Instance = GetInstance();

PEngine* PEngine::GetInstance()
{
	if (Instance == nullptr)
	{
		Instance = new PEngine();
	}
	return Instance;
}


bool PEngine::Startup(uint32 InWidth, uint32 InHeight)
{
	LOG_INFO("Starting up engine.")

	Renderer = std::make_shared<PRenderer>(InWidth, InHeight);
	bRunning = true;

	// Track starting time
	StartTime = PTimer::Now();

	// Bind input events
	if (IInputHandler* Input = PWin32InputHandler::GetInstance())
	{
		// Keyboard
		Input->KeyPressed.AddRaw(this, &PEngine::OnKeyPressed);

		// Mouse
		Input->MouseMiddleScrolled.AddRaw(this, &PEngine::OnMouseMiddleScrolled);
		Input->MouseLeftUp.AddRaw(this, &PEngine::OnLeftMouseUp);
		Input->MouseMiddleUp.AddRaw(this, &PEngine::OnMiddleMouseUp);

		// Menu
		Input->MenuActionPressed.AddRaw(this, &PEngine::OnMenuActionPressed);
	}

	LOG_INFO("Renderer constructed.")
	return true;
}

bool PEngine::Shutdown()
{
	LOG_INFO("Shutting down engine.")
	bRunning = false;
	return true;
}

void PEngine::Tick()
{
	const TimePoint EndTime = PTimer::Now();
	DeltaTime = std::chrono::duration_cast<DurationMs>(EndTime - StartTime).count();
	StartTime = PTimer::Now();

	// Update camera movement
	if (IInputHandler* Input = PWin32InputHandler::GetInstance())
	{
		// Update camera position
		PCamera* Camera = GetViewportCamera();
		FVector2 DeltaMouseCursor = Input->GetDeltaCursorPosition();

		// Orbit
		if (Input->IsMouseDown(EMouseButtonType::Left) && Input->IsAltDown())
		{
			Camera->Orbit(DeltaMouseCursor.X, DeltaMouseCursor.Y);
		}

		// Pan
		if (Input->IsMouseDown(EMouseButtonType::Middle) && Input->IsAltDown())
		{
			Camera->Pan(DeltaMouseCursor.X, DeltaMouseCursor.Y);
		}

		// Zoom
		if (Input->IsMouseDown(EMouseButtonType::Right) && Input->IsAltDown())
		{
			Camera->Zoom(DeltaMouseCursor.Y);
		}
	}

	// Tick every object
	GetViewportCamera()->Update(DeltaTime);

	// Format debug text
	GetViewport()->FormatDebugText();
}

void PEngine::OpenFile(const std::string& FileName)
{
	Meshes.clear();
	auto ObjMesh = std::make_shared<PMesh>();
	ObjImporter::Import(FileName, ObjMesh.get());
	ObjMesh->ProcessTriangles();
	Meshes.push_back(ObjMesh);
}

void PEngine::OnKeyPressed(EKey KeyCode)
{
	switch (KeyCode)
	{
		case EKey::T :
		{
			GetViewport()->ToggleShowDebugText();
			break;
		}
		case EKey::F :
		{
			GetViewport()->ResetView();
			break;
		}
		case EKey::F1 :
		{
			Renderer->Settings.ToggleRenderFlag(ERenderFlag::Wireframe);
			break;
		}
		case EKey::F2 :
		{
			Renderer->Settings.ToggleRenderFlag(ERenderFlag::Shaded);
			break;
		}
		case EKey::F3 :
		{
			Renderer->Settings.ToggleRenderFlag(ERenderFlag::Depth);
			break;
		}
		case EKey::F4 :
		{
			Renderer->Settings.ToggleRenderFlag(ERenderFlag::Normals);
			break;
		}
		default :
			break;
	}
}

void PEngine::OnLeftMouseUp(const FVector2& CursorPosition) const
{
	PCamera* Camera = GetViewportCamera();
	Camera->SphericalDelta.Phi = 0.0f;
	Camera->SphericalDelta.Theta = 0.0f;
}
void PEngine::OnMiddleMouseUp(const FVector2& CursorPosition) const
{
	PCamera* Camera = GetViewportCamera();
	Camera->PanOffset = 0;
}

void PEngine::OnMenuActionPressed(EMenuAction ActionId)
{
	PApplication* App = PApplication::GetInstance();
	IPlatform* Platform = App->GetPlatform();
	switch (ActionId)
	{
		case EMenuAction::Open :
			OnOpenPressed();
			break;
		case EMenuAction::Quit :
			bRunning = false;
			break;
		case EMenuAction::Wireframe :
			Platform->SetMenuItemChecked(EMenuAction::Wireframe, Renderer->Settings.ToggleRenderFlag(ERenderFlag::Wireframe));
			break;
		case EMenuAction::Shaded :
			Platform->SetMenuItemChecked(EMenuAction::Shaded, Renderer->Settings.ToggleRenderFlag(ERenderFlag::Shaded));
			break;
		case EMenuAction::Depth :
			Platform->SetMenuItemChecked(EMenuAction::Depth, Renderer->Settings.ToggleRenderFlag(ERenderFlag::Depth));
			break;
		case EMenuAction::Normals :
			Platform->SetMenuItemChecked(EMenuAction::Normals, Renderer->Settings.ToggleRenderFlag(ERenderFlag::Normals));
			break;
		case EMenuAction::Multithreaded :
			Platform->SetMenuItemChecked(EMenuAction::Multithreaded, Renderer->Settings.ToggleMultithreaded());
			break;
	}
}
void PEngine::OnOpenPressed()
{
	PApplication* App = PApplication::GetInstance();
	IPlatform* Platform = App->GetPlatform();
	std::string FileName;
	if (Platform->GetFileDialog(FileName))
	{
		// Load model
		OpenFile(FileName);
	}
}

void PEngine::OnMouseMiddleScrolled(float Delta) const
{
	PCamera* Camera = GetViewportCamera();
	Camera->SetFov(Camera->Fov + (Delta));
}
