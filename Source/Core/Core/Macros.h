#pragma once

#define GENERATE_SUPER(n) using Super = n;

#if defined(_WIN32) || defined(_WIN64)
	#define WINDOWS_PLATFORM
#endif