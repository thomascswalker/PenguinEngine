#pragma once

#include <memory>
#include <streambuf>
#include <bit>
#include <cassert>

#include "Platforms/Generic/PlatformMemory.h"

inline uint8 g_bitsPerByte = 8;

/**
 * @brief Swaps the specified value's byte order.
 *
 * 01234567 <=> 76543210
 *
 * @param value The value to swap the order on.
 * @return T The swapped value.
 */
template <typename T> T swapByteOrder(T value)
{
	T	result = 0;
	int size = sizeof(T) - 1;
	for (int i = 0; i <= size; i++)
	{
		result = result | ((value >> (i * 8) & 0xFF) << (size - i) * 8);
	}

	return result;
}

/**
 * @brief This class stores data of type T with a specified size. The management of this class' memory is done
 * with PlatformMemory functions.
 */
template <typename T> class RawBuffer
{
	T* m_data = nullptr;
	/* Size of this buffer in bytes. */
	size_t m_size = 0;

public:
	RawBuffer() = default;

	explicit RawBuffer(const size_t inSize) : m_size(inSize) { m_data = PlatformMemory::malloc<T>(inSize); }

	explicit RawBuffer(T* inData, const size_t inSize) : m_data(inData), m_size(inSize)
	{
		m_data = PlatformMemory::malloc<T>(inSize);
		std::memcpy(m_data, inData, inSize);
	}

	// Copy constructor
	RawBuffer(const RawBuffer& other) : m_size(other.m_size)
	{
		if (other.m_data)
		{
			m_data = PlatformMemory::malloc<T>(m_size);
			std::memcpy(m_data, other.m_data, m_size);
		}
		else
		{
			m_data = nullptr;
			m_size = 0;
		}
	}

	// Move constructor
	RawBuffer(RawBuffer&& other) noexcept
	{
		m_data = other.m_data;
		m_size = other.m_size;
		other.m_data = nullptr;
		other.m_size = 0;
	}

	// Assignment operator
	RawBuffer& operator=(const RawBuffer& other)
	{
		if (!(*this == other))
		{
			PlatformMemory::free(m_data);
			if (other.m_data)
			{
				m_size = other.m_size;
				m_data = PlatformMemory::malloc<T>(m_size);
				std::memcpy(m_data, other.m_data, m_size);
			}
			else
			{
				m_data = nullptr;
				m_size = 0;
			}
		}

		return *this;
	}

	RawBuffer& operator=(RawBuffer&& other) noexcept
	{
		if (!(*this == other))
		{
			PlatformMemory::free(m_data);
			m_data = other.m_data;
			m_size = other.m_size;
			other.m_data = nullptr;
			other.m_size = 0;
		}
		return *this;
	}

	T* data() { return m_data; }

	T* data() const { return m_data; }

	void setPtr(T* ptr) { m_data = ptr; }

	[[nodiscard]] size_t size() const { return m_size; }

	void resize(const size_t inSize)
	{
#ifdef _DEBUG
		assert(inSize < UINT32_MAX);
#endif
		if (m_data != nullptr)
		{
			PlatformMemory::free(m_data);
			m_data = nullptr;
		}
		m_size = inSize;
		m_data = (T*)PlatformMemory::malloc(inSize);
#ifdef _DEBUG
		assert(m_data != nullptr);
#endif
	}

	void resize(const uint32 width, const uint32 height)
	{
		m_size = width * height * g_bytesPerPixel;
#ifdef _DEBUG
		assert(m_size < UINT32_MAX);
#endif
		if (m_data != nullptr)
		{
			PlatformMemory::free(m_data);
			m_data = nullptr;
		}
		m_data = (T*)PlatformMemory::malloc(m_size);
#ifdef _DEBUG
		assert(m_data != nullptr);
#endif
	}

	void extend(const size_t addSize)
	{
		// Store the old size
		size_t oldSize = m_size;

		// Allocate a temporary buffer for the current data
		void* tmp = PlatformMemory::malloc(oldSize);

		// Copy current data into the temp buffer
		std::memcpy(tmp, m_data, oldSize);

		// Add the new size to the current size
		m_size += addSize;

		// Free the old data
		PlatformMemory::free(m_data);

		// Create a new array with the new size
		m_data = (T*)PlatformMemory::malloc(m_size);

		// Copy the old data into the new buffer
		std::memcpy(m_data, tmp, oldSize);
	}

	void clear()
	{
		PlatformMemory::free(m_data);
		m_size = 0;
		m_data = nullptr;
	}

	bool isValid() const { return m_data != nullptr && m_size != 0; }

	T* begin() { return m_data; }

	[[nodiscard]] const T* begin() const { return m_data; }

	T* end() { return m_data + m_size; }

	[[nodiscard]] const T* end() const { return m_data + m_size; }

	T& operator[](size_t index) { return m_data[index]; }

	const T& operator[](size_t index) const { return m_data[index]; }

	bool operator==(const RawBuffer& other) const
	{
		if (m_size != other.m_size)
		{
			return false;
		}

		if (std::memcmp(m_data, other.m_data, m_size) != 0)
		{
			return false;
		}

		return true;
	}
};

enum class ESeekDir : int32
{
	Beginning = std::ios_base::beg,
	Current = std::ios_base::cur,
	End = std::ios_base::end,
};

class ByteReader
{
	std::unique_ptr<RawBuffer<uint8>> m_buffer;
	/* Position of the cursor. */
	int32 m_pos = 0;
	/* Size of the entire buffer. */
	size_t m_size = 0;
	/* Endian type this buffer reads bytes by. */
	std::endian m_endian = std::endian::native;

	// Bit reading
	uint8 m_bitCount = 0;
	uint8 m_codeBuffer = 0;

	uint8 m_bitPos = 0;
	uint8 m_currentByte = 0;

	template <typename T> T read(size_t size)
	{
		// Reset the bit position
		m_bitPos = 0;

		// Get pointer to the current offset
		auto ptr = m_buffer->data() + m_pos;

		// Reinterpret cast pointer to type T
		T value = *reinterpret_cast<T*>(ptr);

		// Increment the position by the size of type T
		m_pos += (int32)size;

		// If this buffer is using the non-native endian format, swap the byte order
		if (std::endian::native != m_endian)
		{
			return swapByteOrder(value);
		}

		// Return the value
		return value;
	}

	template <typename T> T peek(size_t size)
	{
		// Reset the bit position
		m_bitPos = 0;

		// Get pointer to the current offset
		auto ptr = m_buffer->data() + m_pos;

		// Reinterpret cast pointer to type T
		T value = *reinterpret_cast<T*>(ptr);

		// If this buffer is using the non-native endian format, swap the byte order
		if (std::endian::native != m_endian)
		{
			return swapByteOrder(value);
		}

		// Return the value
		return value;
	}

public:
	ByteReader() = default;

	ByteReader(std::string& inString, const size_t inSize, const std::endian endian = std::endian::native) : m_size(inSize), m_endian(endian)
	{
		m_buffer = std::make_unique<RawBuffer<uint8>>((uint8*)inString.data(), inSize);
	}

	explicit ByteReader(uint8* inBuffer, const size_t inSize, const std::endian endian = std::endian::native) : m_size(inSize), m_endian(endian)
	{
		m_buffer = std::make_unique<RawBuffer<uint8>>(inBuffer, inSize);
	}

	explicit ByteReader(RawBuffer<uint8>& inBuffer, const std::endian endian = std::endian::native) : m_size(inBuffer.size()), m_endian(endian)
	{
		m_buffer = std::make_unique<RawBuffer<uint8>>(inBuffer);
	}

	~ByteReader() = default;

	[[nodiscard]] int32 getPos() const { return m_pos; }

	[[nodiscard]] int32 getBitPos() const { return (g_bitsPerByte * m_pos) + m_bitPos; }

	[[nodiscard]] size_t getSize() const { return m_size; }

	uint8* ptr() { return m_buffer->data() + m_pos; }

	int8 readInt8() { return read<int8>(1); }

	int16 readInt16() { return read<int16>(2); }

	int32 readInt32() { return read<int32>(4); }

	int64 readInt64() { return read<int64>(8); }

	uint8 readUInt8() { return read<uint8>(1); }

	uint8 peekUInt8() { return peek<uint8>(1); }

	uint16 readUInt16() { return read<uint16>(2); }

	uint32 readUInt32() { return read<uint32>(4); }

	uint64 readUInt64() { return read<uint64>(8); }

	template <typename T> void readSize(size_t size, std::vector<T>& buffer)
	{
		for (int32 i = 0; i < size; i++)
		{
			buffer.emplace_back(read<T>(sizeof(T)));
		}
	}

	std::string readString(size_t size, int32 step = 0)
	{
		std::string out;
		for (int32 i = 0; i < size; i++)
		{
			unsigned char c = readUInt8();
			out.push_back(c);
		}
		return out;
	}

	void fillBits()
	{
		while (m_bitCount <= 24)
		{
			if (m_codeBuffer >= (1U << m_bitCount))
			{
				return; // EOF
			}
			m_codeBuffer |= (uint32)readInt8() << m_bitCount;
			m_bitCount += 8;
		}
	}

	uint8 readBits(const int32 count)
	{
		if (m_bitCount < count)
		{
			fillBits();
		}
		auto outValue = (uint8)(m_codeBuffer & (1 << count) - 1);
		m_codeBuffer >>= count;
		m_bitCount -= count;
		return outValue;
	}

	uint32 seek(const int32 offset, ESeekDir seekDir = ESeekDir::Current)
	{
		switch (seekDir)
		{
			case ESeekDir::Beginning:
				m_pos = offset;
				break;
			case ESeekDir::Current:
				m_pos += offset;
				break;
			case ESeekDir::End:
				m_pos = (int32)m_size - offset;
				break;
		}
		return m_pos;
	}

	uint8* next() { return m_buffer->data() + m_pos + 1; }

	bool canSeek(const int32 offset) const { return m_pos + offset < m_size; }
};

namespace Algorithm
{
	inline void resizeNearestNeighbor(const uint8* input, uint8* output, int32 sourceWidth, int32 sourceHeight, int32 targetWidth, int32 targetHeight)
	{
		const int32 x_ratio = (int32)((sourceWidth << 16) / targetWidth);
		const int32 y_ratio = (int32)((sourceHeight << 16) / targetHeight);

		int32 x_ratio_with_color = x_ratio;

		for (int32 y = 0; y < targetHeight; y++)
		{
			int32 y2_xsource = ((y * y_ratio) >> 16) * sourceWidth;
			int32 i_xdest = y * targetWidth;

			int32			source_x_offset = 0;
			int32			startingOffset = y2_xsource;
			const uint8* inputLine = input + startingOffset;
			for (int32 x = 0; x < targetWidth; x++)
			{
				i_xdest += 1;
				source_x_offset += x_ratio_with_color;
				int32 sourceOffset = source_x_offset >> 16;

				output[i_xdest] = inputLine[sourceOffset];
			}
		}
	}
} // namespace Algorithm