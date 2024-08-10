#pragma once

#include <memory>

#include "Core/Logging.h"
#include "Platforms/PlatformInterface.h"
#include "Engine/Engine.h"
#include "Input/InputHandler.h"

class PEngine;

class PApplication
{
public:
	static PApplication* m_instance;
	static PApplication* getInstance();

	PApplication(const PApplication& other) = delete;
	PApplication(PApplication&& other) noexcept = delete;
	PApplication& operator=(const PApplication& other) = delete;
	PApplication& operator=(PApplication&& other) noexcept = delete;

	template <class PlatformType, typename... Types>
	void Init(Types&&... Args) // NOLINT
	{
		m_platform = std::make_shared<PlatformType>(std::forward<Types>(Args)...);
		m_engine = std::make_shared<PEngine>();

		m_inputHandler = PWin32InputHandler::GetInstance();
		LOG_INFO("Initialized application.")
	}

	int32 run() const;
	IPlatform* getPlatform() const { return m_platform.get(); }
	PEngine* getEngine() const { return m_engine.get(); }

protected:
	PApplication() = default;
	IInputHandler* m_inputHandler;

private:
	std::shared_ptr<IPlatform> m_platform;
	std::shared_ptr<PEngine> m_engine;
};
