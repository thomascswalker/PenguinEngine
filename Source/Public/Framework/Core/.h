/*
	https://www.rfc-editor.org/rfc/rfc1950
	https://pyokagan.name/blog/2019-10-18-zlibinflate/
*/
#pragma once
#include <cassert>

#include "Framework/Core/Bitmask.h"
#include "Framework/Core/Buffer.h"

namespace ZLib
{
	inline uint32 g_zlibDeflate = 8;

	inline std::vector g_lengthExtraBits = {
		0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0
	};
	inline std::vector g_lengthBase = {
		3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227,
		258
	};
	inline std::vector g_distanceExtraBits = {
		0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13
	};
	inline std::vector g_distanceBase = {
		1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097,
		6145, 8193, 12289, 16385, 24577
	};
	static std::vector g_codeLengthCodesOrder = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

	enum EZLibCompressionType
	{
		None,
		Fixed,   // Huffman
		Dynamic, // Huffman
		Reserved
	};

	DEFINE_BITMASK_OPERATORS(EZLibCompressionType)

	struct Node
	{
		uint8 symbol;
		Node* left;
		Node* right;
	};

	struct HuffmanTree
	{
		Node* root;
	};

	inline void blListToTree(const std::vector<int>& bl, const std::vector<int>& alphabet, HuffmanTree* tree)
	{
		int maxBits = *std::ranges::max_element(bl);
	}

	inline void decodeTrees(ByteReader* reader, HuffmanTree* literalTree, HuffmanTree* distanceTree)
	{
		// The number of literal/length codes
		auto HLIT = reader->readBits(5) + 257;

		// The number of distance does
		auto HDIST = reader->readBits(5) + 1;

		// The number of code length codes
		auto HCLEN = reader->readBits(4) + 4;

		// Read code lengths for the code length alphabet
		std::vector codeLengthTreeBl(19, 0);
		for (int i = 0; i < HCLEN; i++)
		{
			codeLengthTreeBl[g_codeLengthCodesOrder[i]] = reader->readBits(3);
		}

		HuffmanTree codeLengthTree;
		std::vector alphabet({0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18});
		blListToTree(codeLengthTreeBl, alphabet, &codeLengthTree);
	}

	inline uint8 decodeSymbol(ByteReader* reader, const HuffmanTree* tree)
	{
		Node* node = tree->root;
		while (node->left || node->right)
		{
			uint8 b = reader->readBits(1);
			node = b ? node->right : node->left;
		}
		return node->symbol;
	}

	inline void inflateBlockData(ByteReader* reader, HuffmanTree* literalTree, HuffmanTree* distanceTree,
	                             std::vector<uint8>* out)
	{
		while (true)
		{
			uint8 sym = decodeSymbol(reader, literalTree);
			if (sym <= 254) // Literal byte
			{
				out->push_back(sym);
			}
			else if (sym == 255) // end of block
			{
				return;
			}
			sym -= 256;
			uint8 length = reader->readBits(g_lengthExtraBits[sym]) + g_lengthBase[sym];
			uint8 distSym = decodeSymbol(reader, distanceTree);
			uint8 dist = reader->readBits(g_lengthExtraBits[distSym]) + g_lengthBase[distSym];
			for (int i = 0; i < length; i++)
			{
				int j = out->size() - dist;
				out->push_back(out->at(j));
			}
		}
	}

	inline Buffer<uint8> inflateNoCompression(ByteReader* reader)
	{
		Buffer<uint8> header(4);
		header[0] = reader->readUInt8();
		header[1] = reader->readUInt8();
		header[2] = reader->readUInt8();
		header[3] = reader->readUInt8();
		auto len = header[1] * 256 + header[0];
		auto nlen = header[3] * 256 + header[2];
		auto tmpNlen = len ^ 65535;
		assert(nlen == tmpNlen);
		Buffer<uint8> outBuffer(len);
		for (int i = 0; i < len; i++)
		{
			outBuffer[i] = reader->readUInt8();
		}
		return outBuffer;
	}

	inline Buffer<uint8> inflateFixedHuffman(ByteReader* reader)
	{
		Buffer<uint8> outBuffer;
		return outBuffer;
	}

	inline Buffer<uint8> inflateDynamicHuffman(ByteReader* reader)
	{
		Buffer<uint8> outBuffer;
		HuffmanTree literalTree;
		HuffmanTree distanceTree;
		decodeTrees(reader, &literalTree, &distanceTree);
		std::vector<uint8> out;
		inflateBlockData(reader, &literalTree, &distanceTree, &out);
		return outBuffer;
	}

	inline Buffer<uint8> inflate(ByteReader* reader)
	{
		int32 finalBit = 0;
		EZLibCompressionType compressionType;
		while (!finalBit)
		{
			// True if this is the last block of data
			finalBit = reader->readBits(1);

			// Specifies how the data is compressed
			// 00 - No compression
			// 01 - Fixed Huffman
			// 10 - Dynamic Huffman
			// 11 - Reserved (error)
			compressionType = (EZLibCompressionType)reader->readBits(2);

			switch (compressionType)
			{
			case None:
				{
					return inflateNoCompression(reader);
				}
			case Fixed:
				{
					return inflateFixedHuffman(reader);
				}
			// TODO: Implement this
			case Dynamic:
				{
					return inflateDynamicHuffman(reader);
				}
			case Reserved:
				{
					assert(false);
				}
			}
		}
	}

	/**
	 * @brief Decompresses the data within the reader. Reads the header to determine validity.
	 * @param reader The reader with data to decompress.
	 * @return The decompressed data.
	 */
	inline Buffer<uint8> decompress(ByteReader* reader)
	{
		// Read the chunk header
		uint8 cmf = reader->readUInt8();
		uint8 cm = cmf & 15;
		uint8 flg = reader->readUInt8();

		if ((cmf * 256 + flg) % 31 != 0)
		{
			throw std::runtime_error("Bad ZLib header.");
		}
		if (flg & 32)
		{
			throw std::runtime_error("No preset dict.");
		}
		if (cm != g_zlibDeflate)
		{
			throw std::runtime_error("Bad compression.");
		}

		// Inflate the actual data of the buffer
		Buffer<uint8> outBuffer = inflate(reader);

		uint32 adler32 = reader->readUInt32();

		return outBuffer;
	}
}
