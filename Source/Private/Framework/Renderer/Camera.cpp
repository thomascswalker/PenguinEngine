#include "Framework/Renderer/Camera.h"
#include "Framework/Engine/Engine.h"

// View Camera
void Camera::computeViewProjectionMatrix()
{
	const vec3f translation = m_spherical.ToCartesian();
	const vec3f eye = translation;
	const vec3f center = m_target;
	const vec3f up = vec3f::UpVector();

	m_viewMatrix = mat4f_lookat(eye, center, up);
	m_projectionMatrix = mat4f_persp(Math::DegreesToRadians(m_fov), getAspect(), m_minZ, m_maxZ);
	m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
	m_invViewProjectionMatrix = m_viewProjectionMatrix.GetInverse();
}

void Camera::orbit(const float dx, const float dy)
{
	m_sphericalDelta.Theta = Math::DegreesToRadians(-dx); // Horizontal
	m_sphericalDelta.Phi = Math::DegreesToRadians(dy); // Vertical
}

void Camera::pan(const float dx, const float dy)
{
	// Compute target distance
	const vec3f position = getTranslation();
	const vec3f offset = position - m_target;

	// The length of the Offset vector gives us the distance from the camera to the target.
	float targetDistance = offset.Length();

	// Next, we need to scale this distance by the tangent of half the field of view.
	// This is because the field of view is measured in degrees, but the tangent function expects an angle in radians.
	// We also divide by the height of the viewport to account for the aspect ratio.
	targetDistance *= Math::Tan((m_fov / 2.0f) * PI / 180.0f);

	// Pan left/right
	vec3f xOffset = {m_viewMatrix.M[0][0], m_viewMatrix.M[1][0], m_viewMatrix.M[2][0]}; // X Rotation, column 0
	xOffset.Normalize();
	xOffset *= dx * targetDistance / static_cast<float>(m_height);
	m_panOffset = xOffset;

	// Pan up/down
	vec3f yOffset = {m_viewMatrix.M[0][1], m_viewMatrix.M[1][1], m_viewMatrix.M[2][1]}; // Y Rotation, column 1
	yOffset.Normalize();
	yOffset *= dy * targetDistance / static_cast<float>(m_height);
	m_panOffset += yOffset;
}

void Camera::zoom(const float value)
{
	vec3f translation = getTranslation();
	m_spherical = FSphericalCoords::FromCartesian(translation.X, translation.Y, translation.Z);
	m_spherical.Radius = Math::Max(m_minZoom, Math::Min(m_spherical.Radius - (value * 0.1f), m_maxZoom));
	translation = m_spherical.ToCartesian();
	setTranslation(translation);
}

void Camera::setFov(const float newFov)
{
	m_fov = Math::Clamp(newFov, m_minFov, m_maxFov);
}

void Camera::update(float deltaTime)
{
	// Get the offset from the current camera position to the target position
	const vec3f position = getTranslation();
	vec3f offset = position - m_target;

	// Convert offset to spherical coordinates
	m_spherical = FSphericalCoords::FromCartesian(offset.X, offset.Y, offset.Z);

	// Offset spherical coordinates by the current spherical delta
	m_spherical.Theta += m_sphericalDelta.Theta;
	m_spherical.Phi += m_sphericalDelta.Phi;

	// Restrict Phi to min/max polar angle to prevent locking
	m_spherical.Phi = Math::Max(m_minPolarAngle, Math::Min(m_maxPolarAngle, m_spherical.Phi));
	m_spherical.MakeSafe(0.1f);

	// Set camera rotation pitch/yaw
	const rotf newRotation(
		Math::RadiansToDegrees(m_spherical.Theta), // Yaw
		Math::RadiansToDegrees(m_spherical.Phi), // Pitch
		0.0f // Roll
	);
	setRotation(newRotation);

	// Convert spherical coordinates back to position
	offset = m_spherical.ToCartesian();

	// Offset the target position based on the computed PanOffset (Camera::Pan)
	m_target += m_panOffset;

	// Set the camera position to the target position + offset
	setTranslation(m_target + offset);
}

void Camera::deprojectScreenToWorld(const vec2f& screenPoint, vec3f& outWorldPosition,
                                    vec3f& outWorldDirection) const
{
	const int32 pixelX = static_cast<int32>(screenPoint.X);
	const int32 pixelY = static_cast<int32>(screenPoint.Y);

	// Convert to 0..1
	const float normalizedX = (pixelX - 0.5f) / static_cast<float>(m_width);
	const float normalizedY = (pixelY - 0.5f) / static_cast<float>(m_height);

	// Convert to -1..1
	const float screenSpaceX = (normalizedX - 0.5f) * 2.0f;
	const float screenSpaceY = ((1.0f - normalizedY) - 0.5f) * 2.0f;

	// Starting ray, Z=1, near
	const vec4f rayStartProjectionSpace(screenSpaceX, screenSpaceY, 1.0f, 1.0f);
	// Ending ray Z=0.1, far, any distance in order to calculate the direction
	const vec4f rayEndProjectionSpace(screenSpaceX, screenSpaceY, 0.01f, 1.0f);

	//
	const vec4f homoRayStartWorldSpace = m_invViewProjectionMatrix * rayStartProjectionSpace;
	const vec4f homoRayEndWorldSpace = m_invViewProjectionMatrix * rayEndProjectionSpace;
	vec3f rayStartWorldSpace(homoRayStartWorldSpace.X, homoRayStartWorldSpace.Y, homoRayStartWorldSpace.Z);
	vec3f rayEndWorldSpace(homoRayEndWorldSpace.X, homoRayEndWorldSpace.Y, homoRayEndWorldSpace.Z);

	if (homoRayStartWorldSpace.W != 0.0f)
	{
		rayStartWorldSpace /= homoRayStartWorldSpace.W;
	}
	if (homoRayEndWorldSpace.W != 0.0f)
	{
		rayEndWorldSpace /= homoRayEndWorldSpace.W;
	}

	vec3f rayDirWorldSpace = rayEndWorldSpace - rayStartWorldSpace;
	rayDirWorldSpace = rayDirWorldSpace.Normalized();

	outWorldPosition = vec3f{rayStartWorldSpace.X, rayStartWorldSpace.Y, rayStartWorldSpace.Z};
	outWorldDirection = vec3f{rayDirWorldSpace.X, rayDirWorldSpace.Y, rayDirWorldSpace.Z};
}
