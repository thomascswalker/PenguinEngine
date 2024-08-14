#pragma once
#include <bit>
#include "Math/MathFwd.h"
#include "Framework/Core/Bitmask.h"
#include "Framework/Core/IO.h"
#include "Framework/Core/Buffer.h"
#include "Framework/Renderer/Bitmap.h"

/** https://en.wikipedia.org/wiki/PNG */
inline uint8 g_magicPng[8] = {137, 'P', 'N', 'G', 13, 10, 26, 10};

enum class EPngColorType : uint8
{
	None    = 0,
	Palette = 1,
	Color   = 2,
	Alpha   = 4
};

DEFINE_BITMASK_OPERATORS(EPngColorType)

namespace PngChunkTypes
{
	inline auto IHDR = "IHDR";
	inline auto IEND = "IEND";
	inline auto IDAT = "IDAT";
}

enum class EPngCompressionMethod
{
	Default = 0
};

struct PngChunk
{
	/** The length of data in the chunk. This does not include the length of the header. */
	uint8* data;
	uint8* uncompressedData;
	uint32 length;
	std::string name;
	uint32 width;
	uint32 height;
	uint8 bitDepth;
	EPngColorType colorType;
	uint8 compressionMethod;
	uint8 filterMethod;
	uint8 interlaceMethod;
};

/** https://www.nayuki.io/page/png-file-chunk-inspector */
class PngImporter
{
	static bool isValidHeader(BufferedReader& reader)
	{
		int32 isValid = 0;
		for (uint8 value : g_magicPng)
		{
			isValid |= reader.readUInt8() == value;
		}
		return (bool)isValid;
	}

	static bool parseIHDR(PngChunk* chunk)
	{
		BufferedReader reader(chunk->data, chunk->length);

		// 13 bytes
		chunk->width = reader.readInt32();                   // 4
		chunk->height = reader.readInt32();                  // 8
		chunk->bitDepth = reader.readInt8();                 // 9
		chunk->colorType = (EPngColorType)reader.readInt8(); // 10
		chunk->compressionMethod = reader.readInt8();        // 11
		chunk->filterMethod = reader.readInt8();             // 12
		chunk->interlaceMethod = reader.readInt8();          // 13

		return true;
	}

	static bool parseIDAT(PngChunk* chunk, const EPngCompressionMethod compressionMethod)
	{
		BufferedReader reader(chunk->data, chunk->length);
		chunk->uncompressedData = (uint8*)PlatformMemory::alloc(chunk->length);

		switch (compressionMethod)
		{
		// Uncompressed
		case EPngCompressionMethod::Default:
			{
				for (int32 index = 0; index < chunk->length;)
				{
					chunk->uncompressedData[index++] = reader.readUInt8();
					chunk->uncompressedData[index++] = reader.readUInt8();
					chunk->uncompressedData[index++] = reader.readUInt8();
					chunk->uncompressedData[index++] = reader.readUInt8();
				}
				break;
			}
		default:
			{
				return false;
			}
		}

		return true;
	}

	static bool readChunk(BufferedReader& reader, PngChunk* chunk, const std::string& name = "")
	{
		chunk->length = reader.readUInt32();

		int8 name0 = reader.readInt8();
		int8 name1 = reader.readInt8();
		int8 name2 = reader.readInt8();
		int8 name3 = reader.readInt8();

		chunk->name.push_back(name0);
		chunk->name.push_back(name1);
		chunk->name.push_back(name2);
		chunk->name.push_back(name3);

		chunk->data = (uint8*)PlatformMemory::alloc(chunk->length);
		for (uint32 i = 0; i < chunk->length; i++)
		{
			chunk->data[i] = reader.readUInt8();
		}

		// TODO: Actually calculate the CRC and validate it
		auto crc = reader.readUInt32();

		auto pos = reader.getPos();

		if (!name.empty())
		{
			return chunk->name == name;
		}
		return true;
	}

public:
	static bool import(const std::string& fileName, Bitmap* bitmap)
	{
		// Read the file into a buffer
		std::string data;
		if (!IO::readFile(fileName, data))
		{
			LOG_ERROR("Unable to read file {}", fileName)
			return false;
		}

		// Create a reader from the string data
		BufferedReader reader(data, data.size(), std::endian::big);

		// Validate the header is the correct PNG header
		if (!isValidHeader(reader))
		{
			LOG_ERROR("Unable to load bitmap.")
			LOG_ERROR("Invalid PNG header in file {}.", fileName)
			return false;
		}

		std::vector<PngChunk> chunks;

		// Read and parse the IHDR chunk. This is always the first one.
		PngChunk ihdrChunk;
		if (!readChunk(reader, &ihdrChunk, "IHDR"))
		{
			LOG_ERROR("Error reading IHDR chunk.")
			return false;
		}
		parseIHDR(&ihdrChunk);
		chunks.emplace_back(ihdrChunk);

		// Read chunks until we hit the end of the file.
		while (true)
		{
			PngChunk chunk;
			readChunk(reader, &chunk);

			// If we reach the end chunk, exit
			if (chunk.name == PngChunkTypes::IEND)
			{
				chunks.emplace_back(chunk);
				break;
			}

			// If we reach any other chunk other than a data chunk, just continue
			// TODO: Implement other chunk types
			if (chunk.name != PngChunkTypes::IDAT)
			{
				chunks.emplace_back(chunk);
				continue;
			}

			// At this point we should only be dealing with a data chunk, so parse it.
			parseIDAT(&chunk, (EPngCompressionMethod)ihdrChunk.compressionMethod);
			// TODO: This doesn't actually load the data correctly
			bitmap = Bitmap::fromData(chunk.uncompressedData, chunk.width, chunk.height);
			break;
		}

		return true;
	}
};
