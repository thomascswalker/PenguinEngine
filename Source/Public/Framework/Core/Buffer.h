#pragma once

#include "Math/MathFwd.h"

enum EWhence : uint8
{
	Start,
	Current,
	End
};

struct Buffer
{
	void* data = nullptr;
	int32 size = 0;
	int32 pos = 0;

	explicit Buffer(void* inData, const int32 inSize) : data(inData), size(inSize) {}

	void seek(const int32 offset, const EWhence whence = Start)
	{
		switch (whence)
		{
		case Start:
			{
				pos = 0;
				break;
			}
		case Current:
			{
				pos += offset;
				break;
			}
		case End:
			{
				pos = size - offset;
				break;
			}
		}
	}

	template <typename T>
	[[nodiscard]] T read()
	{
		T value = *static_cast<T*>(data);
		pos += sizeof(T);
		return value;
	}

	int8 readInt8()
	{
		return read<int8>();
	}

	int16 readInt16()
	{
		return read<int16>();
	}

	int32 readInt32()
	{
		return read<int32>();
	}

	int64 readInt64()
	{
		return read<int64>();
	}

	uint8 readUInt8()
	{
		return read<uint8>();
	}

	uint16 readUInt16()
	{
		return read<uint16>();
	}

	uint32 readUInt32()
	{
		return read<uint32>();
	}

	uint64 readUInt64()
	{
		return read<uint64>();
	}
};
