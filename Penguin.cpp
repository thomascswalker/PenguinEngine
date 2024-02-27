#include <iostream>

#include <Framework/Application.h>

#if defined(_WIN32) || defined(_WIN64)

// Windows entry point
#include "Framework/Platforms/WindowsPlatform.h"
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    LOG_INFO("Starting engine.")
    PApplication* App = PApplication::GetInstance();
    App->Init<PWindowsPlatform>(hInstance);
    const int Result = App->Run();
    LOG_INFO("Shutting down engine.")
    delete App;
    return Result;
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
