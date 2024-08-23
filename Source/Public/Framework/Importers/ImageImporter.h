#pragma once

#include <map>
#include <bit>

#include "Math/MathFwd.h"
#include "Framework/Core/Bitmask.h"
#include "Framework/Core/IO.h"
#include "Framework/Core/Buffer.h"
#include "Framework/Renderer/Bitmap.h"

/** https://en.wikipedia.org/wiki/PNG */
inline uint8 g_magicPng[8] = {137, 'P', 'N', 'G', 13, 10, 26, 10};

constexpr int32 g_invalidImageIndex = -1;
inline std::vector<Bitmap> g_bitmaps;

enum class EPngColorType : uint8
{
	None    = 0,
	Palette = 1,
	Color   = 2,
	Alpha   = 4
};

DEFINE_BITMASK_OPERATORS(EPngColorType)

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
	Buffer<uint8> uncompressedBuffer;
	Buffer<uint8> compressedBuffer;
	//uint8* uncompressedData;
	//uint8* compressedData;
	//int32 uncompressedSize;
	//int32 compressedSize;
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

	uint8 channelCount; // Number of uncompressedData channels per pixel (1,2,3,4).
	uint8 pixelDepth;   // Number of bits per channel.
	uint8 spareByte;    // To align the uncompressedData.
};

struct PngBitmap
{
	PngMetadata metadata;
	Buffer<uint8> data;
};

/** https://www.nayuki.io/page/png-file-chunk-inspector */
class PngImporter
{
	static bool isValidHeader(ByteReader& reader);

	static bool parseIHDR(PngChunk* chunk, PngMetadata* metadata);

	/**
	 * @brief Based on https://pyokagan.name/blog/2019-10-18-zlibinflate/.
	 * @param chunk The IDAT chunk we are parsing.
	 * @param metadata The PNG file metadata.
	 * @return True if parsing is successful, false otherwise.
	 */
	static bool parseIDAT(PngChunk* chunk, const PngMetadata* metadata);

	static bool readChunk(ByteReader* reader, PngChunk* chunk, PngMetadata* metadata);

	static int32 importNative(const std::string& fileName, Bitmap& bitmap);
	static int32 importStb(const std::string& fileName, Bitmap& bitmap);

public:
	static int32 import(const std::string& fileName, Bitmap& bitmap);
};
