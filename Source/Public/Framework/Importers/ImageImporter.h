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
	Default = 0 // ZLib
};

enum class EPngChunkType
{
	// Required
	IHDR,
	PLTE,
	IDAT,
	IEND,

	// Optional
	BKGD,
	CHRM,
	CICP,
	DSIG,
	EXIF,
	GAMA,
	HIST,
	ICCP,
	ITXT,
	PHYS,
	SBIT,
	SPLT,
	SRGB,
	STER,
	TEXT,
	TIME,
	TRNS,
	ZTXT
};

inline std::map<std::string, EPngChunkType> g_pngChunkTypeMap =
{
	{"IHDR", EPngChunkType::IHDR},
	{"PLTE", EPngChunkType::PLTE},
	{"IDAT", EPngChunkType::IDAT},
	{"IEND", EPngChunkType::IEND},
	{"bKGD", EPngChunkType::BKGD},
	{"cHRM", EPngChunkType::CHRM},
	{"cICP", EPngChunkType::CICP},
	{"dSIG", EPngChunkType::DSIG},
	{"eXIf", EPngChunkType::EXIF},
	{"gAMA", EPngChunkType::GAMA},
	{"hIST", EPngChunkType::HIST},
	{"iCCP", EPngChunkType::ICCP},
	{"iTXt", EPngChunkType::ITXT},
	{"pHYs", EPngChunkType::PHYS},
	{"sBIT", EPngChunkType::SBIT},
	{"sPLT", EPngChunkType::SPLT},
	{"sRGB", EPngChunkType::SRGB},
	{"sTER", EPngChunkType::STER},
	{"tEXt", EPngChunkType::TEXT},
	{"tIME", EPngChunkType::TIME},
	{"tRNS", EPngChunkType::TRNS},
	{"zTXt", EPngChunkType::ZTXT}
};

struct PngChunk
{
	uint8* rawData;
	uint8* data;

	uint32 size; // The size of rawData in the chunk. This does not include the size of the header.
	EPngChunkType type;
};

struct PngMetadata
{
	uint32 width;   // Width of the image in pixels.
	uint32 height;  // Height of the image in pixels.
	uint8 bitDepth; // 1, 2, 4, 8, or 16 bits/channel
	EPngColorType colorType;
	EPngCompressionMethod compressionMethod;
	uint8 filterMethod;
	uint8 interlaceMethod;

	uint8 channelCount; // Number of rawData channels per pixel (1,2,3,4).
	uint8 pixelDepth;   // Number of bits per channel.
	uint8 spareByte;    // To align the rawData.
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

	static bool parseIHDR(const PngChunk* chunk, PngMetadata* metadata)
	{
		BufferedReader reader(chunk->rawData, chunk->size);

		metadata->width = swapByteOrder(reader.readInt32());                    // 4
		metadata->height = swapByteOrder(reader.readInt32());                   // 8
		metadata->bitDepth = reader.readInt8();                                 // 9
		metadata->colorType = (EPngColorType)reader.readInt8();                 // 10
		metadata->compressionMethod = (EPngCompressionMethod)reader.readInt8(); // 11
		metadata->filterMethod = reader.readInt8();                             // 12
		metadata->interlaceMethod = reader.readInt8();                          // 13

		return true;
	}

	static bool parseIDAT(PngChunk* chunk, const PngMetadata* metadata)
	{
		BufferedReader reader(chunk->rawData, chunk->size);
		chunk->data = (uint8*)PlatformMemory::alloc(chunk->size);
		uint32 chunkByteSize = chunk->size;
		switch (metadata->compressionMethod)
		{
		// TODO: Account for zlib compression here
		// Each pixel row starts with a single byte which determines the filter type for that row
		case EPngCompressionMethod::Default:
			{
				for (uint32 index = 0; index < chunkByteSize; index++)
				{
					chunk->data[index] = reader.readUInt8();
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

	static bool readChunk(BufferedReader* reader, PngChunk* chunk, PngMetadata* metadata)
	{
		// Read the size of the rawData, in bytes, in this chunk
		chunk->size = reader->readUInt32();

		// Read each char of the name
		int8 name0 = reader->readInt8();
		int8 name1 = reader->readInt8();
		int8 name2 = reader->readInt8();
		int8 name3 = reader->readInt8();

		// Determine the chunk type
		std::string name;
		name.push_back(name0);
		name.push_back(name1);
		name.push_back(name2);
		name.push_back(name3);
		chunk->type = g_pngChunkTypeMap[name];

		// Allocate the memory for this chunk's rawData, given the rawData size
		chunk->rawData = (uint8*)PlatformMemory::alloc(chunk->size);

		// Loop through and read all bytes
		for (uint32 i = 0; i < chunk->size; i++)
		{
			chunk->rawData[i] = reader->readUInt8();
		}

		// TODO: Actually calculate the CRC and validate it
		auto crc = reader->readUInt32();

		// If a name argument was passed in, verify the name we read above is the same.
		return true;
	}

public:
	static bool import(const std::string& fileName, Bitmap& bitmap)
	{
		// Read the file into a buffer
		std::string data;
		if (!IO::readFile(fileName, data))
		{
			LOG_ERROR("Unable to read file {}", fileName)
			return false;
		}

		// Create a reader from the string rawData
		BufferedReader reader(data, data.size(), std::endian::big);

		// Validate the header is the correct PNG header
		if (!isValidHeader(reader))
		{
			LOG_ERROR("Unable to load bitmap.")
			LOG_ERROR("Invalid PNG header in file {}.", fileName)
			return false;
		}

		// Read and parse the IHDR chunk. This is always the first one.
		PngMetadata metadata;
		PngChunk ihdrChunk;
		if (!readChunk(&reader, &ihdrChunk, &metadata))
		{
			LOG_ERROR("Error reading IHDR chunk.")
			return false;
		}
		parseIHDR(&ihdrChunk, &metadata);

		// Read chunks until we hit the end of the file.
		bool atEnd = false;
		while (!atEnd)
		{
			PngChunk chunk;
			readChunk(&reader, &chunk, &metadata);

			// If we reach the end chunk, exit
			switch (chunk.type)
			{
			case EPngChunkType::IHDR:
				{
					break;
				}
			case EPngChunkType::IDAT:
				{
					parseIDAT(&chunk, &metadata);
					bitmap = Bitmap::fromData(chunk.rawData, metadata.width, metadata.height);
					break;
				}

			case EPngChunkType::IEND:
				{
					atEnd = true;
					break;
				}
			case EPngChunkType::PLTE:
			case EPngChunkType::BKGD:
			case EPngChunkType::CHRM:
			case EPngChunkType::CICP:
			case EPngChunkType::DSIG:
			case EPngChunkType::EXIF:
			case EPngChunkType::GAMA:
			case EPngChunkType::HIST:
			case EPngChunkType::ICCP:
			case EPngChunkType::ITXT:
			case EPngChunkType::PHYS:
			case EPngChunkType::SBIT:
			case EPngChunkType::SPLT:
			case EPngChunkType::SRGB:
			case EPngChunkType::STER:
			case EPngChunkType::TEXT:
			case EPngChunkType::TIME:
			case EPngChunkType::TRNS:
			case EPngChunkType::ZTXT:
				{
					break;
				}
			}
		}

		return true;
	}
};
