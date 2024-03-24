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
    if (IInputHandler* Input = PWin32InputHandler::GetInstance())
    {
        // Update camera position
        PCamera* Camera = GetViewportCamera();
        const float ScaledCameraSpeed = CameraSpeed * CameraSpeedMultiplier * DeltaTime;

        FVector3 DeltaTranslation;
        if (Input->IsKeyDown('W')) { DeltaTranslation.Z = ScaledCameraSpeed; } // Forward
        if (Input->IsKeyDown('S')) { DeltaTranslation.Z = -ScaledCameraSpeed; } // Backward
        if (Input->IsKeyDown('D')) { DeltaTranslation.X = ScaledCameraSpeed; } // Right
        if (Input->IsKeyDown('A')) { DeltaTranslation.X = -ScaledCameraSpeed; } // Left
        if (Input->IsKeyDown('E')) { DeltaTranslation.Y = ScaledCameraSpeed; } // Up
        if (Input->IsKeyDown('Q')) { DeltaTranslation.Y = -ScaledCameraSpeed; } // Down

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
    if (auto Mesh = PMesh::CreateTeapot(4))
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
