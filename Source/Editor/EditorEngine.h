#pragma once

#include "Engine/Engine.h"
#include "Platforms/Generic/GenericApplication.h"

inline EditorEngine* g_editor = nullptr;

class EditorEngine : public Engine
{
	GENERATE_SUPER(Engine)

protected:
	IApplication* m_application;

	// UI
	std::shared_ptr<GenericWindow> m_mainWindow;
	std::shared_ptr<Button>		   m_exitButton;
	std::shared_ptr<Button>		   m_newWindowButton;

	std::shared_ptr<GenericWindow> m_newWindow;

public:
	static Engine* create();
	bool		   initialize(IApplication* app) override;
	bool		   shutdown() override;
	void		   tick(float deltaTime) override;

	void constructUI();
	void createMainWindow();
	void createNewWindow();
};
