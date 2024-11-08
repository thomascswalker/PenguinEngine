#pragma once

//#define PENG_HARDWARE_ACCELERATION

#include "Engine/Actors/Camera.h"
#include "Grid.h"
#include "Settings.h"
#include "Core/Buffer.h"
#include "Core/Types.h"
#include "Math/MathCommon.h"
#include "Pipeline/RHI.h"

class Viewport
{
	std::unique_ptr<Grid> m_grid;
	std::shared_ptr<IRHI> m_rhi;

public:
	/* Render settings. */
	RenderSettings m_settings;
	Camera* m_camera;

	std::string m_debugText;
	bool m_showDebugText = true;

	bool m_flipNormals = false;

	Viewport(int32 inWidth, int32 inHeight);
	~Viewport();

	/** Misc **/

	void resize(int32 inWidth, int32 inHeight) const;
	[[nodiscard]] int32 getWidth() const;
	[[nodiscard]] int32 getHeight() const;
	[[nodiscard]] vec2i getSize() const;
	[[nodiscard]] float getAspect() const;
	[[nodiscard]] Camera* getCamera() const;
	void resetView() const;

	/** Render pipeline **/

	void draw();
	bool createRHI();
	bool initRHI(void* windowHandle) const;
	[[nodiscard]] IRHI* getRHI() const;

	/** Debug **/

	void formatDebugText();
	[[nodiscard]] std::string getDebugText() const;
	[[nodiscard]] bool getShowDebugText() const;
	void toggleShowDebugText();
};
