#pragma once

enum EErrorCodes
{
	// Generic
	Success = 0,

	// Application
	ApplicationInitError = 10,
	ApplicationCreateError,
	ApplicationShowError,
	ApplicationStartError,
	ApplicationEndError,
	ApplicationLoopError,
};
