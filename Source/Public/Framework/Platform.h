#pragma once

class IPlatform
{
public:
    virtual ~IPlatform() = default;
    virtual void Create() = 0;
    virtual int Show() = 0;
    virtual int Loop() = 0;
    virtual bool IsInitialized() const = 0;
};
