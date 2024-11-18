#pragma once

#include "Core/Classes.h"
#include "Math/MathFwd.h"


#ifdef WINDOWS_PLATFORM
typedef Win32Application Application;
#endif

/* @brief Enumerator defining the platform type. */
enum class EApplicationType : uint8
{
	Windows,
	MacOS,
	Linux,
	Unknown
};

inline EApplicationType getApplicationType()
{
#if defined(_WIN32) || defined(_WIN64)
	return EApplicationType::Windows;
#elif __APPLE__
	return EApplicationType::MacOS;
#elif __linux__
	return EApplicationType::Linux;
#endif
}