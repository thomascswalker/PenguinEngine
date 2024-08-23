#pragma once

#include <stdlib.h>

#include "Framework/Platforms/PlatformMemory.h"
#include "Math/Color.h"
#include "Math/Vector.h"

/**
 * @brief Bitmap class for storing pixel data in a 2D format.
 */
class Bitmap
{
	/** The memory pointer this bitmap uses to store pixels. */
	void* m_data = nullptr;
	/** The 2-dimensional size of this bitmap. */
	vec2i m_size;
	/** The size of a single row of pixels. */
	size_t m_pitch;

public:
	Bitmap() {}

	explicit Bitmap(const vec2i inSize) : m_size(inSize), m_pitch(inSize.x)
	{
		size_t memSize = getMemorySize();
		m_data = std::malloc(memSize);
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
		// Free the memory blob upon this bitmap's destruction
		//free(m_data);
	}

	static Bitmap fromData(uint8* inData, const uint32 inWidth, const uint32 inHeight)
	{
		auto bm = Bitmap(vec2i(inWidth, inHeight));
		bm.setMemory(inData);
		return bm;
	}

	void resize(const vec2i inSize)
	{
		m_size = inSize;
		m_pitch = inSize.x;
		m_data = PlatformMemory::realloc(m_data, getMemorySize());
	}

	/**
	 * @brief Returns the raw void pointer to this bitmap's memory.
	 */
	[[nodiscard]] void* getRawMemory() const
	{
		return m_data;
	}

	/**
	 * @brief Returns a type T (e.g. int32, float) pointer to this bitmap's memory.
	 */
	template <typename T>
	T* getMemory() const
	{
		return static_cast<T*>(m_data);
	}

	template <typename T>
	void setMemory(T* newMemory, const size_t inSize = 0)
	{
		auto size = inSize ? inSize : getMemorySize();
		memcpy(m_data, newMemory, size);
	}

	/**
	 * @brief Returns the memory size of this bitmap in bytes.
	 * @return The number of bytes this bitmap allocates.
	 */
	[[nodiscard]] size_t getMemorySize() const
	{
		return m_size.x * m_size.y * g_bytesPerPixel;
	}

	/**
	 * @brief Returns the width of the bitmap.
	 */
	[[nodiscard]] int32 getWidth() const
	{
		return m_size.x;
	}

	/**
	 * @brief Returns the height of the bitmap.
	 */
	[[nodiscard]] int32 getHeight() const
	{
		return m_size.y;
	}

	/**
	 * @brief Fills this bitmap with the specified color.
	 * @param inColor The color to fill this bitmap with.
	 */
	void fill(const Color& inColor) const
	{
		int32 color = inColor.toInt32();
		PlatformMemory::fill(m_data, getMemorySize(), color);
	}

	/**
	 * @brief Fills this bitmap with the specified float value.
	 * @note This is currently probably slower than it needs to be. std::memset & `std::fill` don't accept floats.
	 * @param value The value to fill this bitmap with.
	 */
	void fill(const float value) const
	{
		auto floatData = (float*)m_data;
		int32 size = m_size.x * m_size.y;
		for (int32 i = 0; i < size; i++)
		{
			floatData[i] = value;
		}

		// TODO: Figure out why this fails for floats
		//PlatformMemory::fill(m_data, getMemorySize(), value);
	}

	/**
	 * @brief Returns a pointer to the beginning of a row of pixels.
	 * @tparam T The data type of the pixels (e.g. int32, float).
	 * @param y The row to return.
	 * @return A type T pointer to the row of pixels.
	 */
	template <typename T>
	[[nodiscard]] T* scanline(const int y) const
	{
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

	[[nodiscard]] int32 getPixelAsInt32(const int32 x, const int32 y) const
	{
		int32* line = scanline<int32>(y);
		return line[x];
	}

	[[nodiscard]] float getPixelAsFloat(const int32 x, const int32 y) const
	{
		int32 pixel = getPixelAsInt32(x, y);
		return *(reinterpret_cast<float*>(&pixel));
	}

	template <typename T>
	void setPixel(const int32 x, const int32 y, const T color) const
	{
		T* line = static_cast<T*>(m_data) + (y * m_pitch);
		line[x] = color;
	}

	void setPixelFromColor(const int32 x, const int32 y, const Color& color) const
	{
		int32* line = static_cast<int32*>(m_data) + (y * m_pitch);
		line[x] = color.toInt32();
	}

	void setPixelFromFloat(const int32 x, const int32 y, float value) const
	{
		int32* line = static_cast<int32*>(m_data) + (y * m_pitch);
		int32* castInt = reinterpret_cast<int32*>(&value);
		line[x] = *castInt;
	}
};
