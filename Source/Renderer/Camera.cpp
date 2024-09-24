#include "Renderer/Camera.h"
#include "Engine/Engine.h"

// View Camera
void Camera::computeViewProjectionMatrix()
{
	const vec3f translation = m_spherical.toCartesian();
	const vec3f eye         = translation;
	const vec3f center      = m_target;
	const vec3f up          = vec3f::upVector();

	m_viewMatrix              = mat4f_lookat(eye, center, up);
	m_projectionMatrix        = mat4f_persp(Math::degreesToRadians(m_fov), getAspect(), m_minZ, m_maxZ);
	m_viewProjectionMatrix    = m_projectionMatrix * m_viewMatrix;
	m_invViewProjectionMatrix = m_viewProjectionMatrix.getInverse();
}

void Camera::orbit(const float dx, const float dy)
{
	m_sphericalDelta.theta = Math::degreesToRadians(-dx); // Horizontal, not sure why dx needs to be negated
	m_sphericalDelta.phi   = Math::degreesToRadians(dy);  // Vertical
}

void Camera::pan(const float dx, const float dy)
{
	// Compute target distance
	const vec3f position = getTranslation();
	const vec3f offset   = position - m_target;

	// The length of the Offset vector gives us the distance from the camera to the target.
	float targetDistance = offset.length();

	// Next, we need to scale this distance by the tangent of half the field of view.
	// This is because the field of view is measured in degrees, but the tangent function expects an angle in radians.
	// We also divide by the height of the viewport to account for the aspect ratio.
	targetDistance *= std::tanf((m_fov / 2.0f) * PI / 180.0f);

	// Pan left/right
	vec3f xOffset = {m_viewMatrix.m[0][0], m_viewMatrix.m[1][0], m_viewMatrix.m[2][0]}; // x rotation, column 0
	xOffset.normalize();
	xOffset *= dx * targetDistance / static_cast<float>(m_height);
	m_panOffset = xOffset;

	// Pan up/down
	vec3f yOffset = {m_viewMatrix.m[0][1], m_viewMatrix.m[1][1], m_viewMatrix.m[2][1]}; // y rotation, column 1
	yOffset.normalize();
	yOffset *= dy * targetDistance / static_cast<float>(m_height);
	m_panOffset += yOffset;
}

void Camera::zoom(const float value)
{
	vec3f translation  = getTranslation();
	m_spherical        = sphericalf::fromCartesian(translation.x, translation.y, translation.z);
	m_spherical.radius = std::max(m_minZoom, std::min(m_spherical.radius - (value * 0.1f), m_maxZoom));
	translation        = m_spherical.toCartesian();
	setTranslation(translation);
}

void Camera::setFov(const float newFov)
{
	m_fov = std::clamp(newFov, m_minFov, m_maxFov);
}

void Camera::update(float deltaTime)
{
	// Get the offset from the current camera position to the target position
	const vec3f position = getTranslation();
	vec3f offset         = position - m_target;

	// Convert offset to spherical coordinates
	m_spherical = sphericalf::fromCartesian(offset.x, offset.y, offset.z);

	// Offset spherical coordinates by the current spherical delta
	m_spherical.theta += m_sphericalDelta.theta;
	m_spherical.phi += m_sphericalDelta.phi;

	// Restrict phi to min/max polar angle to prevent locking
	m_spherical.phi = std::max(m_minPolarAngle, std::min(m_maxPolarAngle, m_spherical.phi));
	m_spherical.makeSafe(0.1f);

	// Set camera rotation pitch/yaw
	const rotf newRotation(
		Math::radiansToDegrees(m_spherical.theta), // yaw
		Math::radiansToDegrees(m_spherical.phi),   // pitch
		0.0f                                       // roll
	);
	setRotation(newRotation);

	// Convert spherical coordinates back to position
	offset = m_spherical.toCartesian();

	// Offset the target position based on the computed PanOffset (Camera::Pan)
	m_target += m_panOffset;

	// Set the camera position to the target position + offset
	setTranslation(m_target + offset);
}

void Camera::deprojectScreenToWorld(const vec2f& screenPoint, vec3f& outWorldPosition,
                                    vec3f& outWorldDirection) const
{
	const int32 pixelX = static_cast<int32>(screenPoint.x);
	const int32 pixelY = static_cast<int32>(screenPoint.y);

	// Convert to 0..1
	const float normalizedX = (pixelX - 0.5f) / static_cast<float>(m_width);
	const float normalizedY = (pixelY - 0.5f) / static_cast<float>(m_height);

	// Convert to -1..1
	const float screenSpaceX = (normalizedX - 0.5f) * 2.0f;
	const float screenSpaceY = ((1.0f - normalizedY) - 0.5f) * 2.0f;

	// Starting ray, z=1, near
	const vec4f rayStartProjectionSpace(screenSpaceX, screenSpaceY, 1.0f, 1.0f);
	// Ending ray z=0.1, far, any distance in order to calculate the direction
	const vec4f rayEndProjectionSpace(screenSpaceX, screenSpaceY, 0.01f, 1.0f);

	//
	const vec4f homoRayStartWorldSpace = m_invViewProjectionMatrix * rayStartProjectionSpace;
	const vec4f homoRayEndWorldSpace   = m_invViewProjectionMatrix * rayEndProjectionSpace;
	vec3f rayStartWorldSpace(homoRayStartWorldSpace.x, homoRayStartWorldSpace.y, homoRayStartWorldSpace.z);
	vec3f rayEndWorldSpace(homoRayEndWorldSpace.x, homoRayEndWorldSpace.y, homoRayEndWorldSpace.z);

	if (homoRayStartWorldSpace.w != 0.0f)
	{
		rayStartWorldSpace /= homoRayStartWorldSpace.w;
	}
	if (homoRayEndWorldSpace.w != 0.0f)
	{
		rayEndWorldSpace /= homoRayEndWorldSpace.w;
	}

	vec3f rayDirWorldSpace = rayEndWorldSpace - rayStartWorldSpace;
	rayDirWorldSpace       = rayDirWorldSpace.normalized();

	outWorldPosition  = vec3f{rayStartWorldSpace.x, rayStartWorldSpace.y, rayStartWorldSpace.z};
	outWorldDirection = vec3f{rayDirWorldSpace.x, rayDirWorldSpace.y, rayDirWorldSpace.z};
}
