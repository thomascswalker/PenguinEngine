#pragma once

#include <map>
#include <memory>

#include "Grid.h"
#include "Settings.h"
#include "Shader.h"
#include "Viewport.h"
#include "Framework/Engine/Mesh.h"
#include "Framework/Platforms/PlatformMemory.h"
#include "Math/MathCommon.h"

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
    EChannelType Type;

    PChannel(EChannelType InType, uint32 InWidth, uint32 InHeight) : Type(InType)
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
        assert(Type == EChannelType::Data);
        if (X < 0 || X >= Width || Y < 0 || Y >= Height)
        {
            return;
        }

        float* Ptr = static_cast<float*>(Memory) + GetOffset(X, Y);
        *Ptr = Value;
    }

    void SetPixel(uint32 X, uint32 Y, const FColor& Color) const
    {
        assert(Type == EChannelType::Color);
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

    template <typename T>
    T GetPixel(uint32 X, uint32 Y) const
    {
        return *(static_cast<T*>(Memory) + GetOffset(X, Y));
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

enum class EDataType
{
    Int8,
    Int16,
    Int32,
    UInt8,
    UInt16,
    UInt32,
    Float
};

enum class EBufferType
{
    Array,
    ElementArray
};

template <typename T>
struct PBufferObject
{
    std::vector<T> Memory;
    EDataType DataType;
    EBufferType BufferType;
    size_t Size = 0;

    PBufferObject(const EDataType InDataType, const EBufferType InBufferType)
    {
        DataType = InDataType;
        BufferType = InBufferType;
    }
};

class PRenderer
{
    // Draw channels
    std::map<const char*, std::shared_ptr<PChannel>> Channels;
    std::shared_ptr<PViewport> Viewport;
    std::unique_ptr<FGrid> Grid;

    std::unique_ptr<PBufferObject<float>> VertexBuffer;
    std::unique_ptr<PBufferObject<uint32>> IndexBuffer;

    // Constants
    const FColor WireColor = FColor::FromRgba(255, 175, 50);
    const FColor GridColor = FColor::FromRgba(128, 128, 128);

    // Shaders
    std::shared_ptr<IShader> CurrentShader = nullptr;

public:
    // Settings
    Renderer::PRenderSettings Settings;

    PRenderer(uint32 InWidth, uint32 InHeight);
    void Resize(uint32 InWidth, uint32 InHeight) const;
    uint32 GetWidth() const { return Viewport->GetCamera()->Width; }
    uint32 GetHeight() const { return Viewport->GetCamera()->Height; }
    PViewport* GetViewport() const { return Viewport.get(); }

    /* Channels */

    void AddChannel(EChannelType Type, const char* Name)
    {
        Channels.emplace(Name, std::make_shared<PChannel>(Type, GetWidth(), GetHeight()));
    }
    std::shared_ptr<PChannel> GetChannel(const char* Name) const
    {
        return Channels.at(Name);
    }
    void ClearChannels() const
    {
        // Set all channels to 0
        for (const auto& [Key, Channel] : Channels)
        {
            // Ignore the depth channel, we'll handle that later
            if (Key == "Depth") // NOLINT
            {
                continue;
            }
            Channel->Clear();
        }

        // Fill the depth buffer with the Max Z-depth
        GetDepthChannel()->Fill(DEFAULT_MAXZ);
    }

    std::shared_ptr<PChannel> GetColorChannel() const { return Channels.at("Color"); }
    std::shared_ptr<PChannel> GetDepthChannel() const { return Channels.at("Depth"); }

    /* Drawing */

    bool ClipLine(FVector2* A, FVector2* B) const;
    bool ClipLine(FLine* Line) const;
    void DrawLine(const FVector3& InA, const FVector3& InB, const FColor& Color) const;
    void DrawLine(const FLine3d& Line, const FColor& Color) const;
    void DrawTriangle(const PVertex& V0, const PVertex& V1, const PVertex& V2);
    void DrawMesh(const PMesh* Mesh);
    void DrawGrid() const;
    void Draw();

    // Rasterizing triangles
    
    void Scanline();
    void ScanlineFast();
};
