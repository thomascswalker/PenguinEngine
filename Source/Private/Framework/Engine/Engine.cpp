#include <Framework/Engine/Engine.h>
#include <Framework/Engine/Mesh.h>

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

    // Construct a simple triangle mesh
    if (std::shared_ptr<PMesh> TriangleMesh = PMesh::Triangle())
    {
        Meshes.emplace_back(TriangleMesh);
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

void PEngine::Tick(float DeltaTime)
{
    // LOG_INFO("EngineTick: {}", DeltaTime)
}
