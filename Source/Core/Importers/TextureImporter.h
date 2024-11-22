#pragma once

#include <map>
#include <bit>

#include "Math/MathFwd.h"
#include "Core/Bitmask.h"
#include "Core/IO.h"
#include "Core/Buffer.h"
#include "Renderer/Texture.h"

/** https://en.wikipedia.org/wiki/PNG */
constexpr uint8 g_magicPng[8]       = {137, 'P', 'N', 'G', 13, 10, 26, 10};
constexpr int32 g_invalidImageIndex = -1;

namespace TextureImporterError
{
	inline int32 Ok                  = 0;
	inline int32 NotImplementedError = 1;
	inline int32 FileTypeError       = 2;
	inline int32 FileReadError       = 3;
	inline int32 FileNotFoundError   = 4;

	inline int32 HeaderError        = 5;
	inline int32 DataError          = 6;
	inline int32 MemoryError        = 7;
	inline int32 BufferError        = 8;
	inline int32 CompressionError   = 9;
	inline int32 DecompressionError = 10;
	inline int32 ChannelError       = 11;

	inline int32 PngChunkError  = 20;
	inline int32 PngFilterError = 21;
} // namespace TextureImporterError

enum class ETextureFileType : uint8
{
	Unknown,
	Png,
	Bmp,
	Jpg
};

enum class ETextureFileFormat : uint8
{
	Grayscale = 1,
	Rgb       = 3,
	Rgba      = 4
};

enum class EPngColorType : uint8
{
	None    = 0,
	Palette = 1,
	Color   = 2,
	Alpha   = 4
};

DEFINE_BITMASK_OPERATORS(EPngColorType)

enum class EPngCompressionMethod : uint8
{
	MidGray = 0, // ZLib
	Invalid = 1
};

// http://www.libpng.org/pub/png/book/chapter09.html#png.ch09.div.1
enum class EPngFilterType : uint8
{
	// Each byte is unchanged.
	None = 0,
	// Each byte is replaced with the difference between it and the "corresponding byte" to its left.
	Sub = 1,
	// Each byte is replaced with the difference between it and the byte above it(in the previous row, as it was before filtering).
	Up = 2,
	// Each byte is replaced with the difference between it and the average of the corresponding bytes to its left and above it, truncating any fractional part.
	Average = 3,
	// Each byte is replaced with the difference between it and the Paeth predictor of the corresponding bytes to its left, above it, and to its upper left.
	Paeth = 4,
	// Filter specifically for the first row
	First = 5
};

enum class EPngChunkType : uint8
{
	// Required

	// Image metadata
	IHDR,

	// Image pixel data
	IDAT,

	// Last chunk in an image
	IEND,

	// Optional
	PLTE,
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

inline std::map<std::string, EPngChunkType> g_pngChunkTypeMap = {
	{"IHDR", EPngChunkType::IHDR},
	{"IDAT", EPngChunkType::IDAT},
	{"IEND", EPngChunkType::IEND},
	{"PLTE", EPngChunkType::PLTE},
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
	RawBuffer<uint8> uncompressedBuffer;
	RawBuffer<uint8> compressedBuffer;
	EPngChunkType	 type{};
};

struct PngMetadata
{
	uint32 width;   // Width of the image in pixels.
	uint32 height;  // Height of the image in pixels.
	uint8 bitDepth; // 1, 2, 4, 8, or 16 bits/channel
	EPngColorType colorType;
	EPngCompressionMethod compressionMethod;
	uint8 filterMethod;
	bool interlaced;
	uint8 inChannelCount;  // Number of uncompressedData channels per pixel
	uint8 outChannelCount; // The desired channel count
};

struct PngTexture
{
	PngMetadata metadata{};
	RawBuffer<uint8> data{};
};

class TextureImporter
{
	/** PNG **/

	static bool isValidPngHeader(ByteReader* reader);

	static bool readPngChunk(ByteReader* reader, PngChunk* chunk, PngMetadata* metadata);
	static int32 parsePngIHDR(PngChunk* chunk, PngTexture* png);
	static int32 parsePngIDAT(PngChunk* chunk, PngTexture* png);

	static int32 pngPaeth(int32 a, int32 b, int32 c);
	static int32 pngUnfilterScanline(int32 y, EPngFilterType filter, uint8* currentScanline, uint8* raw,
	                                 int32 rowSizeBytes, int32 filterBytes, uint8* previousScanline);
	static int32 pngStripFilterByte(uint8* in, uint8* out, int32 inSize);

	static int32 pngUnfilter(RawBuffer<uint8>* buffer, PngTexture* png, size_t offset = 0);
	static int32 pngUnfilterInterlaced(RawBuffer<uint8>* buffer, PngTexture* png, size_t offset = 0);

	static int32 importPng(ByteReader* reader, Texture* texture, ETextureFileFormat format);

	/** Misc **/

	static ETextureFileType getTextureFileType(const std::string& fileName);
	static int32 addAlphaChannel(uint8* in, uint8* out, uint32 width, int32 channelCount);

public:
	static int32 import(const std::string& fileName, Texture* texture,
	                    ETextureFileFormat format = ETextureFileFormat::Rgba);
};
