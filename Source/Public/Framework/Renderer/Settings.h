#pragma once
#include "Framework/Core/Bitmask.h"

enum ERenderFlag : uint8
{
    None      = 0,
    Wireframe = 1 << 0,
    Shaded    = 1 << 2,
    Depth     = 1 << 3,
    Textures  = 1 << 4,
    Lights    = 1 << 5,
    Normals   = 1 << 6
};
DEFINE_BITMASK_OPERATORS(ERenderFlag);

namespace Renderer
{
    struct PRenderSettings
    {
    private:
        ERenderFlag RenderFlags = ERenderFlag::Wireframe;
        bool bMultithreaded = true;

    public:
        PRenderSettings()
        {
            RenderFlags = ERenderFlag::Shaded | ERenderFlag::Depth;
        }

        constexpr bool GetRenderFlag(const ERenderFlag Flag)
        {
            return (RenderFlags & Flag) == Flag;
        }

        void SetRenderFlag(const ERenderFlag Flag, const bool bState)
        {
            uint8 CurrentFlag = RenderFlags;
            bState ? CurrentFlag |= Flag : CurrentFlag &= ~Flag;
            RenderFlags = static_cast<ERenderFlag>(CurrentFlag);
        }

        bool ToggleRenderFlag(const ERenderFlag Flag)
        {
            const bool bState = GetRenderFlag(Flag); // Flip the state
            SetRenderFlag(Flag, !bState);
            return !bState;
        }

        bool IsMultithreaded()
        {
            return bMultithreaded;
        }

        void SetMultithreaded(const bool bNewState)
        {
            bMultithreaded = bNewState;
        }
        bool ToggleMultithreaded()
        {
	        bMultithreaded = !bMultithreaded;
        	return bMultithreaded;
        }
    };
}
