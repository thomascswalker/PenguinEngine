﻿#pragma once

#include <map>
#include <memory>

#include "Texture.h"
#include "Grid.h"
#include "Settings.h"
#include "Shader.h"
#include "Viewport.h"
#include "Framework/Engine/Mesh.h"
#include "Math/MathCommon.h"

class Renderer
{
	// Draw channels
	std::shared_ptr<Viewport> m_viewport;
	std::unique_ptr<FGrid> m_grid;

	// Constants
	Color m_wireColor = Color::fromRgba(255, 175, 50);
	Color m_gridColor = Color::fromRgba(128, 128, 128);

	// Shaders
	std::shared_ptr<IShader> m_currentShader = nullptr;

	std::shared_ptr<Texture> m_colorBitmap;
	std::shared_ptr<Texture> m_depthBitmap;

public:
	// Settings
	RenderSettings m_settings;

	Renderer(uint32 inWidth, uint32 inHeight);
	void resize(uint32 inWidth, uint32 inHeight) const;

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

	[[nodiscard]] Texture* getColorBitmap() const
	{
		return m_colorBitmap.get();
	}
	[[nodiscard]] IShader* getShader() const
	{
		return m_currentShader.get();
	}

	/* Drawing */

	bool clipLine(vec2f* a, vec2f* b) const;
	bool clipLine(linef* line) const;
	void drawLine(const vec3f& inA, const vec3f& inB, const Color& color) const;
	void drawLine(const line3d& line, const Color& color) const;
	void drawTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2) const;
	void drawMesh(const Mesh* mesh) const;
	void drawGrid() const;
	void draw() const;

	// Rasterizing triangles
	void scanline() const;
};
