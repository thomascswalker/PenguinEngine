#include <iostream>

#include <Framework/Application.h>

#include "Framework/Core/ErrorCodes.h"

#if defined(_WIN32) || defined(_WIN64)

// Windows entry point
#include "Framework/Platforms/WindowsPlatform.h"
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    PApplication* App = PApplication::GetInstance();
    App->Init<PWindowsPlatform>(hInstance);
    const int ExitCode = App->Run();
    delete App;

    if (ExitCode != Success)
    {
        LOG_ERROR("Application failed with error {}", ExitCode)
    }
    
    return ExitCode;
}

#elif __APPLE__

// MacOS entry point
int main(int argc, char* argv[])
{
    return 0;
}

#elif __linux__

// Linux entry point
int main(int argc, char* argv[])
{
    return 0;
}

#else
error "Unknown compiler"
#endif
