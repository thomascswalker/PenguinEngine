#pragma once

#include <memory>

#include "Core/Logging.h"
#include "Platforms/PlatformInterface.h"
#include "Engine/Engine.h"
#include "Input/InputHandler.h"

class Engine;

class Application
{
public:
	static Application* m_instance;
	static Application* getInstance();

	Application(const Application& other) = delete;
	Application(Application&& other) noexcept = delete;
	Application& operator=(const Application& other) = delete;
	Application& operator=(Application&& other) noexcept = delete;

	template <class PlatformType, typename... Types>
	void Init(Types&&... Args) // NOLINT
	{
		m_platform = std::make_shared<PlatformType>(std::forward<Types>(Args)...);
		m_engine = std::make_shared<Engine>();

		m_inputHandler = Win32InputHandler::getInstance();
		LOG_INFO("Initialized application.")
	}

	int32 run() const;
	IPlatform* getPlatform() const { return m_platform.get(); }
	Engine* getEngine() const { return m_engine.get(); }

protected:
	Application() = default;
	IInputHandler* m_inputHandler;

private:
	std::shared_ptr<IPlatform> m_platform;
	std::shared_ptr<Engine> m_engine;
};
