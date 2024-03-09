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

    // Construct a simple triangle mesh
    if (auto Mesh = PMesh::CreatePlane(2.0f))
    {
        Meshes.emplace_back(Mesh);
    }
    if (auto Mesh = PMesh::CreateSphere(1.0f, 16))
    {
        Meshes.emplace_back(Mesh);
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
        PCamera* Camera = Renderer->GetViewport()->GetCamera();
        const float ScaledCameraSpeed = CameraSpeed * CameraSpeedMultiplier * DeltaTime;

        // if (Input->IsKeyDown('W')) { /* Input->ConsumeKey('W'); */ DeltaTranslation.Z = ScaledCameraSpeed; } // Forward
        // if (Input->IsKeyDown('S')) { /* Input->ConsumeKey('S'); */ DeltaTranslation.Z = -ScaledCameraSpeed; } // Backward
        // if (Input->IsKeyDown('D')) { /* Input->ConsumeKey('D'); */ DeltaTranslation.X = ScaledCameraSpeed; } // Right
        // if (Input->IsKeyDown('A')) { /* Input->ConsumeKey('A'); */ DeltaTranslation.X = -ScaledCameraSpeed; } // Left
        // if (Input->IsKeyDown('E')) { /* Input->ConsumeKey('E'); */ DeltaTranslation.Y = ScaledCameraSpeed; } // Up
        // if (Input->IsKeyDown('Q')) { /* Input->ConsumeKey('Q'); */ DeltaTranslation.Y = -ScaledCameraSpeed; } // Down

        // Calculate rotation amount given the mouse delta
        FVector2 DeltaMouseCursor = Input->GetDeltaCursorPosition() * ScaledCameraSpeed;

        // If there's actual movement on either the X or Y axis, move the camera
        if (DeltaMouseCursor != 0)
        {
            Camera->Orbit(DeltaMouseCursor.X, DeltaMouseCursor.Y);
        }
        Input->ResetDeltaCursorPosition();
    }


    // Format debug text
    Renderer->GetViewport()->FormatDebugText();
}
