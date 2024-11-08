#include <iostream>

#define NOMINMAX

#include "Application.h"

#if defined(_WIN32) || defined(_WIN64)

// Windows entry point
#include "Platforms/Windows/Win32.h"

int32 WINAPI wWinMain(_In_ HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_ LPWSTR lpCmdLine,
                      _In_ int nShowCmd)
{
	// Create a new application
	Application* app = Application::getInstance();

	// Initialize the application with Win32
	app->Init<Win32Platform>(hInstance);

	// Run the application. This encapsulates the entire lifetime of the platform, engine, renderer, etc.
	const int32 exitCode = app->run();

	// Delete the application after it's been run.
	delete app;

	// Return the result of running the application.
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
