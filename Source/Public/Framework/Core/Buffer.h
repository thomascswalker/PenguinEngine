#pragma once

#include <memory>
#include <streambuf>
#include <fstream>

#include "Math/MathFwd.h"

template <typename T>
T swapEndian(T value)
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
	explicit Buffer(T* inData, const size_t inSize)
		: data(inData), size(inSize) {}

	T* getData()
	{
		return data;
	}

	[[nodiscard]] size_t getSize() const
	{
		return size;
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
		auto begin = (int8*)buffer.getData();
		this->setg(begin, begin, begin + buffer.getSize());
	}
};

class BufferedReader
{
	int32 m_pos = 0;
	std::endian m_endian;
	std::unique_ptr<StreamBuffer> m_streamBuffer = nullptr;
	std::unique_ptr<std::istream> m_stream = nullptr;

	template <typename T>
	T read(const size_t size)
	{
		T value;
		m_stream->read(reinterpret_cast<int8*>(&value), (int64)size);
		m_pos += size;
		if (std::endian::native != m_endian)
		{
			return swapEndian(value);
		}
		return value;
	}

public:
	BufferedReader(std::string& inString, const size_t inSize,
	               const std::endian endian = std::endian::native) : m_endian(endian)
	{
		Buffer buffer((uint8*)inString.data(), inSize);
		m_streamBuffer = std::make_unique<StreamBuffer>(buffer);
		m_stream = std::make_unique<std::istream>(m_streamBuffer.get(), false);
	}

	explicit BufferedReader(uint8* inBuffer, const size_t inSize,
	                        const std::endian endian = std::endian::native) : m_endian(endian)
	{
		Buffer buffer(inBuffer, inSize);
		m_streamBuffer = std::make_unique<StreamBuffer>(buffer);
		m_stream = std::make_unique<std::istream>(m_streamBuffer.get(), false);
	}

	explicit BufferedReader(Buffer<uint8>& inBuffer, const std::endian endian = std::endian::native) : m_endian(endian)
	{
		m_streamBuffer = std::make_unique<StreamBuffer>(inBuffer);
		m_stream = std::make_unique<std::istream>(m_streamBuffer.get(), false);
	}

	[[nodiscard]] int32 getPos() const
	{
		return m_pos;
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
};
