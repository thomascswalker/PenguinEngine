#pragma once

#include <string>
#include <filesystem>

#include "Core/IO.h"

namespace ResourceManager
{
	std::filesystem::path getRootPath()
	{
		std::filesystem::path root = __FILE__;
		root = root.parent_path(); // Engine
		root = root.parent_path(); // Source
		return root.parent_path(); // PenguinEngine
	}

	std::filesystem::path getResourceRootPath()
	{
		std::filesystem::path root = getRootPath();
		return (root / "Resources");
	}

	std::string getResourceFileName(const std::string& fileName)
	{
		auto resources = getResourceRootPath();
		auto fullFileName = resources / fileName;
		if (std::filesystem::exists(fullFileName))
		{
			return fullFileName.string();
		}
		LOG_ERROR("File {} not found.", fullFileName.string())
		return "";
	}
}