#pragma once
#include "Framework/Core/Bitmask.h"

enum ERenderFlags : uint8
{
    None = 0,
    Wireframe = 1 << 0,
    Shaded = 1 << 2,
    Depth = 1 << 3,
    Textures = 1 << 4,
    Lights = 1 << 5
};
DEFINE_BITMASK_OPERATORS(ERenderFlags);

namespace Renderer
{
    struct PRenderSettings
    {
    private:
        ERenderFlags RenderFlags = ERenderFlags::Wireframe;
        bool bUseGlm = true;

    public:
        PRenderSettings()
        {
            RenderFlags = ERenderFlags::Shaded | ERenderFlags::Depth;
        }
        
        bool GetRenderFlag(const ERenderFlags Flag)
        {
            return (RenderFlags & Flag) == Flag;
        }

        void SetRenderFlag(const ERenderFlags Flag, const bool bState)
        {
            uint8 CurrentFlag = RenderFlags;
            bState ? CurrentFlag |= Flag : CurrentFlag &= ~Flag;
            RenderFlags = static_cast<ERenderFlags>(CurrentFlag);
        }

        void ToggleRenderFlag(const ERenderFlags Flag)
        {
            const bool bState = GetRenderFlag(Flag); // Flip the state
            SetRenderFlag(Flag, !bState);
        }

        bool GetUseGlm()
        {
            return bUseGlm;
        }

        void SetUseGlm(const bool bNewState)
        {
            bUseGlm = bNewState;
        }
    };
}
