#pragma once

#include <memory>

#include "Viewport.h"
#include "Framework/Core/Core.h"
#include "Framework/Engine/Mesh.h"
#include "Math/MathCommon.h"

#define BYTES_PER_CHANNEL 8
#define BYTES_PER_PIXEL 32

struct PBuffer
{
    void* Memory; // Memory Order BB GG RR XX
    uint32 Width;
    uint32 Height;
    uint32 Pitch;

    PBuffer(uint32 InWidth, uint32 InHeight);
    void Resize(uint32 InWidth, uint32 InHeight);
    void Realloc();

    constexpr uint32 GetOffset(const uint32 X, const uint32 Y) const { return (Y * (Pitch / 4)) + X; }
    constexpr uint32 GetPixelCount() const { return Width * Height; }
    constexpr uint32 GetMemorySize() const { return Width * Height * BYTES_PER_PIXEL * 4; }
    void SetPixel(uint32 X, uint32 Y, const PColor& Color) const;
    void Clear() const
    {
        std::memset(Memory, 0, Width * Height * BYTES_PER_CHANNEL);
    }
    void Fill(const PColor& Color) const
    {
        // uint32* Ptr = Cast<uint32>(Memory);
        // while ()
        // {
        //     *Ptr = ((Color.R << 16) | Color.G << 8) | Color.B;
        // }
    }
};

class PRenderer
{
    std::shared_ptr<PBuffer> Buffer;
    std::shared_ptr<PViewport> Viewport;

public:
    PRenderer(uint32 InWidth, uint32 InHeight);
    void Resize(uint32 InWidth, uint32 InHeight) const;

    PBuffer* GetBuffer() const { return Buffer.get(); }
    uint32 GetWidth() const { return Buffer->Width; }
    uint32 GetHeight() const { return Buffer->Height; }

    PViewport* GetViewport() const { return Viewport.get(); }

    // Drawing
    void DrawLine(const FVector2& InA, const FVector2& InB, const PColor& Color) const;
    void DrawTriangle(const FVector3& V0, const FVector3& V1, const FVector3& V2) const;
    void DrawMesh(const PMesh* Mesh) const;
    void Render() const;
};
