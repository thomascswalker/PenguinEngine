#pragma once

#include "Input/InputHandler.h"
#include "Platforms/Generic/Application.h"
#include "Timer.h"

extern inline Engine* g_engine = nullptr;

class Engine
{
protected:
	IApplication* m_application;

public:
	virtual bool   initialize(IApplication* app);
	virtual bool   shutdown();
	virtual void   tick(float deltaTime);

	void onMouseMiddleScrolled(MouseData& mouse) const;
	void onLeftMouseDown(MouseData& mouse) const;
	void onLeftMouseUp(MouseData& mouse) const;
	void onMiddleMouseUp(MouseData& mouse) const;
	void onMouseMoved(MouseData& mouse) const;

	void onKeyPressed(EKey keyCode) const;
};
