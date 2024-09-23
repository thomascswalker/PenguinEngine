#pragma once

#include "Camera.h"
#include "Grid.h"
#include "Settings.h"
#include "Core/Buffer.h"
#include "Core/Types.h"
#include "Math/MathCommon.h"
#include "Pipeline/RenderPipeline.h"

class Viewport
{
	std::shared_ptr<Viewport> m_viewport;
	std::unique_ptr<Grid> m_grid;
	std::shared_ptr<IRenderPipeline> m_renderPipeline;

public:
	/* Render settings. */
	RenderSettings m_settings;
	std::shared_ptr<Camera> m_camera;

	std::string m_debugText;
	bool m_showDebugText = true;

	Viewport(int32 inWidth, int32 inHeight);
	~Viewport();

	/** Misc **/

	void resize(int32 inWidth, int32 inHeight) const;
	[[nodiscard]] int32 getWidth() const;
	[[nodiscard]] int32 getHeight() const;
	[[nodiscard]] vec2f getSize() const;
	[[nodiscard]] float getAspect() const;
	[[nodiscard]] Camera* getCamera() const;
	void resetView() const;

	/** Render pipeline **/

	void draw();
	bool createRenderPipeline();
	bool initRenderPipeline(void* windowHandle) const;
	IRenderPipeline* getRenderPipeline() const;
	void updateSceneGeometry() const;
	void updateSceneCamera() const;

	/** Debug **/

	void formatDebugText();
	[[nodiscard]] std::string getDebugText() const;
	[[nodiscard]] bool getShowDebugText() const;
	void toggleShowDebugText();
};
