#include "Win32InputHandler.h"

Win32InputHandler* Win32InputHandler::m_instance = getInstance();

Win32InputHandler* Win32InputHandler::getInstance()
{
	if (m_instance == nullptr)
	{
		m_instance = new Win32InputHandler();
	}
	return m_instance;
}
