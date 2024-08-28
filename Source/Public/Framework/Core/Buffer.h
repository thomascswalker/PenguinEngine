#pragma once

#include <memory>
#include <streambuf>
#include <fstream>

#include "Math/MathFwd.h"
#include "Framework/Platforms/PlatformMemory.h"

inline uint8 g_bitsPerByte = 8;

template <typename T>
T swapByteOrder(T value)
{
	T result = 0;
	int size = sizeof(T) - 1;
	for (int i = 0; i <= size; i++)
	{
		result = result | ((value >> (i * 8) & 0xFF) << (size - i) * 8);
	}

	return result;
}

template <typename T>
struct Buffer
{
private:
	T* data = nullptr;
	size_t size = 0;

public:
	Buffer() {}

	explicit Buffer(const size_t inSize) : size(inSize)
	{
		data = PlatformMemory::malloc<T>(inSize);
	}

	explicit Buffer(T* inData, const size_t inSize)
		: data(inData), size(inSize) {}

	~Buffer()
	{
		if (data)
		{
			PlatformMemory::free(data);
		}
		data = nullptr;
	}

	T* getPtr()
	{
		return data;
	}

	void setPtr(T* ptr)
	{
		data = ptr;
	}

	[[nodiscard]] size_t getSize() const
	{
		return size;
	}

	void resize(const size_t inSize)
	{
		if (data)
		{
			PlatformMemory::free(data);
		}
		size = inSize;
		data = PlatformMemory::malloc<T>(size);
	}

	void resize(const uint32 width, const uint32 height)
	{
		if (data)
		{
			PlatformMemory::free(data);
		}
		size = width * height * g_bitsPerPixel;
		data = PlatformMemory::malloc<T>(size);
	}

	T* begin()
	{
		return data;
	}

	[[nodiscard]] const T* begin() const
	{
		return data;
	}

	T* end()
	{
		return data + size;
	}

	[[nodiscard]] const T* end() const
	{
		return data + size;
	}

	T& operator[](size_t index)
	{
		return data[index];
	}

	const T& operator[](size_t index) const
	{
		return data[index];
	}
};

struct StreamBuffer : std::streambuf
{
	explicit StreamBuffer(Buffer<uint8>& buffer)
	{
		auto begin = (int8*)buffer.getPtr();
		this->setg(begin, begin, begin + buffer.getSize());
	}
};

enum class ESeekDir : int32
{
	Beginning = std::ios_base::beg,
	Current   = std::ios_base::cur,
	End       = std::ios_base::end,
};

class ByteReader
{
	int32 m_pos = 0;
	size_t m_size = 0;
	std::endian m_endian = std::endian::native;
	std::unique_ptr<StreamBuffer> m_streamBuffer = nullptr;
	std::unique_ptr<std::istream> m_stream = nullptr;

	// Bit reading
	uint8 m_bitCount = 0;
	uint8 m_codeBuffer = 0;

	uint8 m_bitPos = 0;
	uint8 m_currentByte = 0;

	template <typename T>
	T read(size_t size)
	{
		// Reset the bit position
		m_bitPos = 0;

		// Read compressedSize
		T value;
		m_stream->read(reinterpret_cast<int8*>(&value), (int64)size);
		m_pos += (int32)size;
		if (std::endian::native != m_endian)
		{
			return swapByteOrder(value);
		}
		return value;
	}

	template <typename T>
	T peek(const size_t size)
	{
		T value;
		m_stream->peek(reinterpret_cast<int8*>(&value), (int64)size);
		if (std::endian::native != m_endian)
		{
			return swapByteOrder(value);
		}
		return value;
	}

public:
	ByteReader() {}

	ByteReader(std::string& inString, const size_t inSize,
	           const std::endian endian = std::endian::native) : m_size(inSize), m_endian(endian)
	{
		Buffer buffer((uint8*)inString.data(), inSize);
		m_streamBuffer = std::make_unique<StreamBuffer>(buffer);
		m_stream = std::make_unique<std::istream>(m_streamBuffer.get(), false);
	}

	explicit ByteReader(uint8* inBuffer, const size_t inSize,
	                    const std::endian endian = std::endian::native) : m_size(inSize), m_endian(endian)
	{
		Buffer buffer(inBuffer, inSize);
		m_streamBuffer = std::make_unique<StreamBuffer>(buffer);
		m_stream = std::make_unique<std::istream>(m_streamBuffer.get(), false);
	}

	explicit ByteReader(Buffer<uint8>& inBuffer,
	                    const std::endian endian = std::endian::native) : m_size(inBuffer.getSize()), m_endian(endian)
	{
		m_streamBuffer = std::make_unique<StreamBuffer>(inBuffer);
		m_stream = std::make_unique<std::istream>(m_streamBuffer.get(), false);
	}

	[[nodiscard]] int32 getPos() const
	{
		return m_pos;
	}

	[[nodiscard]] int32 getBitPos() const
	{
		return (g_bitsPerByte * m_pos) + m_bitPos;
	}

	[[nodiscard]] size_t getSize() const
	{
		return m_size;
	}

	int8 readInt8()
	{
		return read<int8>(1);
	}

	int16 readInt16()
	{
		return read<int16>(2);
	}

	int32 readInt32()
	{
		return read<int32>(4);
	}

	int64 readInt64()
	{
		return read<int64>(8);
	}

	uint8 readUInt8()
	{
		return read<uint8>(1);
	}

	uint16 readUInt16()
	{
		return read<uint16>(2);
	}

	uint32 readUInt32()
	{
		return read<uint32>(4);
	}

	uint64 readUInt64()
	{
		return read<uint64>(8);
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
		uint8 outValue = (uint8)(m_codeBuffer & (1 << count) - 1);
		m_codeBuffer >>= count;
		m_bitCount -= count;
		return outValue;
	}

	uint32 seek(const int32 offset, ESeekDir seekDir = ESeekDir::Current)
	{
		m_pos += offset;
		m_stream->seekg(offset, (int32)seekDir);
		return m_pos;
	}
};
