#pragma once

#include <map>
#include <memory>

#include "Grid.h"
#include "Settings.h"
#include "Shader.h"
#include "Viewport.h"
#include "Framework/Engine/Mesh.h"
#include "Math/MathCommon.h"
#include "Framework/Renderer/Channel.h"

class Renderer
{
	// Draw channels
	std::map<std::string, std::shared_ptr<Channel>> m_channels;
	std::shared_ptr<Viewport> m_viewport;
	std::unique_ptr<FGrid> m_grid;

	// Constants
	Color m_wireColor = Color::fromRgba(255, 175, 50);
	Color m_gridColor = Color::fromRgba(128, 128, 128);

	// Shaders
	std::shared_ptr<IShader> m_currentShader = nullptr;

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

	/* Channels */

	void addChannel(EChannelType type, const char* name)
	{
		m_channels.emplace(name, std::make_shared<Channel>(type, getWidth(), getHeight()));
	}

	std::shared_ptr<Channel> getChannel(const char* name) const
	{
		return m_channels.at(name);
	}

	void clearChannels() const
	{
		// Set all channels to 0
		for (const auto& [key, channel] : m_channels)
		{
			// Ignore the depth channel, we'll handle that later
			if (key == "Depth") // NOLINT
			{
				continue;
			}
			channel->clear();
		}

		// Fill the depth buffer with the Max z-depth
		getDepthChannel()->fill(g_defaultMaxz);
	}

	[[nodiscard]] std::shared_ptr<Channel> getColorChannel() const
	{
		return m_channels.at("Color");
	}

	[[nodiscard]] std::shared_ptr<Channel> getDepthChannel() const
	{
		return m_channels.at("Depth");
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
