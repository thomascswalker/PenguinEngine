#include <Framework/Engine/Engine.h>
#include <Framework/Engine/Mesh.h>
#include "Framework/Engine/Timer.h"
#include "Framework/Input/InputHandler.h"
#include "Framework/Application.h"
#include "Framework/Importers/MeshImporter.h"

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
        // Any key press
        Input->KeyPressed.AddRaw(this, &PEngine::OnKeyPressed);

        // Adjust FOV
        Input->MouseMiddleScrolled.AddRaw(this, &PEngine::OnMouseMiddleScrolled);
        Input->MouseLeftUp.AddRaw(this, &PEngine::OnLeftMouseUp);
        Input->MouseMiddleUp.AddRaw(this, &PEngine::OnMiddleMouseUp);
    }

    // Load all geometry into the scene
    LoadSceneGeometry();

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

void PEngine::LoadSceneGeometry()
{
    auto ObjMesh = std::make_shared<PMesh>();
    ObjImporter::Import("C:\\Users\\thoma\\OneDrive\\Documents\\GitHub\\p-engine\\Examples\\Bunny.obj", ObjMesh.get());
    Meshes.emplace_back(ObjMesh);
}

void PEngine::OnKeyPressed(EKey KeyCode)
{
    switch (KeyCode)
    {
    case EKey::Escape :
        {
            bRunning = false;
            break;
        }
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
            Renderer->Settings.ToggleRenderFlag(ERenderFlags::Wireframe);
            break;
        }
    case EKey::F2 :
        {
            Renderer->Settings.ToggleRenderFlag(ERenderFlags::Shaded);
            break;
        }
    case EKey::F3 :
        {
            Renderer->Settings.ToggleRenderFlag(ERenderFlags::Depth);
            break;
        }
    case EKey::F4 :
        {
            Renderer->Settings.ToggleRenderFlag(ERenderFlags::Normals);
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

void PEngine::OnMouseMiddleScrolled(float Delta) const
{
    PCamera* Camera = GetViewportCamera();
    Camera->SetFov(Camera->Fov + (Delta));
}
