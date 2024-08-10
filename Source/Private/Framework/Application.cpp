#include <Framework/Application.h>

#include "Framework/Core/ErrorCodes.h"
#include "Framework/Core/Logging.h"

PApplication* PApplication::m_instance = getInstance();

PApplication* PApplication::getInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new PApplication();
	}
	return m_instance;
}

int32 PApplication::run() const
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
