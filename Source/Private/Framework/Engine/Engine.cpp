#include <Framework/Engine/Engine.h>
#include <Framework/Engine/Mesh.h>

#include "Framework/Engine/Timer.h"

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
    if (auto Mesh = PMesh::CreateSphere(0.5f, 6))
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
}
