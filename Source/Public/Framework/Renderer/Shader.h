#pragma once

#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "glm.hpp"

struct IShader
{
    virtual ~IShader() = default;
    int32 Width, Height;
    FVector3 V0, V1, V2;
    FVector3 S0, S1, S2;
    FVector3 CameraTranslation;
    FVector3 WorldNormal;
    FVector3 CameraNormal;
    float FacingRatio;
    FRect ScreenBounds;
    glm::mat4 MVP;
    
    FColor OutColor = FColor::Magenta();
    
    void Init(glm::mat4 InMVP,
        const FVector3& InV0, const FVector3& InV1, const FVector3& InV2,
        const FVector3& InCameraNormal,
        const FVector3& InCameraTranslation,
        int32 InWidth, int32 InHeight)
    {
        MVP = InMVP;
        V0 = InV0;
        V1 = InV1;
        V2 = InV2;
        WorldNormal = Math::GetSurfaceNormal(V0, V1, V2);
        CameraNormal = InCameraNormal;
        CameraTranslation = InCameraTranslation;
        Width = InWidth;
        Height = InHeight;
    }

    virtual bool Clip(const FVector3& WorldPosition, FVector3& ScreenPosition)
    {
        // Clip 
        glm::vec4 ResultGlm = MVP * glm::vec4(WorldPosition.X, WorldPosition.Y, WorldPosition.Z, 1.0f);
        FVector4 Result{ResultGlm.x, ResultGlm.y, ResultGlm.z, ResultGlm.w};
        if (Result.W > 0.0f)
        {
            // Apply perspective correction
            const FVector3 ClipPosition{
                Result.X / Result.W,
                Result.Y / Result.W,
                Result.Z / Result.W
            };

            // Normalized device coordinates
            const FVector2 NormalizedPosition{
                (ClipPosition.X / 2.0f) + 0.5f,
                (ClipPosition.Y / 2.0f) + 0.5f,
            };

            // Apply the current render width and height
            ScreenPosition = FVector3{
                NormalizedPosition.X * static_cast<float>(Width),
                NormalizedPosition.Y * static_cast<float>(Height),
                (Result.Z + 0.5f) * 0.5f
            };
            return true;
        }
        return false;
    }
    
    virtual bool ComputeVertexShader()
    {
        // Project the world-space points to screen-space
        bool bTriangleOnScreen = false;
        bTriangleOnScreen |= Clip(V0, S0);
        bTriangleOnScreen |= Clip(V1, S1);
        bTriangleOnScreen |= Clip(V2, S2);

        if (!bTriangleOnScreen)
        {
            return false;
        }

        // Reverse the order to CCW if the order is CW
        switch (Math::GetVertexOrder(S0, S1, S2))
        {
        case EWindingOrder::CW : // Triangle is back-facing, exit
        case EWindingOrder::CL : // Triangle has zero area, exit
            return false;
        case EWindingOrder::CCW : // Triangle is front-facing, continue
            break;
        }
        
        // Get the bounding box of the 2d triangle clipped to the viewport
        ScreenBounds = FRect::MakeBoundingBox(S0, S1, S2);

        // Grow by one pixel to accomodate gaps between triangles
        ScreenBounds.Grow(1.0f);

        // Clamp the triangle bounds to the viewport bounds
        const FRect ViewportRect = {0, 0, static_cast<float>(Width), static_cast<float>(Height)};
        ScreenBounds.Clamp(ViewportRect);

        // Get the current camera attributes
        FacingRatio = Math::Dot(WorldNormal, CameraNormal);
        
        // TODO: Figure out how to exit early if back-facing

        return true;
    }

    // No default implementation
    virtual void ComputePixelShader() = 0;
};

struct DefaultShader : IShader
{
    void ComputePixelShader() override
    {
        float RemappedFacingRatio = Math::Remap(FacingRatio, -1.0f, 1.0f, 0.0f, 1.0f);
        int8 R = static_cast<int8>(RemappedFacingRatio * 255.0f);
        int8 G = static_cast<int8>(RemappedFacingRatio * 255.0f);
        int8 B = static_cast<int8>(RemappedFacingRatio * 255.0f);
        OutColor = FColor::FromRgba(R,G,B);
    }
};