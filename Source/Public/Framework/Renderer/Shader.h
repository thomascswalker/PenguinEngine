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

        // Grow by one pixel to accomodate gaps between triangles
        ScreenBounds.Grow(1.0f);

        // Clamp the bounds to the viewport
        const FRect ViewportRect = {0, 0, static_cast<float>(Width), static_cast<float>(Height)};
        ScreenBounds.Clamp(ViewportRect);

        // Average each of the vertices normals to get the triangle normal
        TriangleWorldNormal = (V0.Normal + V1.Normal + V2.Normal) / 3.0f;

        // Calculate the triangle normal relative to the camera
        TriangleCameraNormal = TriangleWorldNormal.Cross(CameraWorldDirection);

        return true;
    }

    // No default implementation
    virtual void ComputePixelShader(float X, float Y) = 0;
};

struct DefaultShader : IShader
{
    void ComputePixelShader(float U, float V) override
    {
        // Calculate the dot product of the triangle normal and camera direction
        FacingRatio = Math::Max(0.0f, Math::Dot(-CameraWorldDirection, TriangleWorldNormal)); // Floor to a min of 0
        float ClampedFacingRatio = Math::Min((1.0f - FacingRatio) * 255.0f, 255.0f);                   // Clamp to a max of 255

        uint8 R = static_cast<uint8>(ClampedFacingRatio);
        uint8 G = static_cast<uint8>(ClampedFacingRatio);
        uint8 B = static_cast<uint8>(ClampedFacingRatio);

        OutColor = FColor::FromRgba(R, G, B);
    }
};
