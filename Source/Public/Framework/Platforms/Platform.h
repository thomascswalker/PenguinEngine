#pragma once

#include "Math/MathFwd.h"

/* @brief Enumerator defining the platform type. */
enum class EPlatformType : uint8
{
	Windows,
	MacOS,
	Linux,
	Unknown
};

inline EPlatformType getPlatformType()
{
#if defined(_WIN32) || defined(_WIN64)
	return EPlatformType::Windows;
#elif __APPLE__
	return EPlatformType::MacOS;
#elif __linux__
	return EPlatformType::Linux;
#endif
}