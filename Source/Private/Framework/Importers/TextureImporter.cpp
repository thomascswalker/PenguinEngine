#include "Framework/Importers/TextureImporter.h"
#include "Framework/Core/Compression.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

constexpr int32 g_channelCount = 4;
#define TRUNCATE(width) ((uint8) ((width) & 255))

inline ETextureFileFormat TextureImporter::getTextureFileFormat(const std::string& fileName)
{
	if (fileName.ends_with(".png"))
	{
		return ETextureFileFormat::Png;
	}
	else if (fileName.ends_with(".bmp"))
	{
		return ETextureFileFormat::Bmp;
	}
	else if (fileName.ends_with(".jpg") || fileName.ends_with(".jpeg"))
	{
		return ETextureFileFormat::Bmp;
	}
	return ETextureFileFormat::Unknown;
}

bool TextureImporter::isValidPngHeader(ByteReader* reader)
{
	int32 isValid = 0;
	for (uint8 value : g_magicPng)
	{
		isValid |= reader->readUInt8() == value;
	}
	return (bool)isValid;
}

bool TextureImporter::parsePngIHDR(PngChunk* chunk, PngTexture* png)
{
	ByteReader reader(chunk->compressedBuffer);
	PngMetadata* metadata = &png->metadata;

	metadata->width = swapByteOrder(reader.readInt32());                    // 4
	metadata->height = swapByteOrder(reader.readInt32());                   // 8
	metadata->bitDepth = reader.readInt8();                                 // 9
	metadata->colorType = (EPngColorType)reader.readInt8();                 // 10
	metadata->compressionMethod = (EPngCompressionMethod)reader.readInt8(); // 11
	metadata->filterMethod = reader.readInt8();                             // 12
	metadata->interlaced = (bool)reader.readInt8();                          // 13

	// Compute the channel count from the color type
	int8 color = (int8)metadata->colorType;
	metadata->channelCount = (color & 2 ? 3 : 1) + (color & 4 ? 1 : 0);

	if (metadata->channelCount != 4)
	{
		LOG_ERROR("Currently only RGBA PNG files are supported.")
			return false;
	}

	return true;
}

bool TextureImporter::parsePngIDAT(PngChunk* chunk, PngTexture* png)
{
	PngMetadata* metadata = &png->metadata;

	// Compute the uncompressed size of the image
	uint32 bytesPerLine = (metadata->width * metadata->bitDepth + 7) / 8;
	uint32 uncompressedSize = bytesPerLine * metadata->height * metadata->channelCount + metadata->height;

	// Resize the uncompressed buffer to the uncompressed size
	chunk->uncompressedBuffer.resize((size_t)uncompressedSize);

	// Resize the final PNG data to the uncompressed size
	png->data.resize((size_t)uncompressedSize);

	// Based on the PNG spec, the only method available is the Default method (ZLib).
	switch (metadata->compressionMethod)
	{
	case EPngCompressionMethod::Default:
	{
		// Uncompress the whole compressedBuffer with ZLib's inflate
		bool result = Compression::uncompressZlib(&chunk->uncompressedBuffer, &chunk->compressedBuffer);
		if (!result)
		{
			return false;
		}
		// If interlaced, create the final PNG image data by deinterlacing, or if not, create
		// the final PNG image data from the raw uncompressed data.
		// This involves unfiltering each row.
		return metadata->interlaced ? createPngInterlaced(&chunk->uncompressedBuffer, png) : createPng(&chunk->uncompressedBuffer, png);
	}
	case EPngCompressionMethod::Invalid:
	default:
	{
		return false;
	}
	}
}

bool TextureImporter::readPngChunk(ByteReader* reader, PngChunk* chunk, PngMetadata* metadata)
{
	// Read the compressedSize of the uncompressedData, in bytes, in this chunk
	auto compressedSize = reader->readUInt32();
	chunk->compressedBuffer.resize(compressedSize);

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

	// Allocate the memory for this chunk's uncompressedData, given the uncompressedData compressedSize

	// Loop through and read all bytes
	for (uint32 i = 0; i < chunk->compressedBuffer.getSize(); i++)
	{
		chunk->compressedBuffer[i] = reader->readUInt8();
	}

	// TODO: Actually calculate the CRC and validate it
	auto crc = reader->readUInt32();

	// If a name argument was passed in, verify the name we read above is the same.
	return true;
}

bool TextureImporter::createPng(Buffer<uint8>* uncompressedBuffer, PngTexture* png)
{
	uint8* raw = uncompressedBuffer->getPtr();

	PngMetadata* metadata = &png->metadata;
	int32 color = (int32)metadata->colorType;
	int32 totalSize = uncompressedBuffer->getSize();
	int32 depth = metadata->bitDepth;
	int32 width = metadata->width;
	int32 height = metadata->height;
	int32 inChannels = metadata->channelCount; // copy it into a local for later

	int32 bytes = (depth == 16 ? 2 : 1);
	uint32 stride = width * g_channelCount * bytes;
	uint32 imageLength, imageWidthBytes;
	uint8* filterBuffer;
	int32 all_ok = 1;

	int32 outputBytes = g_channelCount * bytes;
	int32 filterBytes = inChannels * bytes;

	uint8* out = png->data.getPtr();
	out = PlatformMemory::malloc<uint8>(width * height * outputBytes); // extra bytes to write off the end into
	if (!out)
	{
		LOG_ERROR("Out of memory.");
		return false;
	}

	imageWidthBytes = (((inChannels * width * depth) + 7) >> 3);
	imageLength = (imageWidthBytes + 1) * height;

	// we used to check for exact match between raw_len and img_len on non-interlaced PNGs,
	// but issue #276 reported a PNG in the wild that had extra data at the end (all zeros),
	// so just check for raw_len < img_len always.
	if (totalSize < imageLength)
	{
		LOG_ERROR("Not enough pixels; corrupt PNG.");
		return false;
	}

	// Allocate two scan lines worth of filter workspace buffer.
	filterBuffer = PlatformMemory::malloc<uint8>(imageWidthBytes * 2);
	if (!filterBuffer)
	{
		LOG_ERROR("Out of memory.");
		return false;
	}

	// Filtering for low-bit-depth images
	if (depth < 8)
	{
		filterBytes = 1;
		width = imageWidthBytes;
	}

	for (int32 y = 0; y < height; y++)
	{
		// cur/prior filter buffers alternate
		uint8* currentScanline = filterBuffer + (y & 1) * imageWidthBytes;
		uint8* previousScanline = filterBuffer + (~y & 1) * imageWidthBytes;
		uint8* outScanline = out + stride * y;
		int32 rowSizeBytes = width * filterBytes;
		EPngFilterType filter = (EPngFilterType)*raw++;

		// check filter type
		if (filter > EPngFilterType::Paeth)
		{
			LOG_ERROR("Invalid filter; corrupt PNG.");
			break;
		}

		unfilterPngScanline(y, filter, currentScanline, raw, rowSizeBytes, filterBytes, previousScanline);
		expandPngScanline(depth, color, currentScanline, outScanline, width, inChannels, g_channelCount);
		raw += rowSizeBytes;
	}

	PlatformMemory::free(filterBuffer);

	// Swap the RGBA bytes for BRGA (RGB => BRG)
	for (int32 index = 0; index < imageLength; index += 4)
	{
		uint8 r = out[index];
		uint8 g = out[index + 1];
		uint8 b = out[index + 2];
		uint8 a = out[index + 3];

		out[index] = b;     // blue
		out[index + 1] = r; // red
		out[index + 2] = g; // green
		out[index + 3] = a;
	}

	png->data.setPtr(out);

	return true;
}

inline void TextureImporter::expandPngScanline(int32 depth, int32 color, uint8* currentScanline, uint8* outScanline, int32 width, int32 inChannels, int32 outChannels)
{
	// expand decoded bits in cur to dest, also adding an extra alpha channel if desired
	if (depth < 8)
	{
		stbi_uc scale = (color == 0) ? stbi__depth_scale_table[depth] : 1; // scale grayscale values to 0..255 range
		stbi_uc* in = currentScanline;
		stbi_uc* out = outScanline;
		stbi_uc inb = 0;
		stbi__uint32 nsmp = width * inChannels;

		// expand bits to bytes first
		if (depth == 4)
		{
			for (int32 i = 0; i < nsmp; ++i)
			{
				if ((i & 1) == 0)
				{
					inb = *in++;
				}
				*out++ = scale * (inb >> 4);
				inb <<= 4;
			}
		}
		else if (depth == 2)
		{
			for (int32 i = 0; i < nsmp; ++i)
			{
				if ((i & 3) == 0)
				{
					inb = *in++;
				}
				*out++ = scale * (inb >> 6);
				inb <<= 2;
			}
		}
		else
		{
			for (int32 i = 0; i < nsmp; ++i)
			{
				if ((i & 7) == 0)
				{
					inb = *in++;
				}
				*out++ = scale * (inb >> 7);
				inb <<= 1;
			}
		}

		// insert alpha=255 values if desired
		if (inChannels != outChannels)
		{
			stbi__create_png_alpha_expand8(outScanline, outScanline, width, inChannels);
		}
	}
	else if (depth == 8)
	{
		if (inChannels == outChannels)
		{
			memcpy(outScanline, currentScanline, width * inChannels);
		}
		else
		{
			stbi__create_png_alpha_expand8(outScanline, currentScanline, width, inChannels);
		}
	}
	else if (depth == 16)
	{
		// convert the image data from big-endian to platform-native
		uint16* dest16 = (uint16*)outScanline;
		uint32 nsmp = width * inChannels;

		if (inChannels == outChannels)
		{
			for (int32 i = 0; i < nsmp; ++i, ++dest16, currentScanline += 2)
			{
				*dest16 = (currentScanline[0] << 8) | currentScanline[1];
			}
		}
		else
		{
			if (inChannels == 1)
			{
				for (int32 i = 0; i < width; ++i, dest16 += 2, currentScanline += 2)
				{
					dest16[0] = (currentScanline[0] << 8) | currentScanline[1];
					dest16[1] = 0xffff;
				}
			}
			else
			{
				STBI_ASSERT(inChannels == 3);
				for (int32 i = 0; i < width; ++i, dest16 += 4, currentScanline += 6)
				{
					dest16[0] = (currentScanline[0] << 8) | currentScanline[1];
					dest16[1] = (currentScanline[2] << 8) | currentScanline[3];
					dest16[2] = (currentScanline[4] << 8) | currentScanline[5];
					dest16[3] = 0xffff;
				}
			}
		}
	}
}

inline void TextureImporter::unfilterPngScanline(int32 y, EPngFilterType filter, uint8* currentScanline, uint8* raw, int32 rowSizeBytes, int32 filterBytes, uint8* previousScanline)
{
	// if first row, use special filter that doesn't sample previous row
	if (y == 0)
	{
		switch (filter)
		{
		case EPngFilterType::Up:
			filter = EPngFilterType::None;
			break;
		case EPngFilterType::Average:
			filter = EPngFilterType::First;
			break;
		case EPngFilterType::Paeth:
			filter = EPngFilterType::Sub;
			break;
		case EPngFilterType::None:
		case EPngFilterType::Sub:
		case EPngFilterType::First:
		default:
			break;
		}
	}

	// perform actual filtering
	switch (filter)
	{
	case EPngFilterType::None:
	{
		memcpy(currentScanline, raw, rowSizeBytes);
		break;
	}
	case EPngFilterType::Sub:
	{
		memcpy(currentScanline, raw, filterBytes);
		for (int32 x = filterBytes; x < rowSizeBytes; x++)
		{
			currentScanline[x] = TRUNCATE(raw[x] + currentScanline[x - filterBytes]);
		}
		break;
	}
	case EPngFilterType::Up:
	{
		for (int32 x = 0; x < rowSizeBytes; x++)
		{
			currentScanline[x] = TRUNCATE(raw[x] + previousScanline[x]);
		}
		break;
	}
	case EPngFilterType::Average:
	{
		for (int32 x = 0; x < filterBytes; x++)
		{
			currentScanline[x] = TRUNCATE(raw[x] + (previousScanline[x] >> 1));
		}
		for (int32 x = filterBytes; x < rowSizeBytes; x++)
		{
			currentScanline[x] = TRUNCATE(raw[x] + ((previousScanline[x] + currentScanline[x - filterBytes]) >> 1));
		}
		break;
	}
	case EPngFilterType::Paeth:
	{
		for (int32 x = 0; x < filterBytes; x++)
		{
			currentScanline[x] = TRUNCATE(raw[x] + previousScanline[x]);  // prior[x] == stbi__paeth(0,prior[x],0)
		}
		for (int32 x = filterBytes; x < rowSizeBytes; x++)
		{
			currentScanline[x] = TRUNCATE(raw[x] + stbi__paeth(currentScanline[x - filterBytes], previousScanline[x], previousScanline[x - filterBytes]));
		}
		break;
	}
	case EPngFilterType::First:
	{
		memcpy(currentScanline, raw, filterBytes);
		for (int32 x = filterBytes; x < rowSizeBytes; x++)
		{
			currentScanline[x] = TRUNCATE(raw[x] + (currentScanline[x - filterBytes] >> 1));
		}
		break;
	}
	}
}

// stbi__create_png_image
bool TextureImporter::createPngInterlaced(Buffer<uint8>* buffer, PngTexture* png)
{
	return false;
}

int32 TextureImporter::importPng(ByteReader* reader, Texture& texture)
{

	// Validate the header is the correct PNG header
	if (!isValidPngHeader(reader))
	{
		LOG_ERROR("Unable to load texture.");
		return g_invalidImageIndex;
	}

	// Read and parse the IHDR chunk. This is always the first one.
	PngTexture png;
	PngChunk ihdrChunk;
	if (!readPngChunk(reader, &ihdrChunk, &png.metadata))
	{
		LOG_ERROR("Error reading IHDR chunk.");
		return g_invalidImageIndex;
	}
	parsePngIHDR(&ihdrChunk, &png);

	// Read chunks until we hit the end of the file.
	bool atEnd = false;
	while (!atEnd)
	{
		PngChunk chunk;
		readPngChunk(reader, &chunk, &png.metadata);

		// If we reach the end chunk, exit
		switch (chunk.type)
		{
		case EPngChunkType::IHDR:
		{
			break;
		}
		case EPngChunkType::IDAT:
		{
			if (!parsePngIDAT(&chunk, &png))
			{
				return false;
			}
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

	uint8* data = png.data.getPtr();
	texture = Texture::fromData(data, png.metadata.width, png.metadata.height);

	g_textures.emplace_back(texture);
	return (int32)g_textures.size() - 1;
}

int32 TextureImporter::import(const std::string & fileName, Texture & texture)
{
	// Read the file into a buffer
	std::string data;
	if (!IO::readFile(fileName, data))
	{
		LOG_ERROR("Unable to read file {}", fileName);
		return g_invalidImageIndex;
	}

	// Create a reader from the string uncompressedData
	ByteReader reader(data, data.size(), std::endian::big);

	ETextureFileFormat format = getTextureFileFormat(fileName);
	switch (format)
	{
	case ETextureFileFormat::Png:
	{
		return importPng(&reader, texture);
	}
	case ETextureFileFormat::Bmp:
	{
		LOG_ERROR("Bmp not currently supported.");
		return -1;
	}
	case ETextureFileFormat::Jpg:
	{
		LOG_ERROR("Jpg not currently supported.");
		return -1;
	}
	default:
		LOG_ERROR("Invalid filetype for {}", fileName);
		return -1;
	}

}
