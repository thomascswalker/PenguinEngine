#pragma once

#include "Camera.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "Framework/Engine/Mesh.h"
#include "glm.hpp"

struct IShader
{
    virtual ~IShader() = default;
    int32 Width, Height;
    PVertex V0, V1, V2;
    FVector3 S0, S1, S2;

    FVector3 CameraPosition;
    FVector3 CameraWorldDirection;

    FVector3 TriangleWorldNormal;
    FVector3 TriangleCameraNormal;

    FVector3 PixelWorldPosition;
    FVector3 PixelWorldNormal;
    float FacingRatio;

    FRect ScreenBounds;
    glm::mat4 MVP;
    glm::mat4 ViewMatrix;
    glm::mat4 ProjectionMatrix;
    FVector3 UVW;

    bool bHasNormals = false;
    bool bHasTexCoords = false;

    FColor OutColor = FColor::Magenta();

    PViewData ViewData;

    void Init(const PViewData& InViewData)
    {
        ViewData = InViewData;
        MVP = InViewData.ViewProjectionMatrix;
        CameraWorldDirection = InViewData.Direction;
        CameraPosition = InViewData.Translation;
        Width = InViewData.Width;
        Height = InViewData.Height;
    }

    virtual bool ComputeVertexShader(const PVertex& InV0, const PVertex& InV1, const PVertex& InV2)
    {
        V0 = InV0;
        V1 = InV1;
        V2 = InV2;

        // Project the world-space points to screen-space
        bool bTriangleOnScreen = false;
        bTriangleOnScreen |= Math::ProjectWorldToScreen(V0.Position, S0, ViewData);
        bTriangleOnScreen |= Math::ProjectWorldToScreen(V1.Position, S1, ViewData);
        bTriangleOnScreen |= Math::ProjectWorldToScreen(V2.Position, S2, ViewData);

        // If the triangle is completely off screen, exit
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

        // Grow the bounds by 1 pixel to account for gaps between pixels.
        ScreenBounds.Grow(1.0f);

        // Clamp the bounds to the viewport
        const FRect ViewportRect = {0, 0, static_cast<float>(Width), static_cast<float>(Height)};
        ScreenBounds.Clamp(ViewportRect);

        // Average each of the vertices' normals to get the triangle normal
        FVector4 V01Normal;
        VecAddVec(V0.Normal, V1.Normal, V01Normal);

        FVector4 V012Normal;
        VecAddVec(V01Normal, V2.Normal, V012Normal);

        TriangleWorldNormal = V012Normal * 0.33333333f;

        // Calculate the triangle normal relative to the camera
        FVector4 Tmp;
        VecCrossVec(TriangleWorldNormal, CameraWorldDirection, Tmp);
        TriangleCameraNormal.X = Tmp.X;
        TriangleCameraNormal.Y = Tmp.Y;
        TriangleCameraNormal.Z = Tmp.Z;

        return true;
    }

    // No default implementation
    virtual void ComputePixelShader(float X, float Y) = 0;
};

struct DefaultShader : IShader
{
    void ComputePixelShader(float U, float V) override
    {
        // Calculate the weighted normal of the current point on this triangle. This uses the UVW
        // barycentric coordinates to weight each vertex normal of the triangle.
        const FVector3 WeightedWorldNormal = V0.Normal * UVW.X + V1.Normal * UVW.Y + V2.Normal * UVW.Z;

        // Calculate the dot product of the triangle normal and camera direction
        VecDotVec(-CameraWorldDirection, WeightedWorldNormal, &FacingRatio);
        FacingRatio = Math::Max(0.0f, FacingRatio);                               // Floor to a min of 0
        const float ClampedFacingRatio = Math::Min(FacingRatio * 255.0f, 255.0f); // Clamp to a max of 255

        uint8 R = static_cast<uint8>(ClampedFacingRatio);

        OutColor = FColor::FromRgba(R, R, R);
    }
};
