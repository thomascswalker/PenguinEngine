#pragma once

class IPlatform
{
public:
    virtual ~IPlatform() = default;
    virtual int Create() = 0;
    virtual int Show() = 0;
    virtual int Start() = 0;
    virtual int Loop(float DeltaTime) = 0;
    virtual int End() = 0;
    virtual bool IsInitialized() const = 0;
};
