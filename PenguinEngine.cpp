#include <iostream>

#include <GLFW/glfw3.h>

#include <Framework/Application.h>
#include "Framework/Core/ErrorCodes.h"

#if defined(_WIN32) || defined(_WIN64)

// Windows entry point
#include "Source\Public\Framework\Platforms\Win32Platform.h"
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    
    
    PApplication* App = PApplication::GetInstance();
    App->Init<PWin32Platform>(hInstance);
    const int ExitCode = App->Run();
    delete App;

    if (ExitCode != Success)
    {
        const auto ErrorMsgs = Logging::Logger::GetInstance()->GetMessages(Logging::ELogLevel::Error);
        std::string Msg = "Application failed with error(s):\n\n";
        for (const auto& EMsg : ErrorMsgs)
        {
            Msg += EMsg + '\n';
        }
        std::wstring WMsg(Msg.begin(), Msg.end());
        MessageBox(nullptr, WMsg.c_str(), L"Error", MB_OK | MB_ICONERROR);
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
