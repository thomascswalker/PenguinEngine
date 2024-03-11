#include "Framework/Input/InputHandler.h"

IInputHandler* IInputHandler::GetInstance()
{
#if (defined(_WIN32) || defined(_WIN64))
    return PWin32InputHandler::GetInstance();
#else
    // Other handlers
#endif
}

PWin32InputHandler* PWin32InputHandler::Instance = GetInstance();

PWin32InputHandler* PWin32InputHandler::GetInstance()
{
    if (Instance == nullptr)
    {
        Instance = new PWin32InputHandler();
    }
    return Instance;
}
