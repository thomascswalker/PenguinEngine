#include <Framework/Application.h>

#include "Framework/Core/Logging.h"

PApplication* PApplication::GetInstance()
{
    static auto Instance = new PApplication();
    return Instance;
}

int PApplication::Run() const
{
    // Initialize the platform
    Platform->Create();
    if (!Platform->IsInitialized())
    {
        return 1; // App run failure
    }

    // Attempt to display the window
    if (Platform->Show() != 0)
    {
        return 2; // Show failure
    }

    // Run the main loop
    return Platform->Loop();
}
