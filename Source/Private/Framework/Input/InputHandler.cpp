#include "Framework/Input/InputHandler.h"

IInputHandler* IInputHandler::getInstance()
{
#if (defined(_WIN32) || defined(_WIN64))
	return Win32InputHandler::getInstance();
#else
	// Other handlers
#endif
}

Win32InputHandler* Win32InputHandler::m_instance = getInstance();

Win32InputHandler* Win32InputHandler::getInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new Win32InputHandler();
	}
	return m_instance;
}
