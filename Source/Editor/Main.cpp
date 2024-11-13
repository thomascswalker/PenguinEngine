#include <iostream>

#define NOMINMAX

#include <memory>

#include "Core/Logging.h"
#include "Engine/Engine.h"

#if defined(_WIN32) || defined(_WIN64)

	// Windows entry point
	#include "Platforms/Windows/Win32.h"

int32 WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	// Initialize the application with Win32
	g_hInstance = hInstance;
	auto application = Win32Application::create(hInstance);
	application->init();

	// Create the main window
	application->createMainWindow();
	auto mainWindow = application->getMainWindow();

	WindowDescription desc;
	desc.x = 25;
	desc.y = 50;
	desc.width = 200;
	desc.height = 300;
	application->createWindow(desc, mainWindow);

	TimePoint startTime;
	TimePoint endTime;
	while (application->getIsRunning())
	{
		startTime = PTimer::now();
		float deltaTime = std::chrono::duration_cast<DurationMs>(endTime - startTime).count();
		application->tick(deltaTime);
		endTime = PTimer::now();
	}

	return 0;
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
