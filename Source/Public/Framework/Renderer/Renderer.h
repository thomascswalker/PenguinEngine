#pragma once

#include <memory>
#include <map>

#include "Grid.h"
#include "Viewport.h"
#include "Framework/Engine/Mesh.h"
#include "Math/MathCommon.h"
#include "Framework/Platforms/PlatformMemory.h"

enum
{
    BYTES_PER_CHANNEL = 8,
    BYTES_PER_PIXEL = 32
};

struct PChannel
{
    void* Memory; // Memory Order BB GG RR XX
    uint32 ChannelCount = BYTES_PER_PIXEL / BYTES_PER_CHANNEL; // 4
    uint32 Width;
    uint32 Height;
    uint32 Pitch;
    EBufferType Type;

    PChannel(EBufferType InType, uint32 InWidth, uint32 InHeight) : Type(InType)
    {
        Resize(InWidth, InHeight);
    }
    void Resize(uint32 InWidth, uint32 InHeight)
    {
        Width = InWidth;
        Height = InHeight;
        Pitch = ChannelCount * InWidth;
        Realloc();
    }
    void Realloc()
    {
        Memory = PPlatformMemory::Realloc(Memory, GetMemorySize());
    }

    constexpr uint32 GetOffset(const uint32 X, const uint32 Y) const { return (Y * (Pitch / ChannelCount)) + X; }
    constexpr uint32 GetPixelCount() const { return Width * Height; }
    constexpr uint32 GetMemorySize() const { return Width * Height * BYTES_PER_PIXEL * ChannelCount; }
    
    void SetPixel(uint32 X, uint32 Y, const float Value) const
    {
        assert(Type == EBufferType::Data);
        if (X < 0 || X >= Width || Y < 0 || Y >= Height)
        {
            return;
        }
        
        float* Ptr = static_cast<float*>(Memory) + GetOffset(X, Y);
        *Ptr = Value;
    }
    
    void SetPixel(uint32 X, uint32 Y, const PColor& Color) const
    {
        assert(Type == EBufferType::Color);
        if (X < 0 || X >= Width || Y < 0 || Y >= Height)
        {
            return;
        }

        uint32* Ptr = static_cast<uint32*>(Memory) + GetOffset(X, Y);

        // Placing pixels in memory order within a uint32:
        // BB GG RR AA
        // 00 00 00 00
        *Ptr = ((Color.R << 16) | Color.G << 8) | Color.B; // TODO: Disregard alpha channel for now
    }
    
    uint32 GetPixel(uint32 X, uint32 Y) const
    {
        return *(static_cast<uint32*>(Memory) + GetOffset(X, Y));
    }
    
    void Clear() const
    {
        PPlatformMemory::Fill(Memory, Width * Height * 4, 0);
    }

    template <typename T>
    void Fill(T Value)
    {
        PPlatformMemory::Fill(Memory, Width * Height * 4, Value);
    }
};

struct PBuffer
{
    int32* Memory;
    PBuffer(int32* Ptr)
    {
        Memory = Ptr;
    }
    ~PBuffer()
    {
        delete[] Memory;
    }
};

class PRenderer
{
    // Render channels
    std::map<const char*, std::shared_ptr<PChannel>> Channels;
    std::shared_ptr<PColorChannel> ColorChannel;
    std::shared_ptr<PDataChannel> DepthChannel;
    std::shared_ptr<PViewport> Viewport;
    std::unique_ptr<FGrid> Grid;

    // Buffers (vertex, index, etc.)
    std::vector<PBuffer> Buffers;
    
    void GenBuffer(int32* Ptr)
    {
        Buffers.emplace_back(Ptr);
    }
    template <typename T>
    void BindBuffer(T* Ptr)
    {
        
    }
    template <typename T>
    void BufferData(size_t Size, T* Data)
    {
        
    }

    // Constants
    const PColor WireColor = PColor::FromRgba(255, 175, 50);
    const PColor GridColor = PColor::FromRgba(128, 128, 128);

public:
    PRenderer(uint32 InWidth, uint32 InHeight);
    void Resize(uint32 InWidth, uint32 InHeight) const;

    void AddBuffer(EBufferType Type, const char* Name)
    {
        Channels.emplace(Name, std::make_shared<PChannel>(Type, GetWidth(), GetHeight()));
    }
    std::shared_ptr<PChannel> GetBuffer(const char* Name) const
    {
        return Channels.at(Name);
    }

    std::shared_ptr<PChannel> GetColorBuffer() const { return Channels.at("Color"); }
    std::shared_ptr<PChannel> GetDepthBuffer() const { return Channels.at("Depth"); }
    
    uint32 GetWidth() const { return Viewport->GetCamera()->Width; }
    uint32 GetHeight() const { return Viewport->GetCamera()->Height; }

    PViewport* GetViewport() const { return Viewport.get(); }

    // Drawing
    bool ClipLine(FVector2* A, FVector2* B) const;
    bool ClipLine(FLine* Line) const;
    void DrawLine(const FVector3& InA, const FVector3& InB, const PColor& Color) const;
    void DrawLine(const FLine3d& Line, const PColor& Color) const;
    void DrawTriangle(const FVector3& V0, const FVector3& V1, const FVector3& V2) const;
    void DrawMesh(const PMesh* Mesh) const;
    void DrawGrid() const;
    void Render() const;
    void ClearBuffers() const;
};
