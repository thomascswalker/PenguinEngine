#pragma once

#include "Mesh.h"
#include "Timer.h"
#include "Framework/Input/InputHandler.h"
#include "Framework/Renderer/Renderer.h"

class PEngine
{
    std::shared_ptr<PRenderer> Renderer;
    bool bRunning = false;

    TimePoint StartTime;
    float DeltaTime = 0.0f;

    float CameraSpeed = .01f;
    float CameraSpeedMultiplier = 1.0f;

    std::vector<float*> Vertexes;
    std::vector<uint32*> Indexes;

public:
    static PEngine* Instance;
    static PEngine* GetInstance();

    bool Startup(uint32 InWidth, uint32 InHeight);
    bool Shutdown();

    void Tick();

    PRenderer* GetRenderer() const { return Renderer.get(); }
    PViewport* GetViewport() const { return GetRenderer()->GetViewport(); }
    PCamera* GetViewportCamera() const { return GetViewport()->GetCamera(); }
    bool IsRunning() const { return bRunning; }
    void SetRunning(bool bNewRunning) { bRunning = bNewRunning; }

    void LoadSceneGeometry();

    std::vector<std::shared_ptr<PMesh>> Meshes;
    std::vector<std::shared_ptr<PMesh>> GetMeshes() const { return Meshes; }

    constexpr float GetFps() const { return 1000.0f / DeltaTime; }

    void OnLeftMouseDown(const FVector2& CursorPosition) const;
    void OnLeftMouseUp(const FVector2& CursorPosition) const;
    void OnMouseMiddleScrolled(float Delta) const;
    void OnKeyPressed(EKey KeyCode);
};
