#pragma once
#include "Framework/Core/Logging.h"

class PEngine
{
public:
    static PEngine* Instance;
    static PEngine* GetInstance();

    bool Startup()
    {
        LOG_INFO("Starting up engine.")
        return true;
    }
    
    bool Shutdown()
    {
        LOG_INFO("Shutting down engine.")
        return true;
    }

    void Tick(float DeltaTime);
};
