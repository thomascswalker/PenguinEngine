#include "Framework/Renderer/Renderer.h"

#include "Framework/Core/Logging.h"
#include "Math/Types.h"

#if _WIN32
    #include <windows.h>
#endif

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
    return static_cast<uint32>(sizeof(uint32) * 4 * Buffer->Height * Buffer->Width);
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
    uint8* Row = static_cast<uint8*>(Buffer->Memory);
    for (uint32 Y = 0; Y < Buffer->Height; Y++)
    {
        uint32* Pixel = reinterpret_cast<uint32*>(Row);
        for (uint32 X = 0; X < Buffer->Width; X++)
        {
            uint8 Red = static_cast<uint8>(X);
            uint8 Green = static_cast<uint8>(Y);
            uint8 Blue = 0;

            *(Pixel++) = ((Red << 16) | Green << 8) | Blue;
        }
        Row += Buffer->Pitch;
    }
}
