#include "Framework/Renderer/Camera.h"
#include "Framework/Engine/Engine.h"

// View Camera
void PCamera::computeViewProjectionMatrix()
{
	FVector3 Translation = m_spherical.ToCartesian();
	FVector3 Eye = Translation;
	FVector3 Center = m_target;
	FVector3 Up = FVector3::UpVector();

	m_viewMatrix = FLookAtMatrix(Eye, Center, Up);
	m_projectionMatrix = FPerspectiveMatrix(Math::DegreesToRadians(m_fov), getAspect(), m_minZ, m_maxZ);
	m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
	m_invViewProjectionMatrix = m_viewProjectionMatrix.GetInverse();
}

void PCamera::orbit(float dx, float dy)
{
	m_sphericalDelta.Theta = Math::DegreesToRadians(-dx); // Horizontal
	m_sphericalDelta.Phi = Math::DegreesToRadians(dy); // Vertical
}

void PCamera::pan(float dx, float dy)
{
	// Compute target distance
	FVector3 Position = GetTranslation();
	FVector3 Offset = Position - m_target;

	// The length of the Offset vector gives us the distance from the camera to the target.
	float TargetDistance = Offset.Length();

	// Next, we need to scale this distance by the tangent of half the field of view.
	// This is because the field of view is measured in degrees, but the tangent function expects an angle in radians.
	// We also divide by the height of the viewport to account for the aspect ratio.
	TargetDistance *= Math::Tan((m_fov / 2.0f) * P_PI / 180.0f);

	// Pan left/right
	FVector3 XOffset = {m_viewMatrix.M[0][0], m_viewMatrix.M[1][0], m_viewMatrix.M[2][0]}; // X Rotation, column 0
	XOffset.Normalize();
	XOffset *= dx * TargetDistance / static_cast<float>(m_height);
	m_panOffset = XOffset;

	// Pan up/down
	FVector3 YOffset = {m_viewMatrix.M[0][1], m_viewMatrix.M[1][1], m_viewMatrix.M[2][1]}; // Y Rotation, column 1
	YOffset.Normalize();
	YOffset *= dy * TargetDistance / static_cast<float>(m_height);
	m_panOffset += YOffset;
}

void PCamera::zoom(float value)
{
	FVector3 Translation = GetTranslation();
	m_spherical = FSphericalCoords::FromCartesian(Translation.X, Translation.Y, Translation.Z);
	m_spherical.Radius = Math::Max(m_minZoom, Math::Min(m_spherical.Radius - (value * 0.1f), m_maxZoom));
	Translation = m_spherical.ToCartesian();
	SetTranslation(Translation);
}

void PCamera::setFov(float newFov)
{
	m_fov = Math::Clamp(newFov, m_minFov, m_maxFov);
}

void PCamera::Update(float deltaTime)
{
	// Get the offset from the current camera position to the target position
	FVector3 Position = GetTranslation();
	FVector3 Offset = Position - m_target;

	// Convert offset to spherical coordinates
	m_spherical = FSphericalCoords::FromCartesian(Offset.X, Offset.Y, Offset.Z);

	// Offset spherical coordinates by the current spherical delta
	m_spherical.Theta += m_sphericalDelta.Theta;
	m_spherical.Phi += m_sphericalDelta.Phi;

	// Restrict Phi to min/max polar angle to prevent locking
	m_spherical.Phi = Math::Max(m_minPolarAngle, Math::Min(m_maxPolarAngle, m_spherical.Phi));
	m_spherical.MakeSafe(0.1f);

	// Set camera rotation pitch/yaw
	FRotator NewRotation(
		Math::RadiansToDegrees(m_spherical.Theta), // Yaw
		Math::RadiansToDegrees(m_spherical.Phi), // Pitch
		0.0f // Roll
	);
	SetRotation(NewRotation);

	// Convert spherical coordinates back to position
	Offset = m_spherical.ToCartesian();

	// Offset the target position based on the computed PanOffset (PCamera::Pan)
	m_target += m_panOffset;

	// Set the camera position to the target position + offset
	SetTranslation(m_target + Offset);
}

void PCamera::deprojectScreenToWorld(const FVector2& screenPoint, FVector3& outWorldPosition,
                                     FVector3& outWorldDirection) const
{
	int32 PixelX = static_cast<int32>(screenPoint.X);
	int32 PixelY = static_cast<int32>(screenPoint.Y);

	// Convert to 0..1
	const float NormalizedX = (PixelX - 0.5f) / static_cast<float>(m_width);
	const float NormalizedY = (PixelY - 0.5f) / static_cast<float>(m_height);

	// Convert to -1..1
	const float ScreenSpaceX = (NormalizedX - 0.5f) * 2.0f;
	const float ScreenSpaceY = ((1.0f - NormalizedY) - 0.5f) * 2.0f;

	// Starting ray, Z=1, near
	FVector4 RayStartProjectionSpace(ScreenSpaceX, ScreenSpaceY, 1.0f, 1.0f);
	// Ending ray Z=0.1, far, any distance in order to calculate the direction
	FVector4 RayEndProjectionSpace(ScreenSpaceX, ScreenSpaceY, 0.01f, 1.0f);

	//
	FVector4 HomoRayStartWorldSpace = m_invViewProjectionMatrix * RayStartProjectionSpace;
	FVector4 HomoRayEndWorldSpace = m_invViewProjectionMatrix * RayEndProjectionSpace;
	FVector3 RayStartWorldSpace(HomoRayStartWorldSpace.X, HomoRayStartWorldSpace.Y, HomoRayStartWorldSpace.Z);
	FVector3 RayEndWorldSpace(HomoRayEndWorldSpace.X, HomoRayEndWorldSpace.Y, HomoRayEndWorldSpace.Z);

	if (HomoRayStartWorldSpace.W != 0.0f)
	{
		RayStartWorldSpace /= HomoRayStartWorldSpace.W;
	}
	if (HomoRayEndWorldSpace.W != 0.0f)
	{
		RayEndWorldSpace /= HomoRayEndWorldSpace.W;
	}

	FVector3 RayDirWorldSpace = RayEndWorldSpace - RayStartWorldSpace;
	RayDirWorldSpace = RayDirWorldSpace.Normalized();

	outWorldPosition = FVector3{RayStartWorldSpace.X, RayStartWorldSpace.Y, RayStartWorldSpace.Z};
	outWorldDirection = FVector3{RayDirWorldSpace.X, RayDirWorldSpace.Y, RayDirWorldSpace.Z};
}
