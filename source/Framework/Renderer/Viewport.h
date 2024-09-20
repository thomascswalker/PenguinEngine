#pragma once

#include "Camera.h"
#include "Math/MathCommon.h"

class Viewport
{
	std::shared_ptr<Camera> m_camera;
	bool m_showDebugText = true;
	std::string m_debugText;

public:
	Viewport(uint32 inWidth, uint32 inHeight);
	void resize(uint32 inWidth, uint32 inHeight) const;

	[[nodiscard]] uint32 getWidth() const
	{
		return m_camera->m_width;
	}

	[[nodiscard]] uint32 getHeight() const
	{
		return m_camera->m_height;
	}

	[[nodiscard]] vec2f getSize() const;

	[[nodiscard]] float getAspect() const
	{
		return static_cast<float>(m_camera->m_width) / static_cast<float>(m_camera->m_height);
	}

	[[nodiscard]] Camera* getCamera() const
	{
		return m_camera.get();
	}

	void resetView() const;

	void formatDebugText();

	[[nodiscard]] std::string getDebugText() const
	{
		return m_debugText;
	}

	[[nodiscard]] bool getShowDebugText() const
	{
		return m_showDebugText;
	}

	void toggleShowDebugText()
	{
		m_showDebugText = !m_showDebugText;
	}
};
