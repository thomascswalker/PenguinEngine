#pragma once

#include <memory>
#include <thread>

#include "Framework/Engine/Mesh.h"
#include "Grid.h"
#include "Settings.h"
#include "Shader.h"
#include "Texture.h"
#include "Tile.h"
#include "Viewport.h"

#include "Pipeline/Scanline.h"

constexpr int32 g_defaultTileSize = 128;
inline int32 g_defaultTileCount   = true ? 2 : (int32)std::thread::hardware_concurrency();

/**
 * This class manages the viewport, output color and depth buffers, and the current shader used to render.
 */
class Renderer
{
	std::shared_ptr<Viewport> m_viewport;
	std::unique_ptr<Grid> m_grid;
	std::shared_ptr<IRenderPipeline> m_renderPipeline;

	Buffer<uint8> m_frameData;

	Color m_wireColor = Color::fromRgba(255, 175, 50);
	Color m_gridColor = Color::fromRgba(128, 128, 128);

public:
	/* Render settings. */
	RenderSettings m_settings;

	Renderer(uint32 inWidth, uint32 inHeight);

	~Renderer();

	void resize(uint32 inWidth, uint32 inHeight);

	[[nodiscard]] int32 getWidth() const
	{
		return m_viewport->getCamera()->m_width;
	}

	[[nodiscard]] int32 getHeight() const
	{
		return m_viewport->getCamera()->m_height;
	}

	[[nodiscard]] Viewport* getViewport() const
	{
		return m_viewport.get();
	}

	void draw();

	void drawGrid() const;

	uint8* getFrameData() const
	{
		return m_renderPipeline->getFrameData();
	}
};
