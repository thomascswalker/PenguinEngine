#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "Core/Buffer.h"
#include "Core/Types.h"
#include "Core/IO.h"

struct Font;
class FontDatabase;

inline std::unique_ptr<FontDatabase> g_fontDatabase = std::make_unique<FontDatabase>();

struct Font
{
	std::string family;
	std::string name;
};

class FontDatabase
{
	std::vector<Font> m_fonts;

	std::string getFontDirectory()
	{
#if defined(_WIN32) || defined(_WIN64)
		return "C:\\Windows\\Fonts";
#else
		return "";
#endif
	}

	void registerFont(RawBuffer<uint8> data, const std::string& fileName)
	{
		LOG_INFO("Registering {}", fileName)

		ByteReader buffer(data, std::endian::big);
		while (1)
		{
			auto c = buffer.readInt8();
			LOG_INFO("{}", c)
		}
	}

	void loadFonts()
	{
		std::filesystem::path fontDir = getFontDirectory();
		if (!std::filesystem::exists(fontDir))
		{
			LOG_ERROR("OS not implemented.");
			return;
		}
		for (const auto& entry : std::filesystem::directory_iterator(fontDir))
		{
			int8*		data = nullptr;
			std::string path = entry.path().string();

			// Only read .ttf files
			if (!path.ends_with(".ttf"))
			{
				continue;
			}
			RawBuffer<uint8> buffer;
			if (!IO::readFile(path, buffer))
			{
				LOG_ERROR("Failed to load font {}.", path)
				return;
			}
			registerFont(buffer, path);

			// TODO: Remove this `break`
			break;
		}
	}

public:
	void init()
	{
		loadFonts();
	}
};