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

	uint32 getWidth() const { return m_camera->m_width; }
	uint32 getHeight() const { return m_camera->m_height; }
	vec2f getSize() const;
	float getAspect() const { return static_cast<float>(m_camera->m_width) / static_cast<float>(m_camera->m_height); }
	Camera* getCamera() const { return m_camera.get(); }

	void resetView() const;
	bool projectWorldToScreen(const vec3f& worldPosition, vec3f& screenPosition) const;
	bool projectScreenToWorld(const vec2f& screenPosition, float depth, vec3f& worldPosition) const;

	void formatDebugText();
	std::string getDebugText() const { return m_debugText; }
	bool getShowDebugText() const { return m_showDebugText; }
	void toggleShowDebugText() { m_showDebugText = !m_showDebugText; }
};
