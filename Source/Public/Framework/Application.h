#pragma once
#include <memory>

#include "Platform.h"

class PApplication
{
public:
    static PApplication* GetInstance();
    PApplication(const PApplication& Other) = delete;
    PApplication(PApplication&& Other) noexcept = delete;
    PApplication& operator=(const PApplication& Other) = delete;
    PApplication& operator=(PApplication&& Other) noexcept = delete;

    template <class PlatformType, typename ... Types>
    void Init(Types&&... Args)  // NOLINT
    {
        Platform = std::make_shared<PlatformType>(std::forward<Types>(Args)...);
    }
    
    int Run() const;
    IPlatform* GetPlatform() const { return Platform.get(); }

protected:
    PApplication() = default;

private:
    std::shared_ptr<IPlatform> Platform;
};
