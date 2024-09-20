#pragma once
#include "Framework/Core/Types.h"
#include "Framework/Renderer/Grid.h"
#include "Framework/Renderer/Settings.h"

class IRenderPipeline
{
public:
	IRenderPipeline()          = default;
	virtual ~IRenderPipeline() = default;

	/** Main pipeline functions. **/

	virtual void startup() = 0;
	virtual void preDraw() = 0;
	virtual void draw() = 0;
	virtual void postDraw() = 0;
	virtual void shutdown() = 0;
	virtual void resize(int32 width, int32 height) = 0;

	/** Drawing functions. **/

	virtual void drawGrid(Grid* grid) = 0;
	virtual void drawLine(const vec3f& inA, const vec3f& inB, const Color& color) = 0;

	/** Getter/setters **/

	virtual uint8* getFrameData() = 0;
	virtual void setViewData(ViewData* newViewData) = 0;
	virtual void setRenderSettings(RenderSettings* newRenderSettings) = 0;
};
