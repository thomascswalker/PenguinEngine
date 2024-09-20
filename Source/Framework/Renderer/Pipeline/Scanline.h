#pragma once

#include <memory>
#include <thread>

#include "RenderPipeline.h"

#include "Framework/Engine/Mesh.h"
#include "Framework/Renderer/Camera.h"
#include "Framework/Renderer/Grid.h"
#include "Framework/Renderer/Settings.h"
#include "Framework/Renderer/Shader.h"
#include "Framework/Renderer/Texture.h"
#include "Framework/Renderer/Tile.h"

class ScanlineRenderPipeline : public IRenderPipeline
{
	std::shared_ptr<IShader> m_currentShader = nullptr;
	std::shared_ptr<Texture> m_colorTexture  = nullptr;
	std::shared_ptr<Texture> m_depthTexture  = nullptr;

	std::vector<Tile> m_tiles;
	int32 m_threadCount = 0;

	ViewData* m_viewData             = nullptr;
	Mesh* m_currentMesh              = nullptr;
	Triangle* m_currentTriangle      = nullptr;
	RenderSettings* m_renderSettings = nullptr;

	int32 m_width  = 0;
	int32 m_height = 0;

public:
	void startup() override;
	void preDraw() override;
	void draw() override;
	void postDraw() override;
	void shutdown() override {}
	void resize(int32 width, int32 height) override;

	void drawGrid(Grid* grid) override;
	void drawTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2);
	void drawScanline() const;
	void drawLine(const vec3f& inA, const vec3f& inB, const Color& color) override;

	uint8* getFrameData() override;
	void setViewData(ViewData* newViewData) override;
	void setRenderSettings(RenderSettings* newRenderSettings) override;
};
