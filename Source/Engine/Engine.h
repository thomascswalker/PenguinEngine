#pragma once

#include "Timer.h"
#include "Input/InputHandler.h"
#include "Renderer/Viewport.h"

class Engine
{
	std::shared_ptr<Viewport> m_viewport;
	bool					  m_isRunning = false;

	TimePoint m_startTime;
	float	  m_deltaTime = 0.0f;

	float m_cameraSpeed = .01f;
	float m_cameraSpeedMultiplier = 1.0f;

public:
	static Engine* m_instance;
	static Engine* getInstance();

	/**
	 * @brief Called during the IPlatform::create() method.
	 * @param inWidth The default window width.
	 * @param inHeight The default window height.
	 * @return bool True if startup is successful, false otherwise.
	 */
	bool startup(uint32 inWidth, uint32 inHeight);

	/**
	 * @brief Called when the engine is requested to shutdown. This is handled by the specific IPlatform instantiation.
	 * @return bool True if the shutdown is successful, false otherwise.
	 */
	bool shutdown();

	/**
	 * @brief Called every game loop. This will tick all objects in the scene (cameras, meshes, etc.)
	 */
	void tick();

	[[nodiscard]] Viewport* getViewport() const
	{
		return m_viewport.get();
	}

	[[nodiscard]] Camera* getViewportCamera() const
	{
		return m_viewport->getCamera();
	}

	[[nodiscard]] bool isRunning() const
	{
		return m_isRunning;
	}

	void setRunning(const bool newState)
	{
		m_isRunning = newState;
	}

	[[nodiscard]] constexpr float getFps() const
	{
		return 1000.0f / m_deltaTime;
	}

	void onMouseMiddleScrolled(const MouseData& mouse) const;
	void onLeftMouseDown(const MouseData& mouse) const;
	void onLeftMouseUp(const MouseData& mouse) const;
	void onMiddleMouseUp(const MouseData& mouse) const;
	void onMouseMoved(const MouseData& mouse) const;

	void onKeyPressed(EKey keyCode) const;

	void onMenuActionPressed(EMenuAction actionId);

	/**
	 * @brief Called when the Load Model menu action is pressed. Loads the mesh selected by the file open dialog.
	 */
	void loadMesh() const;

	/**
	 * @brief Called when the Load Texture menu action is pressed. Loads the texture selected by the file open dialog.
	 */
	void loadTexture() const;
};
