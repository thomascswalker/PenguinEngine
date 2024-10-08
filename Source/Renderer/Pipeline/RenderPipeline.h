#pragma once

#include "Core/Types.h"
#include "Engine/Object.h"
#include "Renderer/Grid.h"
#include "Renderer/Settings.h"

class IRenderPipeline
{
public:
	IRenderPipeline() = default;
	IRenderPipeline(int32 width, int32 height) {}
	virtual ~IRenderPipeline() = default;

	/** Main pipeline functions. **/

	virtual bool init(void* windowHandle) = 0;
	virtual void beginDraw() = 0;
	virtual void draw(IRenderable* renderable) = 0;
	virtual void endDraw() = 0;
	virtual void shutdown() = 0;
	virtual void resize(int32 width, int32 height) = 0;

	/** Drawing functions. **/

	virtual void drawGrid(Grid* grid) = 0;
	virtual void drawLine(const vec3f& inA, const vec3f& inB, const Color& color) = 0;

	/** Getter/setters **/

	virtual uint8* getFrameData() = 0;
	virtual void setViewData(ViewData* newViewData) = 0;
	virtual void setRenderSettings(RenderSettings* newRenderSettings) = 0;

	virtual void setVertexData(float* data, size_t size, int32 count) = 0;
};

class ScanlineRenderPipeline;
class D3D11RenderPipeline;
