#pragma once

#include "Math/MathFwd.h"
#include "Platforms/Generic/GenericMemory.h"

#include "zlib/zlib.h"

#if defined(_WIN32) || defined(_WIN64)
#define ZLIB_WINAPI
#endif

constexpr int32 g_zlibChunk            = 16384;
constexpr int32 g_defaultZlibBitWindow = MAX_WBITS;

namespace Compression
{
	static voidpf zalloc(void* opaque, const uint32 size, const uint32 num)
	{
		return ApplicationMemory::malloc(static_cast<size_t>(size) * num);
	}

	static void zfree(void* opaque, void* p)
	{
		ApplicationMemory::free(p);
	}

	static int32 uncompressZlib(RawBuffer<uint8>* uncompressedBuffer, RawBuffer<uint8>* compressedBuffer)
	{
		uint32 uncompressedSize = (uint32)uncompressedBuffer->size();
		uint8* uncompressedData = ApplicationMemory::malloc<uint8>(uncompressedSize);

		z_stream stream;
		stream.zalloc    = &zalloc;
		stream.zfree     = &zfree;
		stream.opaque    = nullptr;
		stream.next_in   = compressedBuffer->data();
		stream.avail_in  = (uInt)compressedBuffer->size();
		stream.next_out  = uncompressedData;
		stream.avail_out = uncompressedSize;

		int32 result = inflateInit2(&stream, g_defaultZlibBitWindow);
		if (result != Z_OK)
		{
			return result;
		}

		while (result != Z_STREAM_END)
		{
			result = inflate(&stream, Z_NO_FLUSH);
			switch (result)
			{
			case Z_ERRNO:
			case Z_STREAM_ERROR:
			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
			case Z_BUF_ERROR:
			case Z_VERSION_ERROR:
				{
					LOG_ERROR("ZLib error: {}", result)
					inflateEnd(&stream);
					return result;
				}
			default:
			case Z_OK:
				{
					break;
				}
			}
		}
		result = inflateEnd(&stream);

		memcpy(uncompressedBuffer->data(), uncompressedData, uncompressedSize);

		return result;
	}
} // namespace Compression
