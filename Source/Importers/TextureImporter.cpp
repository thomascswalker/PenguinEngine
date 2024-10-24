#include <filesystem>

#include "Importers/TextureImporter.h"

#include "zlib.h"

#include "Application.h"
#include "Core/Compression.h"

constexpr int32 g_channelCount = 4; // Desired channel count for all textures
constexpr int32 g_channelGray  = 1; // Gray scale (single channel)
constexpr int32 g_channelRgb   = 3; // RGB (no alpha)
constexpr int32 g_channelRgba  = 4; // RGBA (includes alpha)

auto app = Application::getInstance();

inline uint8 truncate(int32 value)
{
	return (uint8)(value & UINT8_MAX);
}

inline ETextureFileType TextureImporter::getTextureFileType(const std::string& fileName)
{
	if (fileName.ends_with(".png"))
	{
		return ETextureFileType::Png;
	}
	if (fileName.ends_with(".bmp"))
	{
		return ETextureFileType::Bmp;
	}
	if (fileName.ends_with(".jpg") || fileName.ends_with(".jpeg"))
	{
		return ETextureFileType::Jpg;
	}
	LOG_ERROR("Invalid texture type {}.", fileName)
	return ETextureFileType::Unknown;
}

bool TextureImporter::isValidPngHeader(ByteReader* reader)
{
	int32 isValid = 0;
	for (uint8 value : g_magicPng)
	{
		unsigned char c = reader->readUInt8();
		isValid |= c == value;
	}
	return (bool)isValid;
}

int32 TextureImporter::parsePngIHDR(PngChunk* chunk, PngTexture* png)
{
	ByteReader reader(chunk->compressedBuffer);
	PngMetadata* metadata = &png->metadata;

	// Read the first 13 bytes of the chunk. This always totals to 13.
	// We swap the byte order for width and height as it's always stored in Big endian, but your computer
	// may be Little endian. `swapByteOrder` performs the swap if the expected order is not your default.
	metadata->width             = swapByteOrder(reader.readInt32());        // 4
	metadata->height            = swapByteOrder(reader.readInt32());        // 8
	metadata->bitDepth          = reader.readInt8();                        // 9
	metadata->colorType         = (EPngColorType)reader.readInt8();         // 10
	metadata->compressionMethod = (EPngCompressionMethod)reader.readInt8(); // 11
	metadata->filterMethod      = reader.readInt8();                        // 12
	metadata->interlaced        = (bool)reader.readInt8();                  // 13

	if (reader.getPos() != 13)
	{
		LOG_ERROR("Error reading IHDR chunk.")
		return TextureImporterError::PngChunkError;
	}

	// Compute the channel count from the color type
	auto color               = (int8)metadata->colorType;
	metadata->inChannelCount = (color & 2 ? 3 : 1) + (color & 4 ? 1 : 0);

	return TextureImporterError::Ok;
}

int32 TextureImporter::parsePngIDAT(PngChunk* chunk, PngTexture* png)
{
	PngMetadata* metadata = &png->metadata;

	// Compute the uncompressed size of the image.
	// (PixelCount in each row + an extra pixel for the filter type)
	uint32 inCompressedSize = metadata->width * metadata->height * metadata->outChannelCount + metadata->height;
	if (inCompressedSize >= UINT32_MAX)
	{
		LOG_ERROR("Cannot allocate {} bytes. Memory limit reached.", inCompressedSize);
		return TextureImporterError::MemoryError;
	}
	// Resize the uncompressed buffer to the uncompressed size
	chunk->uncompressedBuffer.resize((size_t)inCompressedSize);

	// Resize the final PNG data to the output uncompressed size, accounting for if, for example, input channel
	// count is 3 and output is 4.
	uint32 outUncompressedSize = (metadata->width * metadata->height * metadata->outChannelCount) + metadata->height;
	if (outUncompressedSize >= UINT32_MAX)
	{
		LOG_ERROR("Cannot allocate {} bytes. Memory limit reached.", outUncompressedSize);
		return TextureImporterError::MemoryError;
	}

	size_t offset  = png->data.size();
	size_t newSize = offset + outUncompressedSize;
	png->data.extend(newSize);

	// Based on the PNG spec, the only method available is the Default method
	// (ZLib).
	switch (metadata->compressionMethod)
	{
	case EPngCompressionMethod::MidGray:
		{
			// Uncompress the whole compressedBuffer with ZLib's inflate
			int32 result = Compression::uncompressZlib(&chunk->uncompressedBuffer, &chunk->compressedBuffer);
			if (result != Z_OK)
			{
				return TextureImporterError::DecompressionError;
			}
			// If interlaced, create the final PNG image data by deinterlacing, or
			// if not, create the final PNG image data from the raw uncompressed
			// data. This involves unfiltering each row.]
			result = metadata->interlaced
				         ? pngUnfilterInterlaced(&chunk->uncompressedBuffer, png)
				         : pngUnfilter(&chunk->uncompressedBuffer, png);
			return result;
		}
	case EPngCompressionMethod::Invalid:
	default:
		{
			return TextureImporterError::DecompressionError;
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

	// Allocate the memory for this chunk's uncompressedData, given the
	// uncompressedData compressedSize

	// Loop through and read all bytes
	for (uint32 i = 0; i < chunk->compressedBuffer.size(); i++)
	{
		chunk->compressedBuffer[i] = reader->readUInt8();
	}

	// TODO: Actually calculate the CRC and validate it
	auto crc = reader->readUInt32();

	// If a name argument was passed in, verify the name we read above is the
	// same.
	return true;
}

int32 TextureImporter::pngUnfilter(RawBuffer<uint8>* buffer, PngTexture* png, size_t offset)
{
	// Raw pointer to the final PNG data which is pushed to the Texture object
	// This is offset by the size of the previous IDAT chunk, in case there's multiple chunks.
	uint8* out     = png->data.getPtr() + offset;
	size_t outSize = png->data.size();

	// Raw pointer to the uncompressed image data. This pointer is what is incremented and accessed
	// to retrieve the current working byte.
	uint8* in = buffer->getPtr();

	// Metadata local vars
	PngMetadata* metadata = &png->metadata;

	auto colorType        = (int32)metadata->colorType;
	int32 totalSize       = (int32)buffer->size();
	int32 depth           = metadata->bitDepth;
	int32 width           = (int32)metadata->width;
	int32 height          = (int32)metadata->height;
	int32 inChannelCount  = metadata->inChannelCount;
	int32 outChannelCount = metadata->outChannelCount;

	int32 bpc     = (depth == DEPTH_16 ? 2 : 1); // Bytes-per-component (R, G, B, A)
	uint32 stride = width * outChannelCount * bpc;
	// Total number of bytes in a single row of pixels in the output buffer

	// Compute the input and output bytes-per-pixel. These may be different depending on the number
	// of requested channels for the Texture to load vs. the actual number of channels in the PNG image.
	int32 outBpp = outChannelCount * bpc; // Bytes-per-pixel for the output image
	int32 inBpp  = inChannelCount * bpc;  // Bytes-per-pixel from the input image

	// Allocate memory to the output PNG given the bytes-per-pixel of the desired channel count.
	out = PlatformMemory::malloc<uint8>(width * height * outBpp); // extra bytes to write off the end into
	if (!out)
	{
		LOG_ERROR("Out of memory.");
		return TextureImporterError::MemoryError;
	}

	uint32 inRowByteCount = (((inChannelCount * width * depth) + 7) >> 3); // Number of bytes in a single row of pixels
	uint32 inTotalByteCount = (inRowByteCount + 1) * height; // Total number of bytes in the entire image.

	uint32 outRowByteCount = (((outChannelCount * width * depth) + 7) >> 3);
	// Number of bytes in a single row of pixels
	uint32 outTotalByteCount = (outRowByteCount + 1) * height; // Total number of bytes in the entire image.

	// we used to check for exact match between raw_len and img_len on
	// non-interlaced PNGs, but issue #276 reported a PNG in the wild that had
	// extra data at the end (all zeros), so just check for raw_len < img_len
	// always.
	if (totalSize < (int32)inTotalByteCount)
	{
		LOG_ERROR("Not enough pixels; corrupt PNG.");
		return TextureImporterError::DataError;
	}

	// Allocate two scan m_lines worth of memory to the workspace buffer. This buffer will contain the temporary
	// unfiltered bytes. This is eventually copied into the final PNG buffer.
	auto filterBuffer = PlatformMemory::malloc<uint8>(outRowByteCount * 2);
	if (!filterBuffer)
	{
		LOG_ERROR("Out of memory.");
		return TextureImporterError::MemoryError;
	}

	// Filtering for low-bit-depth images
	if (depth < DEPTH_8)
	{
		inBpp = 1;
		width = inRowByteCount;
	}

	// Iterate through each row of the image
	for (int32 y = 0; y < height; y++)
	{
		// Pointer to the beginning of the current scanline we're working on.
		uint8* currentScanline = filterBuffer + (y & 1) * inRowByteCount;
		// Pointer to the beginning of the previous scanline we had worked on.
		uint8* previousScanline = filterBuffer + (~y & 1) * inRowByteCount;
		// Pointer to the current scanline in the final PNG image.
		uint8* outScanline = out + stride * y;

		// The first byte of a row is always the filter type.
		auto filter = (EPngFilterType)*in++;

		// Validate the filter type.
		if (filter > EPngFilterType::Paeth)
		{
			LOG_ERROR("Invalid filter; corrupt PNG.");
			return TextureImporterError::PngFilterError;
		}

		// Unfilter the current scanline
		pngUnfilterScanline(y, filter, currentScanline, in, inRowByteCount, inBpp, previousScanline);

		// If the image's channel count is the same as our desired channel count, copy the current
		// scanline into the out scanline
		if (inChannelCount == outChannelCount)
		{
			memcpy(outScanline, currentScanline, width * inChannelCount);
		}
		// Otherwise add an alpha channel filled with 255 to the image
		else
		{
			int32 result = addAlphaChannel(currentScanline, outScanline, width, inChannelCount);
			if (result != TextureImporterError::Ok)
			{
				return result;
			}
		}

		// Offset our in buffer by the rest of the row size so we start at a new row. The current byte
		// after this is the next row's filter type.
		in += inRowByteCount;
	}

	// Free the temporary workspace buffer we were using
	PlatformMemory::free(filterBuffer);

	// Set the final PNG data pointer to our `out` pointer where we've stored all the data. This probably shouldn't
	// be necessary as the `out` pointer is derived from `png->data.getPtr()` in the first place, but this seems
	// to be required.
	png->data.setPtr(out);

	return TextureImporterError::Ok;
}

int32 TextureImporter::addAlphaChannel(uint8* in, uint8* out, uint32 width, int32 channelCount)
{
	if (channelCount == 4)
	{
		return TextureImporterError::Ok;
	}

	if (channelCount != 3)
	{
		LOG_ERROR("Currently can only add alpha channel to RGB. Does not work with grayscale.")
		return TextureImporterError::ChannelError;
	}

	for (int x = width - 1; x >= 0; x--)
	{
		int32 inOffset  = x * 3;
		int32 outOffset = x * 4;

		auto b             = in[inOffset + 2];
		auto g             = in[inOffset + 1];
		auto r             = in[inOffset + 0];
		out[outOffset + 3] = 255;
		out[outOffset + 2] = b;
		out[outOffset + 1] = g;
		out[outOffset + 0] = r;
	}

	return TextureImporterError::Ok;
}

inline int32 TextureImporter::pngPaeth(int32 a, int32 b, int32 c)
{
	int32 threshold = c * 3 - (a + b);
	int32 low       = a < b ? a : b;
	int32 high      = a < b ? b : a;
	int32 t0        = (high <= threshold) ? low : c;
	int32 t1        = (threshold <= low) ? high : t0;
	return t1;
}

int32 TextureImporter::pngUnfilterScanline(int32 y, EPngFilterType filter, uint8* currentScanline, uint8* raw,
                                           int32 rowSizeBytes, int32 filterBytes, uint8* previousScanline)
{
	// if first row, use special filter that doesn't sample previous row
	if (y == 0)
	{
		switch (filter)
		{
		// Up => None
		case EPngFilterType::Up:
			{
				filter = EPngFilterType::None;
				break;
			}
		// Average => First
		case EPngFilterType::Average:
			{
				filter = EPngFilterType::First;
				break;
			}
		// Paeth => Sub
		case EPngFilterType::Paeth:
			{
				filter = EPngFilterType::Sub;
				break;
			}
		case EPngFilterType::None:
		case EPngFilterType::Sub:
		case EPngFilterType::First:
		default:
			{
				break;
			}
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
				currentScanline[x] = truncate(raw[x] + currentScanline[x - filterBytes]);
			}
			break;
		}
	case EPngFilterType::Up:
		{
			for (int32 x = 0; x < rowSizeBytes; x++)
			{
				currentScanline[x] = truncate(raw[x] + previousScanline[x]);
			}
			break;
		}
	case EPngFilterType::Average:
		{
			for (int32 x = 0; x < filterBytes; x++)
			{
				currentScanline[x] = truncate(raw[x] + (previousScanline[x] >> 1));
			}
			for (int32 x = filterBytes; x < rowSizeBytes; x++)
			{
				currentScanline[x] = truncate(raw[x] + ((previousScanline[x] + currentScanline[x - filterBytes]) >> 1));
			}
			break;
		}
	case EPngFilterType::Paeth:
		{
			for (int32 x = 0; x < filterBytes; x++)
			{
				currentScanline[x] = truncate(raw[x] + previousScanline[x]);
			}
			for (int32 x = filterBytes; x < rowSizeBytes; x++)
			{
				currentScanline[x] = truncate(raw[x] + pngPaeth(currentScanline[x - filterBytes], previousScanline[x],
				                                                previousScanline[x - filterBytes]));
			}
			break;
		}
	case EPngFilterType::First:
		{
			memcpy(currentScanline, raw, filterBytes);
			for (int32 x = filterBytes; x < rowSizeBytes; x++)
			{
				currentScanline[x] = truncate(raw[x] + (currentScanline[x - filterBytes] >> 1));
			}
			break;
		}
	}

	return 0;
}

int32 TextureImporter::pngStripFilterByte(uint8* in, uint8* out, int32 inSize)
{
	return int32();
}

// stbi__create_png_image
int32 TextureImporter::pngUnfilterInterlaced(RawBuffer<uint8>* buffer, PngTexture* png, size_t offset)
{
	return 0;
}

int32 TextureImporter::importPng(ByteReader* reader, Texture* texture, ETextureFileFormat format)
{
	// Validate the header is the correct PNG header
	if (!isValidPngHeader(reader))
	{
		LOG_ERROR("Unable to load texture.");
		return TextureImporterError::HeaderError;
	}

	// Read and parse the IHDR chunk. This is always the first one.
	PngTexture png;
	png.metadata.outChannelCount = (uint8)format;
	PngChunk ihdrChunk;
	if (!readPngChunk(reader, &ihdrChunk, &png.metadata))
	{
		LOG_ERROR("Error reading IHDR chunk.");
		return TextureImporterError::PngChunkError;
	}
	parsePngIHDR(&ihdrChunk, &png);

	// Read chunks until we hit the end of the file.
	bool atEnd   = false;
	int32 result = 0;
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
				result = parsePngIDAT(&chunk, &png);
				if (result != TextureImporterError::Ok)
				{
					png.data.clear();
					return result;
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

	RawBuffer<uint8> data = png.data;
	// Bytes per row
	int32 bpr          = (png.metadata.width * g_bytesPerPixel) + 1;
	int32 tmpSize      = bpr * png.metadata.height;
	RawBuffer<uint8> tmp(tmpSize);

	for (int32 row = 0; row < (int32)png.metadata.height; row++)
	{
		size_t offset = row * bpr;
		memcpy(tmp.getPtr() + offset, data.getPtr() + offset, bpr);
	}

	texture->resize({(int32)png.metadata.width, (int32)png.metadata.height});
	texture->setData(&data);
#ifndef PENG_HARDWARE_ACCELERATION
	// Swap byte order for scanline
	texture->setByteOrder(ETextureByteOrder::BRGA);
#endif
	texture->setChannelCount((uint8)format);

	return result;
}

int32 TextureImporter::import(const std::string& fileName, Texture* texture, const ETextureFileFormat format)
{
	if (!std::filesystem::exists(fileName))
	{
		LOG_ERROR("File {} not found.", fileName);
		return TextureImporterError::FileNotFoundError;
	}

	// Read the file into a buffer
	std::string fileData;
	if (!IO::readFile(fileName, fileData))
	{
		LOG_ERROR("Unable to read file {}", fileName);
		return TextureImporterError::FileReadError;
	}

	// Create a reader from the string uncompressedData
	ByteReader reader(fileData, fileData.size(), std::endian::big);

	int32 result;

	switch (getTextureFileType(fileName))
	{
	case ETextureFileType::Png:
		{
			result = importPng(&reader, texture, format);
			break;
		}
	case ETextureFileType::Bmp:
		{
			LOG_ERROR("Bmp not currently supported.");
			result = TextureImporterError::NotImplementedError;
			break;
		}
	case ETextureFileType::Jpg:
		{
			LOG_ERROR("Jpg not currently supported.");
			result = TextureImporterError::NotImplementedError;
			break;
		}
	default:
		{
			LOG_ERROR("Invalid filetype for {}", fileName);
			result = TextureImporterError::FileTypeError;
			break;
		}
	}

	return result;
}
