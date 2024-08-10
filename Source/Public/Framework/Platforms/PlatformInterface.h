#pragma once

#include <string>
#include <Math/MathFwd.h>

#include "Framework/Input/InputHandler.h"

enum class EPlatformType : uint8
{
	Windows,
	MacOS,
	Linux,
	Unknown
};

class IPlatform
{
public:
	virtual ~IPlatform() = default;

	// Core
	virtual int32 create() = 0;
	virtual int32 show() = 0;
	virtual int32 start() = 0;
	virtual int32 loop() = 0;
	virtual int32 paint() = 0;
	virtual int32 end() = 0;
	virtual int32 swap() = 0;
	virtual bool isInitialized() const = 0;

	// Properties
	virtual FRect getSize() = 0;
	virtual EPlatformType getPlatformType() = 0;

	// UI
	virtual bool getFileDialog(std::string& outFileName) = 0;

	virtual void constructMenuBar() = 0;
	virtual void setMenuItemChecked(EMenuAction actionId, bool bChecked) = 0;
};
