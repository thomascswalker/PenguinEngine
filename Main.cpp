#include <iostream>

#include "Framework/Application.h"
#include "Framework/Core/ErrorCodes.h"

#if defined(_WIN32) || defined(_WIN64)

// Windows entry point
#include "Framework/Platforms/Win32Platform.h"

int32 WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int32 nShowCmd)
{
	PApplication* app = PApplication::getInstance();
	app->Init<PWin32Platform>(hInstance);
	const int32 exitCode = app->run();
	delete app;

	if (exitCode != Success)
	{
		const auto errorMsgs = Logging::Logger::GetInstance()->GetMessages(Logging::ELogLevel::Error);
		std::string msg = "Application failed with error(s):\n\n";
		for (const auto& errorMsg : errorMsgs)
		{
			msg += errorMsg + '\n';
		}
		const std::wstring wMsg(msg.begin(), msg.end());
		MessageBox(nullptr, wMsg.c_str(), L"Error", MB_OK | MB_ICONERROR);
	}

	return exitCode;
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
