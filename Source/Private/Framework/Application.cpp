#include <Framework/Application.h>

#include "Framework/Core/ErrorCodes.h"
#include "Framework/Core/Logging.h"

PApplication* PApplication::Instance = GetInstance();

PApplication* PApplication::GetInstance()
{
    if (Instance == nullptr)
    {
        Instance = new PApplication();
    }
    return Instance;
}

int32 PApplication::Run() const
{
    // Initialize the platform
    if (Platform->Create() != Success)
    {
        return PlatformInitError; // App run failure
    }

    // Attempt to display the window
    if (Platform->Show() != Success)
    {
        return PlatformShowError; // Show failure
    }

    // Run the main loop
    return Platform->Start();
}
