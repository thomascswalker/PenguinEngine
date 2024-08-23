#pragma once

#include "Math/MathFwd.h"
#include "Framework/Platforms/PlatformMemory.h"

#if defined(_WIN32) || defined(_WIN64)
#define ZLIB_WINAPI
#endif

#include "zlib/zlib.h"

constexpr int32 g_defaultZlibBitWindow = 15;

namespace Compression
{
	static voidpf zalloc(void* opaque, unsigned int size, unsigned int num)
	{
		return PlatformMemory::malloc(size * num);
	}

	static void zfree(void* opaque, void* p)
	{
		PlatformMemory::free(p);
	}

	static bool uncompressZlib(Buffer<uint8>* uncompressedBuffer, Buffer<uint8>* compressedBuffer)
	{
		z_stream stream;
		stream.zalloc = &zalloc;
		stream.zfree = &zfree;
		stream.opaque = nullptr;
		stream.next_in = compressedBuffer->getData();
		stream.avail_in = (uint32)compressedBuffer->getSize();
		stream.next_out = uncompressedBuffer->getData();
		stream.avail_out = (uint32)uncompressedBuffer->getSize();

		int32 result = inflateInit2(&stream, g_defaultZlibBitWindow);

		if (result != Z_OK)
		{
			return false;
		}
		result = inflate(&stream, Z_FINISH);

		int32 endResult = inflateEnd(&stream);
		if (result >= Z_OK)
		{
			result = endResult;
		}

		return result == Z_OK;
	}
}
