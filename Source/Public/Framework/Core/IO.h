#pragma once
#include <filesystem>
#include <fstream>

#include "String.h"

namespace IO
{
	static bool exists(const std::string& fileName)
	{
		return std::filesystem::exists(fileName);
	}

	static bool isType(const std::string& fileName, const std::string& type)
	{
		return fileName.ends_with(type);
	}

	static bool readFile(const std::string& fileName, std::string& buffer)
	{
		if (!exists(fileName))
		{
			LOG_ERROR("File %s not found.", fileName.c_str());
			return false;
		}
		std::ifstream stream(fileName, std::ios::in | std::ios::binary);
		if (stream.bad())
		{
			LOG_ERROR("Unable to read file %s.", fileName.c_str());
			return false;
		}
		const uint64 size = std::filesystem::file_size(fileName);
		buffer.resize(size, '\0');
		stream.read(buffer.data(), size);
		return true;
	}

	static bool readFile(const std::string& fileName, std::vector<std::string>& lines)
	{
		if (!exists(fileName))
		{
			LOG_ERROR("File %s not found.", fileName.c_str());
			return false;
		}
		std::ifstream stream(fileName, std::ios::in | std::ios::binary);
		if (stream.bad())
		{
			LOG_ERROR("Unable to read file %s.", fileName.c_str());
			return false;
		}
		const uint64 size = std::filesystem::file_size(fileName);
		std::string buffer;
		buffer.resize(size, '\0');
		stream.read(buffer.data(), size);

		Strings::split(buffer, lines, "\n");

		return true;
	}

	static std::istream& readLine(std::istream& stream, std::string& line)
	{
		// Clear the content of the Line
		line.clear();

		// Prepare Stream for input
		const std::istream::sentry sentry(stream, true);
		if (!sentry)
		{
			return stream;
		}

		// Create a new buffer from our Stream
		std::streambuf* streamBuffer = stream.rdbuf();


		// Switch on current char
		while (true)
		{
			// Read the current character
			const int c = streamBuffer->sbumpc();
			switch (c)
			{
			// For \n
			case '\n':
				{
					return stream;
				}
			// For \r\n
			case '\r':
				{
					// If the next character is a new Line, we'll bump to that next Line
					if (streamBuffer->sgetc() == '\n')
					{
						streamBuffer->sbumpc();
					}
					return stream;
				}
			// For \r
			case EOF: // -1
				{
					if (line.empty())
					{
						stream.setstate(std::ios::eofbit);
					}
					return stream;
				}
			// All other cases, the rest of the Line
			default:
				{
					line += static_cast<char>(c);
				}
			}
		}
	}
}
