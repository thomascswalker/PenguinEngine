#pragma once

#include <memory>

#include "Core/Logging.h"
#include "Platforms/PlatformInterface.h"
#include "Engine/Engine.h"

/**
 * @brief Manages the lifetime of the entire engine, platform, and input. This class is a singleton.
 */
class Application
{
public:
	/** The single instance of this class. */
	static Application* m_instance;

	/** Returns the single instance of this class. */
	static Application* getInstance();

	Application(const Application& other) = delete;
	Application(Application&& other) noexcept = delete;
	Application& operator=(const Application& other) = delete;
	Application& operator=(Application&& other) noexcept = delete;
	~Application() = default;

	/**
	 * @brief Instantiates a new IPlatform-derived class given the specified PlatformType.
	 * @tparam PlatformType The platform type (Win32, Mac, Linux) to construct.
	 * @tparam Types Additional argument types for platform-specific instantiation.
	 * @param Args Additional arguments for platform-specific instantiation.
	 */
	template <class PlatformType, typename... Types>
	void Init(Types&&... Args) // NOLINT
	{
		m_platform = std::make_shared<PlatformType>(std::forward<Types>(Args)...);
		LOG_INFO("Initialized application.")
	}


	/**
	 * @brief Constructs the platform, platform-specific window, and starts the platform loop.
	 * @return The resulting status code.
	 */
	[[nodiscard]] int32 run() const;


	/**
	 * @brief Returns the constructed platform for this application.
	 * @return The current platform.
	 */
	[[nodiscard]] IPlatform* getPlatform() const
	{
		return m_platform.get();
	}

protected:
	Application() = default;

private:
	/**
	 * @brief Reference to the platform this application manages.
	 */
	std::shared_ptr<IPlatform> m_platform;
};
