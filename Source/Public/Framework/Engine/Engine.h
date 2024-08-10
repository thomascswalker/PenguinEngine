#pragma once

#include "Mesh.h"
#include "Timer.h"
#include "Framework/Input/InputHandler.h"
#include "Framework/Renderer/Renderer.h"

class PEngine
{
	std::shared_ptr<PRenderer> m_renderer;
	bool m_isRunning = false;

	TimePoint m_startTime;
	float m_deltaTime = 0.0f;

	float m_cameraSpeed = .01f;
	float m_cameraSpeedMultiplier = 1.0f;

	std::vector<float*> m_vertexes;
	std::vector<uint32*> m_indexes;

public:
	static PEngine* m_instance;
	static PEngine* getInstance();

	bool startup(uint32 inWidth, uint32 inHeight);
	bool shutdown();

	void tick();

	PRenderer* getRenderer() const { return m_renderer.get(); }
	PViewport* getViewport() const { return getRenderer()->getViewport(); }
	PCamera* getViewportCamera() const { return getViewport()->GetCamera(); }
	bool isRunning() const { return m_isRunning; }
	void setRunning(const bool newState) { m_isRunning = newState; }

	void openFile(const std::string& fileName);

	std::vector<std::shared_ptr<PMesh>> m_meshes;
	std::vector<std::shared_ptr<PMesh>> getMeshes() const { return m_meshes; }

	constexpr float getFps() const { return 1000.0f / m_deltaTime; }

	void onMouseMiddleScrolled(float delta) const;
	void onKeyPressed(EKey keyCode) const;

	void onLeftMouseUp(const FVector2& cursorPosition) const;
	void onMiddleMouseUp(const FVector2& cursorPosition) const;

	void onMenuActionPressed(EMenuAction actionId);
	void onOpenPressed();
};
