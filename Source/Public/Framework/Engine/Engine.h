#pragma once

#include "Mesh.h"
#include "Timer.h"
#include "Framework/Input/InputHandler.h"
#include "Framework/Renderer/Renderer.h"

class Engine
{
	std::shared_ptr<Renderer> m_renderer;
	bool m_isRunning = false;

	timePoint m_startTime;
	float m_deltaTime = 0.0f;

	float m_cameraSpeed = .01f;
	float m_cameraSpeedMultiplier = 1.0f;

	std::vector<float*> m_vertexes;
	std::vector<uint32*> m_indexes;

public:
	static Engine* m_instance;
	static Engine* getInstance();

	bool startup(uint32 inWidth, uint32 inHeight);
	bool shutdown();

	void tick();

	Renderer* getRenderer() const { return m_renderer.get(); }
	Viewport* getViewport() const { return getRenderer()->getViewport(); }
	Camera* getViewportCamera() const { return getViewport()->getCamera(); }
	bool isRunning() const { return m_isRunning; }
	void setRunning(const bool newState) { m_isRunning = newState; }

	void openFile(const std::string& fileName);

	std::vector<std::shared_ptr<Mesh>> m_meshes;
	std::vector<std::shared_ptr<Mesh>> getMeshes() const { return m_meshes; }

	constexpr float getFps() const { return 1000.0f / m_deltaTime; }

	void onMouseMiddleScrolled(float delta) const;
	void onKeyPressed(EKey keyCode) const;

	void onLeftMouseUp(const vec2f& cursorPosition) const;
	void onMiddleMouseUp(const vec2f& cursorPosition) const;

	void onMenuActionPressed(EMenuAction actionId);
	void onOpenPressed();
};
