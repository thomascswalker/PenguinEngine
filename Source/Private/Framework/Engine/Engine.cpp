#include <Framework/Engine/Engine.h>

PEngine* PEngine::Instance = GetInstance();

PEngine* PEngine::GetInstance()
{
    if (Instance == nullptr)
    {
        Instance = new PEngine();
    }
    return Instance;
}

void PEngine::Tick(float DeltaTime)
{
    // LOG_INFO("EngineTick: {}", DeltaTime)
}
