#pragma once

#include <Math/MathFwd.h>

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
    virtual uint32 Create() = 0;
    virtual uint32 Show() = 0;
    virtual uint32 Start() = 0;
    virtual uint32 Loop() = 0;
    virtual uint32 Paint() = 0;
    virtual uint32 End() = 0;
    virtual uint32 Swap() = 0;
    virtual bool IsInitialized() const = 0;

    // Properties
    virtual FRect GetSize() = 0;
    virtual EPlatformType GetPlatformType() = 0;
};
