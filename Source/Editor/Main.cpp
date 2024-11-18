#include <iostream>

#define NOMINMAX

// Launch engine as editor
#define WITH_EDITOR

#include <memory>

#include "Renderer/UI/Widget.h"
#include "Core/Logging.h"
#include "EditorEngine.h"

#if defined(_WIN32) || defined(_WIN64)

	// Windows entry point
	#include "Platforms/Windows/Win32.h"

int32 WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	// Initialize the application with Win32
	g_hInstance = hInstance;

	// Create a new app
	auto app = Win32Application::create(hInstance);
	auto engine = EditorEngine::create();
	g_engine = g_editor;

	app->initialize(engine);
	engine->initialize(app);

	// Run
	int32 exitCode = app->exec();

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
