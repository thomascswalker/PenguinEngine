#pragma once

#include "Math/MathFwd.h"

class IApplication;
class Win32Application;
class AppleApplication;
class LinuxApplication;

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