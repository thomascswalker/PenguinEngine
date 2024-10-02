#include "Renderer/Camera.h"
#include "Engine/Engine.h"

// View Camera
void Camera::computeViewProjectionMatrix()
{
	vec3f position = m_transform.translation;
	vec3f target   = m_target + VERY_SMALL_NUMBER; // In case the target is at [0,0,0]
	vec3f up       = vec3f::upVector();

	m_viewMatrix              = lookAtLH(position, target, up);
	m_projectionMatrix        = perspectiveFovLH(m_fov * DEG_TO_RAD, (float)m_width / (float)m_height, m_minZ, m_maxZ);
	m_viewProjectionMatrix    = m_viewMatrix * m_projectionMatrix;
	m_invViewProjectionMatrix = m_viewProjectionMatrix.getInverse();
}

void Camera::orbit(const float dx, const float dy)
{
	m_sphericalDelta.theta = dx * DEG_TO_RAD; // Horizontal
	m_sphericalDelta.phi   = dy * DEG_TO_RAD; // Vertical
}

void Camera::pan(const float dx, const float dy)
{
	float speed = 0.001f;
#ifndef PENG_HARDWARE_ACCELERATION
	m_panOffset += getUpVector() * dx * speed;
	m_panOffset += getRightVector() * dy * speed;
#else
	m_panOffset += getUpVector() * dy * speed;
	m_panOffset += getRightVector() * dx * speed;
#endif
}

void Camera::zoom(const float value)
{
	m_zoom = std::max(m_minZoom, std::min(m_spherical.radius - value, m_maxZoom));
}

void Camera::update(float deltaTime)
{
	// Get the offset from the current camera position to the target position
	const vec3f position = getTranslation();
	vec3f offset         = position - m_target;

	// Convert offset to spherical coordinates
	m_spherical        = sphericalf::fromCartesian(offset.x, offset.y, offset.z);
	m_spherical.radius = m_zoom;

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

void Camera::setFov(const float newFov)
{
	m_fov = std::clamp(newFov, m_minFov, m_maxFov);
}

float Camera::getTargetDistance() const
{
	// Compute target distance
	const vec3f position = getTranslation();
	const vec3f offset   = position - m_target;

	// The length of the Offset vector gives us the distance from the camera to the target.
	return offset.length();
}

ViewData* Camera::getViewData()
{
	m_viewData.width                   = m_width;
	m_viewData.height                  = m_height;
	m_viewData.fov                     = m_fov;
	m_viewData.minZ                    = m_minZ;
	m_viewData.maxZ                    = m_maxZ;
	m_viewData.target                  = m_target;
	m_viewData.spherical               = m_spherical;
	m_viewData.projectionMatrix        = m_projectionMatrix;
	m_viewData.viewMatrix              = m_viewMatrix;
	m_viewData.viewProjectionMatrix    = m_viewProjectionMatrix;
	m_viewData.invViewProjectionMatrix = m_invViewProjectionMatrix;
	m_viewData.cameraDirection         = getForwardVector();
	m_viewData.cameraTranslation       = getTranslation();
	return &m_viewData;
}

void Camera::setDefault()
{
	m_transform.translation = g_defaultCameraTranslation;
	m_transform.rotation    = rotf();
	m_zoom                  = g_defaultZoom;

	m_target = vec3f::zeroVector();
	computeViewProjectionMatrix();
}
