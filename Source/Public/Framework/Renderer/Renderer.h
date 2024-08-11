#pragma once

#include <map>
#include <memory>

#include "Grid.h"
#include "Settings.h"
#include "Shader.h"
#include "Viewport.h"
#include "Framework/Engine/Mesh.h"
#include "Framework/Platforms/PlatformMemory.h"
#include "Math/MathCommon.h"


constexpr int32 g_bytesPerChannel = 8;
constexpr int32 g_bytesPerPixel = 32;


enum class EOrientation
{
	Horizontal,
	Vertical
};

struct Channel
{
	void* m_memory; // Memory Order BB GG RR XX
	uint32 m_channelCount = g_bytesPerPixel / g_bytesPerChannel; // 4
	int32 m_width; // Width of the frame in pixels
	int32 m_height; // Height of the frame in pixels
	uint32 m_pitch; // Count of bytes in a single row (equivalent to Width)
	EChannelType m_type; // Color (RGB, 8-bit) or Data (float, 32-bit) channel

	Channel(const EChannelType inType, const int32 inWidth, const int32 inHeight) : m_type(inType)
	{
		resize(inWidth, inHeight);
	}

	void resize(const int32 inWidth, const int32 inHeight)
	{
		m_width = inWidth;
		m_height = inHeight;
		m_pitch = m_channelCount * inWidth;
		realloc();
	}

	void realloc()
	{
		m_memory = PlatformMemory::realloc(m_memory, getMemorySize());
	}

	constexpr int32 getOffset(const int32 x, const int32 y) const { return (y * (m_pitch / m_channelCount)) + x; }
	constexpr int32 getPixelCount() const { return m_width * m_height; }
	constexpr int32 getMemorySize() const { return m_width * m_height * g_bytesPerPixel * m_channelCount; }

	void setPixel(const int32 x, const int32 y, const float value) const
	{
		assert(m_type == EChannelType::Data);
		if (x < 0 || x >= m_width || y < 0 || y >= m_height)
		{
			return;
		}

		float* ptr = static_cast<float*>(m_memory) + getOffset(x, y);
		*ptr = value;
	}

	void setPixel(const int32 x, const int32 y, const Color& color) const
	{
		assert(m_type == EChannelType::Color);
		if (x < 0 || x >= m_width || y < 0 || y >= m_height)
		{
			return;
		}

		int32* ptr = static_cast<int32*>(m_memory) + getOffset(x, y);

		// Placing pixels in memory order within a uint32:
		// BB GG RR AA
		// 00 00 00 00
		*ptr = ((color.r << 16) | color.g << 8) | color.b; // TODO: Disregard alpha channel for now
	}

	template <typename T>
	T getPixel(const int32 x, const int32 y) const
	{
		return *(static_cast<T*>(m_memory) + getOffset(x, y));
	}

	void clear() const
	{
		PlatformMemory::fill(m_memory, static_cast<size_t>(m_width * m_height * 4), 0);
	}

	template <typename T>
	void fill(T value)
	{
		PlatformMemory::fill(m_memory, static_cast<size_t>(m_width * m_height * 4), value);
	}
};

enum class EDataType
{
	Int8,
	Int16,
	Int32,
	UInt8,
	UInt16,
	UInt32,
	Float
};

enum class EBufferType
{
	Array,
	ElementArray
};

template <typename T>
struct PBufferObject
{
	std::vector<T> m_memory;
	EDataType m_dataType;
	EBufferType m_bufferType;
	size_t m_size = 0;

	PBufferObject(const EDataType inDataType, const EBufferType inBufferType)
	{
		m_dataType = inDataType;
		m_bufferType = inBufferType;
	}
};

class Renderer
{
	// Draw channels
	std::map<std::string, std::shared_ptr<Channel>> m_channels;
	std::shared_ptr<Viewport> m_viewport;
	std::unique_ptr<FGrid> m_grid;

	std::unique_ptr<PBufferObject<float>> m_vertexBuffer;
	std::unique_ptr<PBufferObject<uint32>> m_indexBuffer;

	// Constants
	const Color m_wireColor = Color::fromRgba(255, 175, 50);
	const Color m_gridColor = Color::fromRgba(128, 128, 128);

	// Shaders
	std::shared_ptr<IShader> m_currentShader = nullptr;

public:
	// Settings
	RenderSettings m_settings;

	Renderer(uint32 inWidth, uint32 inHeight);
	void resize(uint32 inWidth, uint32 inHeight) const;
	int32 getWidth() const { return m_viewport->getCamera()->m_width; }
	int32 getHeight() const { return m_viewport->getCamera()->m_height; }
	Viewport* getViewport() const { return m_viewport.get(); }

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

	std::shared_ptr<Channel> getColorChannel() const { return m_channels.at("Color"); }
	std::shared_ptr<Channel> getDepthChannel() const { return m_channels.at("Depth"); }

	/* Drawing */

	bool clipLine(vec2f* a, vec2f* b) const;
	bool clipLine(linef* line) const;
	void drawLine(const vec3f& inA, const vec3f& inB, const Color& color) const;
	void drawLine(const line3d& line, const Color& color) const;
	void drawTriangle(const Vertex& v0, const Vertex& v1, const Vertex& v2);
	void drawMesh(const Mesh* mesh);
	void drawGrid() const;
	void draw();


	// Rasterizing triangles
	void scanline() const;
};
