#pragma once

#include "Framework/Platforms/PlatformMemory.h"
#include "Math/MathCommon.h"

constexpr uint32 g_bytesPerChannel = 8;
constexpr uint32 g_bytesPerPixel = 32;

struct Channel
{
	void* memory;                                              // Memory Order BB GG RR XX
	uint32 channelCount = g_bytesPerPixel / g_bytesPerChannel; // 4
	uint32 width;                                              // Width of the frame in pixels
	uint32 height;                                             // Height of the frame in pixels
	uint32 pitch;                                              // Count of bytes in a single row (equivalent to Width)
	EChannelType channelType;                                  // Color (RGB, 8-bit) or Data (float, 32-bit) channel

	Channel(const EChannelType inType, const uint32 inWidth, const uint32 inHeight) : channelType(inType)
	{
		resize(inWidth, inHeight);
	}

	void resize(const uint32 inWidth, const uint32 inHeight)
	{
		width = inWidth;
		height = inHeight;
		pitch = channelCount * inWidth;
		realloc();
	}

	void realloc()
	{
		memory = PlatformMemory::realloc(memory, getMemorySize());
	}

	[[nodiscard]] constexpr uint32 getOffset(const uint32 x, const uint32 y) const
	{
		return (y * (pitch / channelCount)) + x;
	}

	[[nodiscard]] constexpr uint32 getPixelCount() const
	{
		return width * height;
	}

	[[nodiscard]] constexpr uint32 getMemorySize() const
	{
		return width * height * g_bytesPerPixel * channelCount;
	}

	void setPixel(const uint32 x, const uint32 y, const float value) const
	{
		assert(m_type == EChannelType::Data);
		if (x < 0 || x >= width || y < 0 || y >= height)
		{
			return;
		}

		float* ptr = static_cast<float*>(memory) + getOffset(x, y);
		*ptr = value;
	}

	void setPixel(const uint32 x, const uint32 y, const Color& color) const
	{
		assert(m_type == EChannelType::Color);
		if (x < 0 || x >= width || y < 0 || y >= height)
		{
			return;
		}

		int32* ptr = static_cast<int32*>(memory) + getOffset(x, y);

		// Placing pixels in memory order within a uint32:
		// BB GG RR AA
		// 00 00 00 00
		*ptr = ((color.r << 16) | color.g << 8) | color.b; // TODO: Disregard alpha channel for now
	}

	template <typename T>
	T getPixel(const uint32 x, const uint32 y) const
	{
		return *(static_cast<T*>(memory) + getOffset(x, y));
	}

	void clear() const
	{
		PlatformMemory::fill(memory, width * height * 4, 0);
	}

	template <typename T>
	void fill(T value)
	{
		PlatformMemory::fill(memory, width * height * 4, value);
	}
};
