#pragma once

#include <map>
#include <bit>

#include "Math/MathFwd.h"
#include "Framework/Core/Bitmask.h"
#include "Framework/Core/IO.h"
#include "Framework/Core/Buffer.h"
#include "Framework/Renderer/Texture.h"

/** https://en.wikipedia.org/wiki/PNG */
inline uint8 g_magicPng[8] = { 137, 'P', 'N', 'G', 13, 10, 26, 10 };

constexpr int32 g_invalidImageIndex = -1;

enum class ETextureFileFormat
{
	Unknown,
	Png,
	Bmp,
	Jpg
};

enum class EPngColorType : uint8
{
	None = 0,
	Palette = 1,
	Color = 2,
	Alpha = 4
};

DEFINE_BITMASK_OPERATORS(EPngColorType)

enum class EPngCompressionMethod : uint8
{
	Default = 0, // ZLib
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

enum class EPngChunkType
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

inline std::map<std::string, EPngChunkType> g_pngChunkTypeMap =
{
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
	Buffer<uint8> uncompressedBuffer;
	Buffer<uint8> compressedBuffer;
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
	bool interlaced;
	uint8 channelCount; // Number of uncompressedData channels per pixel
};

struct PngTexture
{
	PngMetadata metadata;
	Buffer<uint8> data;
};

class TextureImporter
{
	static ETextureFileFormat getTextureFileFormat(const std::string& fileName);
	static bool isValidPngHeader(ByteReader* reader);

	static bool readPngChunk(ByteReader* reader, PngChunk* chunk, PngMetadata* metadata);
	static bool parsePngIHDR(PngChunk* chunk, PngTexture* png);
	static bool parsePngIDAT(PngChunk* chunk, PngTexture* png);
	
	static void expandPngScanline(int32 depth, int32 color, uint8* currentScanline, uint8* outScanline, int32 width, int32 inChannels, int32 outChannels);
	static void unfilterPngScanline(int32 y, EPngFilterType filter, uint8* currentScanline, uint8* raw, int32 rowSizeBytes, int32 filterBytes, uint8* previousScanline);

	static bool createPng(Buffer<uint8>* buffer, PngTexture* png);
	static bool createPngInterlaced(Buffer<uint8>* buffer, PngTexture* png);

	static int32 importPng(ByteReader* reader, Texture& texture);
public:
	static int32 import(const std::string & fileName, Texture & texture);
};
