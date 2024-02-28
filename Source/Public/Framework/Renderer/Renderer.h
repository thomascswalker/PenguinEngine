#pragma once

#include <memory>

#include "Math/Color.h"
#include "Math/Types.h"

#define BYTES_PER_CHANNEL 8
#define BYTES_PER_PIXEL 32

struct PBuffer
{
    void* Memory; // Memory Order BB GG RR XX
    uint32 Width;
    uint32 Height;
    uint32 Pitch;

    constexpr uint32 GetOffset(const uint32 X, const uint32 Y) const { return (Y * (Pitch / 4)) + X; }
    constexpr uint32 GetPixelCount() const { return Width * Height; }
    constexpr uint32 GetMemorySize() const { return Width * Height * BYTES_PER_PIXEL; }
    void SetPixel(uint32 X, uint32 Y, const PColor& Color) const;
};

class PRenderer
{
    std::shared_ptr<PBuffer> Buffer;

public:
    PRenderer(uint32 InWidth, uint32 InHeight);
    void SetSize(uint32 InWidth, uint32 InHeight) const;
    PBuffer* GetBuffer() const { return Buffer.get(); }
    uint32 GetWidth() const { return Buffer->Width; }
    uint32 GetHeight() const { return Buffer->Height; }
    uint32 GetMemorySize() const;
    void Realloc() const;

    // Drawing
    void Render() const;
};
