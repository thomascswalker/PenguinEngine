#pragma once

#include "Math/MathFwd.h"
#include "Framework/Platforms/PlatformMemory.h"

#if defined(_WIN32) || defined(_WIN64)
#define ZLIB_WINAPI
#endif

#include "zlib/zlib.h"

constexpr int32 g_ZlibChunk = 16384;
constexpr int32 g_defaultZlibBitWindow = MAX_WBITS;

namespace Compression
{
	static voidpf zalloc(void* opaque, uint32 size, uint32 num)
	{
		return PlatformMemory::malloc(static_cast<size_t>(size) * num);
	}

	static void zfree(void* opaque, void* p)
	{
		PlatformMemory::free(p);
	}

	static bool uncompressZlib(Buffer<uint8>* uncompressedBuffer, Buffer<uint8>* compressedBuffer)
	{
		uint32 uncompressedSize = uncompressedBuffer->getSize();
		uint8* uncompressedData = PlatformMemory::malloc<uint8>(uncompressedSize);

		z_stream stream;
		stream.zalloc = &zalloc;
		stream.zfree = &zfree;
		stream.opaque = nullptr;
		stream.next_in = compressedBuffer->getPtr();
		stream.avail_in = (uInt)compressedBuffer->getSize();
		stream.next_out = uncompressedData;
		stream.avail_out = uncompressedSize;

		if (inflateInit2(&stream, g_defaultZlibBitWindow) != Z_OK)
		{
			return false;
		}

		int32 result = 0;
		while (result != Z_STREAM_END)
		{
			result = inflate(&stream, Z_NO_FLUSH);
			if (result == Z_STREAM_ERROR)
			{
				inflateEnd(&stream);
				return false;
			}
			//size_t have = uncompressedBuffer->getSize() - stream.avail_out;
			//memcpy(uncompressedBuffer->getPtr(), )
		}
		result = inflateEnd(&stream);

		memcpy(uncompressedBuffer->getPtr(), uncompressedData, uncompressedSize);

		return result == Z_OK;
	}
}
