#pragma once

#include <Core/Types.h>
#include <string>

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

	virtual void  initialize(Engine* m_engine) = 0;
	virtual void  tick(float deltaTime) = 0;
	virtual void  setupInput() = 0;
	virtual bool  isEditor() const = 0;
	virtual int32 exec() = 0;

	virtual std::shared_ptr<GenericWindow> createWindow(const WindowDescription& description, std ::shared_ptr<GenericWindow> parent) = 0;
	virtual std::shared_ptr<GenericWindow> createWindow(std::shared_ptr<GenericWindow> parent, const std::string& title, const vec2i& size, const vec2i& pos) = 0;
	virtual std::shared_ptr<GenericWindow> getMainWindow() = 0;
	virtual void						   processMessages() = 0;
	virtual void						   exit() { m_isRunning = false; }
};
