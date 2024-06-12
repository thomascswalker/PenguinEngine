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
    virtual int32 Create() = 0;
    virtual int32 Show() = 0;
    virtual int32 Start() = 0;
    virtual int32 Loop() = 0;
    virtual int32 Paint() = 0;
    virtual int32 End() = 0;
    virtual int32 Swap() = 0;
    virtual bool IsInitialized() const = 0;

    // Properties
    virtual FRect GetSize() = 0;
    virtual EPlatformType GetPlatformType() = 0;

    // UI
    virtual bool GetFileDialog(std::string& OutFileName) = 0;
    
    virtual void ConstructMenuBar() = 0;
    virtual void SetMenuItemChecked(EMenuAction ActionId, bool bChecked) = 0;
};
