#pragma once
#include "Mesh.h"
#include "Timer.h"
#include "Framework/Core/Logging.h"
#include "Framework/Renderer/Renderer.h"

class PEngine
{
    std::shared_ptr<PRenderer> Renderer;
    bool bRunning = false;

    TimePoint StartTime;
    float DeltaTime = 0.0f;

public:
    static PEngine* Instance;
    static PEngine* GetInstance();

    bool Startup(uint32 InWidth, uint32 InHeight);
    bool Shutdown();

    void Tick();

    PRenderer* GetRenderer() const { return Renderer.get(); }
    bool IsRunning() const { return bRunning; }
    void SetRunning(bool bNewRunning) { bRunning = bNewRunning; }

    std::vector<std::shared_ptr<PMesh>> Meshes;
    std::vector<std::shared_ptr<PMesh>> GetMeshes() const { return Meshes; }

    constexpr float GetFps() const { return 1000.0f / DeltaTime; }
};
