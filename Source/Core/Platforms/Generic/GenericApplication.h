#pragma once

#include <string>
#include <Core/Types.h>

#include "GenericWindow.h"
#include "Input/InputHandler.h"

/** @brief Base class of all OS Applications (Win32, Mac, Linux).
 *  This abstracts the implementation of each platform based on its
 *  specific implementation.
 *  This class maintains the lifetime of the engine itself and any input from the user.
 */
class IApplication
{
protected:
	bool m_isRunning = false;

public:
	virtual ~IApplication() = default;

	bool getIsRunning() const { return m_isRunning; }

	virtual void init() = 0;
	virtual void		   tick(float deltaTime) = 0;
	virtual void		   setupInput() = 0;
	virtual void		   processMessages() = 0;
};
