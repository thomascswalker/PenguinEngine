#pragma once

#include "Framework/Platforms/PlatformMemory.h"
#include "Math/Color.h"
#include "Math/Vector.h"

class Bitmap
{
	void* m_data = nullptr;
	vec2i m_size;
	size_t m_pitch;

public:
	explicit Bitmap(const vec2i inSize) : m_size(inSize), m_pitch(inSize.x)
	{
		size_t memSize = getMemorySize();
		m_data = PlatformMemory::alloc(memSize);
	}

	Bitmap(const Bitmap& other)
		: m_data(other.m_data),
		  m_size(other.m_size),
		  m_pitch(other.m_pitch) {}

	Bitmap(Bitmap&& other) noexcept
		: m_data(other.m_data),
		  m_size(other.m_size),
		  m_pitch(other.m_pitch) {}

	Bitmap& operator=(const Bitmap& other)
	{
		if (this == &other)
		{
			return *this;
		}
		m_data = other.m_data;
		m_size = other.m_size;
		m_pitch = other.m_pitch;
		return *this;
	}

	Bitmap& operator=(Bitmap&& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}
		m_data = other.m_data;
		m_size = other.m_size;
		m_pitch = other.m_pitch;
		return *this;
	}

	~Bitmap()
	{
		free(m_data);
	}

	void resize(const vec2i inSize)
	{
		m_size = inSize;
		m_pitch = inSize.x;
		m_data = PlatformMemory::realloc(m_data, getMemorySize());
	}

	void* getRawMemory() const
	{
		return m_data;
	}

	template <typename T>
	T* getMemory() const
	{
		return static_cast<T*>(m_data);
	}

	/**
	 * @brief Returns the memory blob size of this bitmap in bytes.
	 * @return The number of bytes this bitmap allocates.
	 */
	[[nodiscard]] size_t getMemorySize() const
	{
		return m_size.x * m_size.y * g_bytesPerChannel;
	}

	[[nodiscard]] int32 getWidth() const
	{
		return m_size.x;
	}

	[[nodiscard]] int32 getHeight() const
	{
		return m_size.y;
	}

	void fill(const Color& inColor) const
	{
		int32 color = inColor.toInt32();
		PlatformMemory::fill(m_data, getMemorySize(), color);
	}

	void fill(const float inColor) const
	{
		PlatformMemory::fill(m_data, getMemorySize(), inColor);
	}

	template <typename T>
	[[nodiscard]] T* scanline(const int y) const
	{
		// ReSharper disable once CppReinterpretCastFromVoidPtr
		return static_cast<T*>(m_data) + (y * m_pitch);
	}

	template <typename T>
	[[nodiscard]] T getPixel(const int32 x, const int32 y) const
	{
		T* line = scanline<T>(y);
		return line[x];
	}

	[[nodiscard]] Color getPixelAsColor(const int32 x, const int32 y) const
	{
		int32* line = scanline<int32>(y);
		return Color::fromInt32(line[x]);
	}

	template <typename T>
	void setPixel(const int32 x, const int32 y, const T color) const
	{
		auto t = static_cast<T*>(m_data);
		T* line = t + (y * m_pitch);
		line[x] = color;
	}

	void setPixelFromColor(const int32 x, const int32 y, const Color& color) const
	{
		auto t = static_cast<int32*>(m_data);
		int32* line = t + (y * m_pitch);
		line[x] = color.toInt32();
	}
};
