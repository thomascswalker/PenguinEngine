#include "Framework/Renderer/Renderer.h"

#include "Framework/Core/Logging.h"
#include "Math/Types.h"

#if _WIN32
    #include <windows.h>
#endif

/* Buffer */

void PBuffer::SetPixel(const uint32 X, const uint32 Y, const PColor& Color) const
{
    uint32* Ptr = static_cast<uint32*>(Memory) + GetOffset(X, Y);

    // Placing pixels in memory order within a uint32:
    // BB GG RR AA
    // 00 00 00 00
    *Ptr = ((Color.R << 16) | Color.G << 8) | Color.B; // TODO: Disregard alpha channel for now
}

/* Renderer */

PRenderer::PRenderer(uint32 InWidth, uint32 InHeight)
{
    Buffer = std::make_shared<PBuffer>();
    SetSize(InWidth, InHeight);
    Realloc();
    
    LOG_INFO("Constructed renderer.")
}

void PRenderer::SetSize(uint32 InWidth, uint32 InHeight) const
{
    Buffer->Width = InWidth;
    Buffer->Height = InHeight;
    Buffer->Pitch = static_cast<uint32>(sizeof(uint32) * InWidth);
}

uint32 PRenderer::GetMemorySize() const
{
    return BYTES_PER_PIXEL * 4 * Buffer->Height * Buffer->Width;
}

void PRenderer::Realloc() const
{
#if _WIN32
    if (Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }
    Buffer->Memory = VirtualAlloc(nullptr, GetMemorySize(), MEM_COMMIT, PAGE_READWRITE);
#endif
}

void PRenderer::Render() const
{
    for (uint32 Y = 0; Y < Buffer->Height; Y++)
    {
        for (uint32 X = 0; X < Buffer->Width; X++)
        {
            PColor Color = PColor::FromRgba(X, Y, 0);
            Buffer->SetPixel(X, Y, Color);
        }
    }
}
