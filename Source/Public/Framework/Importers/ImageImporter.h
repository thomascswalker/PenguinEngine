#pragma once
#include "Math/MathFwd.h"
#include "Framework/Core/IO.h"
#include "Framework/Renderer/Bitmap.h"

/** https://en.wikipedia.org/wiki/PNG */
inline uint8 g_magicPng[8] = {137, 'P', 'N', 'G', 13, 10, 26, 10};

class PngImporter
{
	static bool isValidHeader(const std::string& string)
	{
		int32 isValid = 1;
		for (int32 index = 0; index < 8; index++)
		{
			isValid |= (uint8)string[index] == g_magicPng[index];
		}
		return (bool)isValid;
	}

public:
	static bool import(const std::string& fileName, Bitmap* bitmap)
	{
		// Read the file into a buffer
		std::string buffer;
		if (!IO::readFile(fileName, buffer))
		{
			LOG_ERROR("Unable to read file {}", fileName)
			return false;
		}

		// Create a string stream from the buffer
		std::stringstream stream;
		stream << buffer.data();

		// Validate the header is the correct PNG header
		if (!isValidHeader(buffer))
		{
			return false;
		}

		stream.seekg(8);

		return true;
	}
};
