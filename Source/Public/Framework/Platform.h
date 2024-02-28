#pragma once
#include "Math/Rect.h"

class IPlatform
{
public:
    virtual ~IPlatform() = default;

    // Core
    virtual int Create() = 0;
    virtual int Show() = 0;
    virtual int Start() = 0;
    virtual int Loop(float DeltaTime) = 0;
    virtual int End() = 0;
    virtual bool IsInitialized() const = 0;

    // Properties
    virtual RectI GetSize() = 0;
};
