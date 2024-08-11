#include <Framework/Application.h>

#include "Framework/Core/ErrorCodes.h"
#include "Framework/Core/Logging.h"

Application* Application::m_instance = getInstance();

Application* Application::getInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new Application();
	}
	return m_instance;
}

int32 Application::run() const
{
	// Initialize the platform
	if (m_platform->create() != Success)
	{
		return PlatformInitError; // App run failure
	}

	// Attempt to display the window
	if (m_platform->show() != Success)
	{
		return PlatformShowError; // Show failure
	}

	// Run the main loop
	return m_platform->start();
}
