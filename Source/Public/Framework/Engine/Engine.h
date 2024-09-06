#pragma once

#include "Mesh.h"
#include "Timer.h"
#include "Framework/Input/InputHandler.h"
#include "Framework/Renderer/Renderer.h"

class Engine
{
	std::shared_ptr<Renderer> m_renderer;
	bool					  m_isRunning = false;

	timePoint m_startTime;
	float	  m_deltaTime = 0.0f;

	float m_cameraSpeed = .01f;
	float m_cameraSpeedMultiplier = 1.0f;

	std::vector<float*>	 m_vertices;
	std::vector<uint32*> m_indexes;

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

	[[nodiscard]] Renderer* getRenderer() const
	{
		return m_renderer.get();
	}

	[[nodiscard]] Viewport* getViewport() const
	{
		return getRenderer()->getViewport();
	}

	[[nodiscard]] Camera* getViewportCamera() const
	{
		return getViewport()->getCamera();
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

	void onMouseMiddleScrolled(float delta) const;
	void onKeyPressed(EKey keyCode) const;

	void onLeftMouseUp(const vec2f& cursorPosition) const;
	void onMiddleMouseUp(const vec2f& cursorPosition) const;

	void onMenuActionPressed(EMenuAction actionId);

	/**
	 * @brief Called when the Load Model menu action is pressed. Loads the mesh selected by the file open dialog.
	 */
	void onLoadModelPressed();

	/**
	 * @brief Called when the Load Texture menu action is pressed. Loads the texture selected by the file open dialog.
	 */
	void onLoadTexturePressed();
};
