#include "GenericApplication.h"
#include "Core/Macros.h"

#ifdef WINDOWS_PLATFORM
	#include "Platforms/Windows/Win32.h"
#endif

IApplication* IApplication::getApplication()
{
#ifdef WINDOWS_PLATFORM
	return g_windowsApplication;
#endif
	return nullptr;
}