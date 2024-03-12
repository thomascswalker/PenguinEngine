#include <Framework/Engine/Engine.h>
#include <Framework/Engine/Mesh.h>
#include "Framework/Engine/Timer.h"
#include "Framework/Input/InputHandler.h"
#include "Framework/Application.h"

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
        const float ScaledCameraSpeed = CameraSpeed * CameraSpeedMultiplier * DeltaTime;

        FVector3 DeltaTranslation;
        if (Input->IsKeyDown('W')) { DeltaTranslation.Z = ScaledCameraSpeed; } // Forward
        if (Input->IsKeyDown('S')) { DeltaTranslation.Z = ScaledCameraSpeed; } // Backward
        if (Input->IsKeyDown('D')) { DeltaTranslation.X = ScaledCameraSpeed; } // Right
        if (Input->IsKeyDown('A')) { DeltaTranslation.X = -ScaledCameraSpeed; } // Left
        if (Input->IsKeyDown('E')) { DeltaTranslation.Y = ScaledCameraSpeed; } // Up
        if (Input->IsKeyDown('Q')) { DeltaTranslation.Y = -ScaledCameraSpeed; } // Down

        // Move in world space
        if (DeltaTranslation != 0)
        {
            Camera->Translate(DeltaTranslation);
        }

        // Calculate rotation amount given the mouse delta
        FVector2 DeltaMouseCursor = Input->GetDeltaCursorPosition() * DeltaTime * 0.00001f;

        // If there's actual movement on either the X or Y axis, move the camera
        if (DeltaMouseCursor != 0)
        {
            Camera->Orbit(DeltaMouseCursor.X, DeltaMouseCursor.Y, FVector3::ZeroVector());
        }
        Input->ResetDeltaCursorPosition();
    }

    // Format debug text
    GetViewport()->FormatDebugText();
}

void PEngine::LoadSceneGeometry()
{
    // Construct a simple triangle mesh
    if (auto Mesh = PMesh::CreatePlane(2.0f))
    {
        Meshes.emplace_back(Mesh);
    }
    if (auto Mesh = PMesh::CreateSphere(1.0f, 16))
    {
        Meshes.emplace_back(Mesh);
    }
}

void PEngine::OnKeyPressed(int32 KeyCode) const
{
    switch (KeyCode)
    {
    case 'T' :
        {
            GetViewport()->ToggleShowDebugText();
            break;
        }
    case 'F' :
        {
            GetViewport()->ResetView();
            break;
        }
    default :
        break;
    }
}

void PEngine::OnMouseMiddleScrolled(float Delta) const
{
    PCamera* Camera = GetViewportCamera();
    Camera->SetFov(Camera->Fov + Delta);
}
