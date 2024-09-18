#pragma once

#include <map>
#include <memory>

#include "Framework/Engine/Mesh.h"
#include "Grid.h"
#include "Math/MathCommon.h"
#include "Settings.h"
#include "Shader.h"
#include "Texture.h"
#include "Viewport.h"

/**
 * This class manages the viewport, output color and depth buffers, and the current shader used to render.
 */
class Renderer
{
	std::shared_ptr<Viewport> m_viewport;
	std::unique_ptr<Grid> m_grid;

	Color m_wireColor = Color::fromRgba(255, 175, 50);
	Color m_gridColor = Color::fromRgba(128, 128, 128);

	/* The current shader to render with. */
	std::shared_ptr<IShader> m_currentShader = nullptr;

	/* The color buffer which is displayed in the application window. */
	std::shared_ptr<Texture> m_colorTexture;

	/* The depth buffer which is used to track depth of each pixel. */
	std::shared_ptr<Texture> m_depthTexture;

public:
	/* Render settings. */
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
		return m_colorTexture.get();
	}

	[[nodiscard]] IShader* getShader() const
	{
		return m_currentShader.get();
	}

	/**
	 * @brief Clips line [\p a, \p b] with the current viewport size.
	 * @param a Point \p a of the line.
	 * @param b Point \p b of the line.
	 * @return bool True if line [\p a, \p b] was clipped, false otherwise.
	 */
	bool clipLine(vec2f* a, vec2f* b) const;

	/**
	 * @brief Clips \p line with the current viewport size.
	 * @param line The line to clip.
	 * @return bool True if \p line was clipped, false otherwise.
	 */
	bool clipLine(linef* line) const;
	void drawLine(const vec3f& inA, const vec3f& inB, const Color& color) const;
	void drawLine(const line3d& line, const Color& color) const;
	void drawTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2) const;

	/**
	 * @brief Draws the specified \p mesh to the color buffer.
	 *
	 * TODO: Rewrite to use a single array of vertices rather than looping through meshes/triangles
	 *
	 * @param mesh The mesh to draw. The mesh lives within the `g_meshes` global variable found in Engine.cpp.
	 */
	void drawMesh(const Mesh* mesh) const;

	/**
	 * @brief Draws the default grid to the color buffer. This does NOT impact the depth buffer.
	 */
	void drawGrid() const;

	/**
	 * @brief Draws the current scene, including all meshes and the grid.
	 * 
	 * 1. Recomputes the view-projection matrix.
	 * 
	 * 2. Clears the color and depth buffers.
	 * 
	 * 3. Draws the grid.
	 * 
	 * 4. Draws each mesh in `g_meshes`.
	 */
	void draw() const;

	/**
	 * @brief Renders the current frame with a standard scanline method.
	 */
	void scanline() const;
};
