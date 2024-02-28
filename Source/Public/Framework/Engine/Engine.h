#pragma once
#include "Mesh.h"
#include "Framework/Core/Logging.h"
#include "Framework/Renderer/Renderer.h"

class PEngine
{
    std::shared_ptr<PRenderer> Renderer;
    bool bRunning = false;

public:
    static PEngine* Instance;
    static PEngine* GetInstance();
    
    bool Startup(uint32 InWidth, uint32 InHeight);

    bool Shutdown();

    void Tick(float DeltaTime);

    PRenderer* GetRenderer() const { return Renderer.get(); }
    bool IsRunning() const { return bRunning; }

    std::vector<std::shared_ptr<PMesh>> Meshes;
};
