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
    if (auto Mesh = PMesh::CreateCube(0.5f))
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
    TimePoint EndTime = PTimer::Now();
    DeltaTime = std::chrono::duration_cast<DurationMs>(EndTime - StartTime).count();
    StartTime = PTimer::Now();


    // Update camera movement
    IInputHandler* Input = PWin32InputHandler::GetInstance();
    if (Input)
    {
        FVector3 DeltaTranslation;

        // Update camera position
        const float ScaledCameraSpeed = CameraSpeed * CameraSpeedMultiplier * DeltaTime;
        if (Input->IsKeyDown('W')) { DeltaTranslation.Y = ScaledCameraSpeed; } // Forward
        if (Input->IsKeyDown('S')) { DeltaTranslation.Y = -ScaledCameraSpeed; } // Backward
        if (Input->IsKeyDown('D')) { DeltaTranslation.X = ScaledCameraSpeed; } // Right
        if (Input->IsKeyDown('A')) { DeltaTranslation.X = -ScaledCameraSpeed; } // Left
        if (Input->IsKeyDown('E')) { DeltaTranslation.Z = ScaledCameraSpeed; } // Up
        if (Input->IsKeyDown('Q')) { DeltaTranslation.Z = -ScaledCameraSpeed; } // Down

        // Arcball rotation
        Renderer->GetViewport()->GetInfo()->Translation += DeltaTranslation;

        FVector2 DeltaMouseCursor = Input->GetDeltaCursorPosition();
    }

    // Format debug text
    Renderer->GetViewport()->FormatDebugText();
}
