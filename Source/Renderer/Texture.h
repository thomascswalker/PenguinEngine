#pragma once

#include <cstdlib>
#include <vector>

#include "Core/Array.h"
#include "Core/Buffer.h"
#include "Platforms/Generic/Platform.h"
#include "Math/Color.h"
#include "Math/Vector.h"

class Texture;
/* Global container for all texture objects. */
inline std::vector<std::shared_ptr<Texture>> g_textures;

namespace TextureManager
{
	inline size_t count()
	{
		return g_textures.size();
	}

	inline Texture* getTexture(const int32 index)
	{
		return g_textures[index].get();
	}
} // namespace TextureManager

enum class ETextureByteOrder
{
	RGBA,
	BRGA
};

/**
 * @brief Texture class for storing pixel data in a 2D format.
 */
class Texture
{
	/** The memory pointer this texture uses to store pixels. */
	RawBuffer<uint8> m_buffer;
	/** The 2-dimensional compressedSize of this texture. */
	vec2i m_size;
	/** The size of a single row of pixels. */
	size_t m_pitch = 0;
	/** The number of channels in this texture. */
	int32 m_channelCount = 0;
	/** The order of RGB bytes in RGBA */
	ETextureByteOrder m_byteOrder = ETextureByteOrder::RGBA;

	void _initFromPlatformType()
	{
		EPlatformType platformType = getPlatformType();
		switch (platformType)
		{
			case EPlatformType::Windows:
				m_byteOrder = ETextureByteOrder::BRGA;
				break;
			case EPlatformType::MacOS:
			case EPlatformType::Linux:
			default:
				m_byteOrder = ETextureByteOrder::RGBA;
				break;
		}
	}

public:
	Texture()
	{
		m_size.x = 1;
		m_size.y = 1;
		m_pitch = 1;
		size_t memSize = getDataSize();
		m_buffer.resize(memSize);
	}

	Texture(const vec2i inSize)
		: m_size(inSize)
		, m_pitch(inSize.x)
	{
		size_t memSize = getDataSize();
		m_buffer.resize(memSize);
	}

	Texture(RawBuffer<uint8>* inData, vec2i inSize)
		: m_size(inSize)
		, m_pitch(inSize.x)
	{
		int32 targetSize = inSize.x * inSize.y * g_bytesPerPixel;
		assert(targetSize == inData->size());
		m_buffer.resize(inData->size());
		memcpy(m_buffer.data(), inData->data(), inData->size());
	}

	Texture(const Texture& other)
		: m_buffer(other.m_buffer)
		, m_size(other.m_size)
		, m_pitch(other.m_pitch) {}

	Texture(Texture&& other) noexcept
		: m_buffer(other.m_buffer)
		, m_size(other.m_size)
		, m_pitch(other.m_pitch) {}

	Texture& operator=(const Texture& other)
	{
		if (this == &other)
		{
			return *this;
		}
		m_buffer = other.m_buffer;
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
		m_buffer = other.m_buffer;
		m_size = other.m_size;
		m_pitch = other.m_pitch;
		return *this;
	}

	bool isValid()
	{
		return m_buffer.data() != nullptr;
	}

	void resize(const vec2i& inSize)
	{
		m_size = inSize;
		m_pitch = inSize.x;
		m_buffer.resize(getDataSize());
	}

	void resize(const vec2i& inSize, size_t dataSize)
	{
		m_size = inSize;
		m_pitch = inSize.x;
		m_buffer.resize(dataSize);
	}

	/**
	 * @brief Returns the raw void pointer to this texture's memory.
	 */
	[[nodiscard]] void* getRawData()
	{
		return (void*)m_buffer.data();
	}

	/**
	 * @brief Returns a type T (e.g. int32, float) pointer to this texture's memory.
	 */
	template <typename T = uint8>
	T* getData() const
	{
		return (T*)m_buffer.data();
	}

	void setData(RawBuffer<uint8>* newMemory, const size_t inSize = 0)
	{
		if (newMemory->data() == nullptr)
		{
			LOG_ERROR("Invalid memory allocation.")
			return;
		}
		auto size = inSize ? inSize : getDataSize();
		memcpy(m_buffer.data(), newMemory->data(), size);
	}

	/**
	 * @brief Returns the memory size of this texture in bytes.
	 * @return The number of bytes this texture allocates.
	 */
	[[nodiscard]] size_t getDataSize() const
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

	int32 getChannelCount()
	{
		return m_channelCount;
	}

	void setChannelCount(int32 count)
	{
		m_channelCount = count;
	}

	/**
	 * @brief Fills this texture with the specified color.
	 * @param inColor The color to fill this texture with.
	 */
	void fill(const Color& inColor)
	{
		int32* ptr = (int32*)m_buffer.data();
		int32  color = inColor.toInt32();
		size_t size = m_size.x * m_size.y;
		std::fill(ptr, ptr + size, color);
	}

	/**
	 * @brief Fills this texture with the specified float value.
	 * @note This is currently probably slower than it needs to be. std::memset & `std::fill` don't accept floats.
	 * @param value The value to fill this texture with.
	 */
	void fill(const float value)
	{
		auto  ptr = (float*)m_buffer.data();
		int32 size = m_size.x * m_size.y;
		std::fill(ptr, ptr + size, value);
	}

	void fillRow(int32 row, const Color& inColor)
	{
		int32* ptr = (int32*)m_buffer.data() + (row * m_pitch);
		int32  color = inColor.toInt32();
		std::fill(ptr, ptr + m_pitch, color);
	}

	/**
	 * @brief Returns a pointer to the beginning of a row of pixels.
	 * @param y The row to return.
	 * @return A type T pointer to the row of pixels.
	 */
	[[nodiscard]] uint32* scanline(const int y)
	{
		return (uint32*)m_buffer.data() + (y * m_pitch);
	}

	template <typename T>
	[[nodiscard]] T getPixel(const int32 x, const int32 y)
	{
		T* line = (T*)scanline(y);
		return line[x];
	}

	[[nodiscard]] Color getPixelAsColor(const int32 x, const int32 y)
	{
		uint32* line = scanline(y);
		uint32	v = line[x];
		return Color::fromUInt32(v);
	}

	[[nodiscard]] uint32 getPixelAsUInt32(const int32 x, const int32 y)
	{
		uint32* line = scanline(y);
		return line[x];
	}

	[[nodiscard]] float getPixelAsFloat(const int32 x, const int32 y)
	{
		uint32 pixel = getPixelAsUInt32(x, y);
		return *(reinterpret_cast<float*>(&pixel));
	}

	void setPixel(const int32 x, const int32 y, const uint8 color)
	{
		uint8* line = m_buffer.data() + (y * m_pitch);
		line[x] = color;
	}

	void setPixelFromColor(const int32 x, const int32 y, const Color& color)
	{
		auto line = (uint32*)m_buffer.data();
		line += (y * m_pitch);
		line[x] = color.toInt32();
	}

	void setPixelFromFloat(const int32 x, const int32 y, float value)
	{
		uint32* line = (uint32*)m_buffer.data();
		line += (y * m_pitch);
		auto*	castInt = reinterpret_cast<uint32*>(&value);
		line[x] = *castInt;
	}

	void setRow(const int32 row, const Color& color)
	{
		auto line = (uint32*)m_buffer.data();
		line += (row * m_pitch);
		int32 value = color.toInt32();
		std::fill(line, line + m_pitch, value);
	}

	// stbi__vertical_flip
	static void flipVertical(void* ptr, int32 width, int32 height)
	{
		size_t bytesPerRow = (size_t)width * g_bytesPerPixel;
		uint8  temp[2048];
		auto*  bytes = static_cast<uint8*>(ptr);

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
		Texture::flipVertical(m_buffer.data(), m_size.x, m_size.y);
	}

	// Swap the RGBA bytes for BGRA
	void setByteOrder(ETextureByteOrder newOrder)
	{
		size_t index = 0;
		size_t size = getDataSize();
		uint8* ptr = m_buffer.data();

		if (!ptr)
		{
			throw std::runtime_error("Texture data is malformed.");
		}

		switch (newOrder)
		{
			case ETextureByteOrder::RGBA:
			{
				while (index < size)
				{
					uint8 b = ptr[index];
					uint8 g = ptr[index + 1];
					uint8 r = ptr[index + 2];
					uint8 a = ptr[index + 3];

					ptr[index] = r;
					ptr[index + 1] = g;
					ptr[index + 2] = b;
					ptr[index + 3] = a;

					index += 4;
				}
				break;
			}
			case ETextureByteOrder::BRGA:
			{
				while (index < size - 4)
				{
					uint8 r = ptr[index];
					uint8 g = ptr[index + 1];
					uint8 b = ptr[index + 2];
					uint8 a = ptr[index + 3];

					ptr[index] = b;
					ptr[index + 1] = g;
					ptr[index + 2] = r;
					ptr[index + 3] = a;

					index += 4;
				}
				break;
			}
			default:
				break;
		}

		m_byteOrder = newOrder;
	}

	void addAlphaChannel() {}
};
