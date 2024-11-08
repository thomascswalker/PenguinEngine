#pragma once

enum EErrorCodes
{
	// Generic
	Success = 0,

	// Platform
	PlatformInitError = 10,
	PlatformCreateError,
	PlatformShowError,
	PlatformStartError,
	PlatformEndError,
	PlatformLoopError,
};
