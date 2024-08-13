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
		m_data = PlatformMemory::alloc(getMemorySize());
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
		PlatformMemory::realloc(m_data, getMemorySize());
	}

	template <typename T>
	T* getMemory() const
	{
		return static_cast<T*>(m_data);
	}

	[[nodiscard]] size_t getMemorySize() const
	{
		return m_size.y * m_pitch;
	}

	[[nodiscard]] int32 getWidth() const
	{
		return m_size.x;
	}

	[[nodiscard]] int32 getHeight() const
	{
		return m_size.y;
	}

	template <typename T>
	[[nodiscard]] T* scanline(const int y) const
	{
		// ReSharper disable once CppReinterpretCastFromVoidPtr
		return reinterpret_cast<T*>(m_data) + (y * m_pitch);
	}

	[[nodiscard]] int8* scanlineInt8(const int y) const
	{
		return scanline<int8>(y);
	}

	[[nodiscard]] int32* scanlineInt32(const int y) const
	{
		return scanline<int32>(y);
	}

	[[nodiscard]] Color getPixel(const int32 x, const int32 y) const
	{
		auto line = scanlineInt32(y);
		return Color::fromInt32(line[x]);
	}

	void setPixel(const int32 x, const int32 y, const Color& color) const
	{
		int32* line = scanlineInt32(y);
		line[x] = color.toInt32();
	}
};
