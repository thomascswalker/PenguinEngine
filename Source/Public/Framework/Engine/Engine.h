#pragma once
#include "Framework/Core/Logging.h"
#include "Framework/Renderer/Renderer.h"

class PEngine
{
    std::shared_ptr<PRenderer> Renderer;
    bool bRunning = false;

public:
    static PEngine* Instance;
    static PEngine* GetInstance();
    
    bool Startup(uint32 InWidth, uint32 InHeight)
    {
        LOG_INFO("Starting up engine.")
        
        Renderer = std::make_shared<PRenderer>(InWidth, InHeight);
        bRunning = true;
        LOG_INFO("Renderer constructed.")
        return true;
    }
    
    bool Shutdown()
    {
        LOG_INFO("Shutting down engine.")
        bRunning = false;
        return true;
    }

    void Tick(float DeltaTime);

    std::shared_ptr<PRenderer> GetRenderer() const { return Renderer; }
    bool IsRunning() const { return bRunning; }
};
