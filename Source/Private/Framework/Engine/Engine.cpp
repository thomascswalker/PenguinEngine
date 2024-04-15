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
        Input->KeyPressed.AddRaw(this, &PEngine::OnKeyPressed);
        Input->MouseMiddleScrolled.AddRaw(this, &PEngine::OnMouseMiddleScrolled);
        Input->MouseLeftDown.AddRaw(this, &PEngine::OnLeftMouseDown);
        Input->MouseLeftUp.AddRaw(this, &PEngine::OnLeftMouseUp);
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
    if (const IInputHandler* Input = PWin32InputHandler::GetInstance())
    {
        // Update camera position
        PCamera* Camera = GetViewportCamera();
        const float ScaledCameraSpeed = CameraSpeed * CameraSpeedMultiplier * DeltaTime;

        FVector3 DeltaTranslation;
        if (Input->IsKeyDown(EKey::W)) { DeltaTranslation.Z = ScaledCameraSpeed; } // Forward
        if (Input->IsKeyDown(EKey::S)) { DeltaTranslation.Z = -ScaledCameraSpeed; } // Backward
        if (Input->IsKeyDown(EKey::D)) { DeltaTranslation.X = ScaledCameraSpeed; } // Right
        if (Input->IsKeyDown(EKey::A)) { DeltaTranslation.X = -ScaledCameraSpeed; } // Left
        if (Input->IsKeyDown(EKey::E)) { DeltaTranslation.Y = ScaledCameraSpeed; } // Up
        if (Input->IsKeyDown(EKey::Q)) { DeltaTranslation.Y = -ScaledCameraSpeed; } // Down

        // Move in world space
        if (DeltaTranslation != 0.0f)
        {
            Camera->Translate(DeltaTranslation);
        }

        // Calculate rotation amount given the mouse delta
        if (Input->IsMouseDown(EMouseButtonType::Left))
        {
            FVector2 DeltaMouseCursor = Input->GetCurrentCursorPosition() - Input->GetClickPosition();

            // If there's actual movement on either the X or Y axis, move the camera
            if (DeltaMouseCursor != 0)
            {
                Camera->Orbit(DeltaMouseCursor.X, DeltaMouseCursor.Y); // Swap X and Y
            }
        }
    }

    // Format debug text
    GetViewport()->FormatDebugText();
}

void PEngine::LoadSceneGeometry()
{
    std::shared_ptr<PMesh> Mesh = std::make_shared<PMesh>();
    ObjImporter::Import("C:\\Users\\thoma\\OneDrive\\Documents\\GitHub\\p-engine\\Examples\\Bunny.obj", Mesh.get());
    Meshes.emplace_back(Mesh);
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
            Renderer->ToggleRenderFlag(ERenderFlags::Wireframe);
            break;
        }
    case EKey::F2 :
        {
            Renderer->ToggleRenderFlag(ERenderFlags::Shaded);
            break;
        }
    case EKey::F3 :
        {
            Renderer->ToggleRenderFlag(ERenderFlags::Depth);
            break;
        }
    default :
        break;
    }
}

void PEngine::OnLeftMouseDown(const FVector2& CursorPosition) const
{
    // Store the original transform when a click begins
    PCamera* Camera = GetViewportCamera();
    Camera->OriginalTransform = Camera->GetTransform();
}

void PEngine::OnLeftMouseUp(const FVector2& CursorPosition) const
{
    // Store the original transform when a click begins
    PCamera* Camera = GetViewportCamera();
    Camera->OriginalTransform = Camera->GetTransform();
}

void PEngine::OnMouseMiddleScrolled(float Delta) const
{
    PCamera* Camera = GetViewportCamera();
    Camera->SetFov(Camera->Fov + Delta);
}
