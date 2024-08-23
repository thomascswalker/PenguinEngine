#include "Framework/Importers/ImageImporter.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool PngImporter::isValidHeader(ByteReader& reader)
{
	int32 isValid = 0;
	for (uint8 value : g_magicPng)
	{
		isValid |= reader.readUInt8() == value;
	}
	return (bool)isValid;
}

bool PngImporter::parseIHDR(const PngChunk* chunk, PngMetadata* metadata)
{
	ByteReader reader(chunk->rawData, chunk->size);

	metadata->width = swapByteOrder(reader.readInt32());                    // 4
	metadata->height = swapByteOrder(reader.readInt32());                   // 8
	metadata->bitDepth = reader.readInt8();                                 // 9
	metadata->colorType = (EPngColorType)reader.readInt8();                 // 10
	metadata->compressionMethod = (EPngCompressionMethod)reader.readInt8(); // 11
	metadata->filterMethod = reader.readInt8();                             // 12
	metadata->interlaceMethod = reader.readInt8();                          // 13

	// Compute the channel count from the color type
	int8 color = (int8)metadata->colorType;
	metadata->channelCount = (color & 2 ? 3 : 1) + (color & 4 ? 1 : 0);

	return true;
}

bool PngImporter::parseIDAT(PngChunk* chunk, const PngMetadata* metadata)
{
	uint32 bytesPerLine = (metadata->width * metadata->bitDepth + 7) / 8;
	chunk->size = bytesPerLine * metadata->height * metadata->channelCount + metadata->height;
	ByteReader reader(chunk->rawData, chunk->size);

	switch (metadata->compressionMethod)
	{
	// TODO: Account for zlib compression here
	// Each pixel row starts with a single byte which determines the filter type for that row
	case EPngCompressionMethod::Default:
		{
			uint32 sizeDataCompressed = chunk->size;
			//uLongf sizeDataUncompressed = chunk->size * 4;
			//auto dataUncompressed = (BYTE*)std::malloc(sizeDataUncompressed);
			//uncompress(dataUncompressed, &sizeDataUncompressed, chunk->rawData, sizeDataCompressed);
			break;
		}
	default:
		{
			return false;
		}
	}

	return true;
}

bool PngImporter::readChunk(ByteReader* reader, PngChunk* chunk, PngMetadata* metadata)
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

int32 PngImporter::import(const std::string& fileName, Bitmap& bitmap)
{
	// Read the file into a buffer
	std::string data;
	if (!IO::readFile(fileName, data))
	{
		LOG_ERROR("Unable to read file {}", fileName)
		return g_invalidImageIndex;
	}

	// Create a reader from the string rawData
	ByteReader reader(data, data.size(), std::endian::big);

	// Validate the header is the correct PNG header
	if (!isValidHeader(reader))
	{
		LOG_ERROR("Unable to load bitmap.")
		LOG_ERROR("Invalid PNG header in file {}.", fileName)
		return g_invalidImageIndex;
	}

	// Read and parse the IHDR chunk. This is always the first one.
	PngBitmap png;
	PngChunk ihdrChunk;
	if (!readChunk(&reader, &ihdrChunk, &png.metadata))
	{
		LOG_ERROR("Error reading IHDR chunk.")
		return g_invalidImageIndex;
	}
	parseIHDR(&ihdrChunk, &png.metadata);
	png.data.resize(png.metadata.width, png.metadata.height);

	// Read chunks until we hit the end of the file.
	bool atEnd = false;
	while (!atEnd)
	{
		PngChunk chunk;
		readChunk(&reader, &chunk, &png.metadata);

		// If we reach the end chunk, exit
		switch (chunk.type)
		{
		case EPngChunkType::IHDR:
			{
				break;
			}
		case EPngChunkType::IDAT:
			{
				parseIDAT(&chunk, &png.metadata);
				memcpy(png.data.getData(), chunk.data, chunk.size);
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

	bitmap = Bitmap::fromData(png.data.getData(), png.metadata.width, png.metadata.height);
	g_bitmaps.emplace_back(bitmap);
	return (int32)g_bitmaps.size() - 1;
}

int32 PngImporter::importStb(const std::string& fileName, Bitmap& bitmap)
{
	int32 x;
	int32 y;
	int32 channelCount;
	uint8* data = stbi_load(fileName.c_str(), &x, &y, &channelCount, 4);

#if defined(_WIN32) || defined(_WIN64)
	// Flip the image vertically because our display buffer is flipped vertically
	stbi__vertical_flip(data, x, y, 4);
	int32 size = x * y * 4;

	// Swap the RGBA bytes for BRGA (RGB => BRG)
	for (int32 i = 0; i < size; i += 4)
	{
		uint8 r = data[i];
		uint8 g = data[i + 1];
		uint8 b = data[i + 2];
		uint8 a = data[i + 3];

		data[i] = b;     // blue
		data[i + 1] = r; // red
		data[i + 2] = g; // green
		data[i + 3] = a;
	}
#endif

	// If we weren't able to load the image, return a negative index
	if (!data)
	{
		return g_invalidImageIndex;
	}

	bitmap = Bitmap::fromData(data, x, y);
	g_bitmaps.emplace_back(bitmap);
	return (int32)g_bitmaps.size() - 1;
}
