#pragma once

#include <map>
#include <stdlib.h>

#include "Framework/Platforms/PlatformMemory.h"
#include "Math/Color.h"
#include "Math/Vector.h"

class Texture;
inline std::vector<Texture> g_textures{};

namespace TextureManager
{
	inline Texture* getTexture(const int32 index)
	{
		return &g_textures[index];
	}
}

/**
 * @brief Texture class for storing pixel data in a 2D format.
 */
class Texture
{
	/** The memory pointer this texture uses to store pixels. */
	void* m_data = nullptr;
	/** The 2-dimensional compressedSize of this texture. */
	vec2i m_size;
	/** The size of a single row of pixels. */
	size_t m_pitch = 0;
	/** The number of channels in this texture. */
	int32 channelCount = 0;

public:
	Texture() {}

	explicit Texture(const vec2i inSize) : m_size(inSize), m_pitch(inSize.x)
	{
		size_t memSize = getMemorySize();
		m_data = PlatformMemory::malloc(memSize);
	}

	Texture(const Texture& other)
		: m_data(other.m_data),
		m_size(other.m_size),
		m_pitch(other.m_pitch)
	{}

	Texture(Texture&& other) noexcept
		: m_data(other.m_data),
		m_size(other.m_size),
		m_pitch(other.m_pitch)
	{}

	Texture& operator=(const Texture& other)
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

	Texture& operator=(Texture&& other) noexcept
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

	~Texture()
	{
		// Free the memory blob upon this texture's destruction
		//free(m_data);
	}

	static Texture fromData(uint8* inData, const uint32 inWidth, const uint32 inHeight)
	{
		Texture bm(vec2i(inWidth, inHeight));
		bm.setMemory(inData);
		return bm;
	}

	bool isValid() const
	{
		return m_data != nullptr;
	}

	void resize(const vec2i inSize)
	{
		m_size = inSize;
		m_pitch = inSize.x;
		m_data = PlatformMemory::malloc<uint8>(getMemorySize());
	}

	/**
	 * @brief Returns the raw void pointer to this texture's memory.
	 */
	[[nodiscard]] void* getRawMemory() const
	{
		return m_data;
	}

	/**
	 * @brief Returns a type T (e.g. int32, float) pointer to this texture's memory.
	 */
	template <typename T>
	T* getMemory() const
	{
		return static_cast<T*>(m_data);
	}

	template <typename T>
	void setMemory(T* newMemory, const size_t inSize = 0)
	{
		if (newMemory == nullptr)
		{
			LOG_ERROR("Invalid memory allocation.")
				return;
		}
		auto size = inSize ? inSize : getMemorySize();
		memcpy(m_data, newMemory, size);
	}

	/**
	 * @brief Returns the memory size of this texture in bytes.
	 * @return The number of bytes this texture allocates.
	 */
	[[nodiscard]] size_t getMemorySize() const
	{
		return m_size.x * m_size.y * g_bytesPerPixel;
	}

	/**
	 * @brief Returns the width of the texture.
	 */
	[[nodiscard]] int32 getWidth() const
	{
		return m_size.x;
	}

	/**
	 * @brief Returns the height of the texture.
	 */
	[[nodiscard]] int32 getHeight() const
	{
		return m_size.y;
	}

	/**
	 * @brief Fills this texture with the specified color.
	 * @param inColor The color to fill this texture with.
	 */
	void fill(const Color& inColor) const
	{
		int32 color = inColor.toInt32();
		PlatformMemory::fill(m_data, getMemorySize(), color);
	}

	/**
	 * @brief Fills this texture with the specified float value.
	 * @note This is currently probably slower than it needs to be. std::memset & `std::fill` don't accept floats.
	 * @param value The value to fill this texture with.
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
		int32* line = (int32*)m_data + (y * m_pitch);
		line[x] = color.toInt32();
	}

	void setPixelFromFloat(const int32 x, const int32 y, float value) const
	{
		int32* line = (int32*)m_data + (y * m_pitch);
		int32* castInt = reinterpret_cast<int32*>(&value);
		line[x] = *castInt;
	}

	// stbi__vertical_flip
	static void flipVertical(void* ptr, int32 width, int32 height)
	{
		size_t bytesPerRow = (size_t)width * g_bytesPerPixel;
		uint8 temp[2048];
		uint8* bytes = (uint8*)ptr;

		for (int32 row = 0; row < (height >> 1); row++)
		{
			uint8* row0 = bytes + (row * bytesPerRow);
			uint8* row1 = bytes + ((height - row - 1) * bytesPerRow);
			// swap row0 with row1
			size_t bytesLeft = bytesPerRow;
			while (bytesLeft)
			{
				size_t bytesToCopy = (bytesLeft < sizeof(temp)) ? bytesLeft : sizeof(temp);
				memcpy(temp, row0, bytesToCopy);
				memcpy(row0, row1, bytesToCopy);
				memcpy(row1, temp, bytesToCopy);
				row0 += bytesToCopy;
				row1 += bytesToCopy;
				bytesLeft -= bytesToCopy;
			}
		}
	}

	void flipVertical()
	{
		Texture::flipVertical(m_data, m_size.x, m_size.y);
	}
};
