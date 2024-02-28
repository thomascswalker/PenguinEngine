#pragma once

#include <memory>
#include "Math/Types.h"

static int32 BYTES_PER_PIXEL = 32;

struct PBuffer
{
    void* Memory; // Memory Order BB GG RR XX
    uint32 Width;
    uint32 Height;
    uint32 Pitch;
};

class PRenderer
{
    std::shared_ptr<PBuffer> Buffer;

public:
    PRenderer(uint32 InWidth, uint32 InHeight);
    void SetSize(uint32 InWidth, uint32 InHeight) const;
    void Render() const;
    PBuffer* GetBuffer() const { return Buffer.get(); }
    uint32 GetWidth() const { return Buffer->Width; }
    uint32 GetHeight() const { return Buffer->Height; }
    uint32 GetMemorySize() const;
    void Realloc() const;
};
