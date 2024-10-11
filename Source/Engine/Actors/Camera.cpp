#include "Camera.h"

#include "Engine/Engine.h"

Camera::Camera()
{
	setTranslation(g_defaultCameraTranslation);
	m_spherical = sphericalf(g_defaultCameraTranslation);
}

constexpr float Camera::getAspect() const
{
	return static_cast<float>(m_width) / static_cast<float>(m_height);
}

// View Camera
void Camera::computeViewProjectionMatrix()
{
	vec3f position = m_transform.translation;
	vec3f target   = m_targetTranslation + VERY_SMALL_NUMBER; // In case the target is at [0,0,0]
	vec3f up       = vec3f::upVector();

	m_viewMatrix              = lookAtLH(position, target, up);
	m_projectionMatrix        = perspectiveFovLH(m_fov * DEG_TO_RAD, (float)m_width / (float)m_height, m_minZ, m_maxZ);
	m_viewProjectionMatrix    = m_viewMatrix * m_projectionMatrix;
	m_invViewProjectionMatrix = m_viewProjectionMatrix.getInverse();
}

void Camera::orbit(const float dx, const float dy)
{
	m_deltaRotation.rotateRight(dx * m_orbitSpeed); // Horizontal
	m_deltaRotation.rotateUp(dy * m_orbitSpeed);    // Vertical
}

void Camera::pan(const float dx, const float dy)
{
	float speed = m_panSpeed;
	m_deltaTranslation += getUpVector() * dx * speed;
	m_deltaTranslation += getRightVector() * dy * speed;
}

void Camera::zoom(const float value)
{
	m_zoom = std::max(m_minZoom, std::min(m_spherical.radius - value * m_zoomSpeed, m_maxZoom));
}

void Camera::update(float deltaTime)
{
	m_spherical.radius = m_zoom;

	// Offset spherical coordinates by the current spherical delta
	m_spherical.theta += m_deltaRotation.theta;
	m_spherical.phi += m_deltaRotation.phi;

	// Restrict phi to min/max polar angle to prevent gimbal lock
	m_spherical.makeSafe(SMALL_NUMBER);

	// Set camera rotation pitch/yaw
	const rotf newRotation(
		Math::radiansToDegrees(m_spherical.theta), // yaw
		Math::radiansToDegrees(m_spherical.phi),   // pitch
		0.0f                                       // roll
	);
	setRotation(newRotation);

	// Offset the target position based on the computed PanOffset (Camera::Pan)
	m_targetTranslation += m_deltaTranslation;

	// Convert spherical coordinates back to position
	vec3f newTranslation = m_spherical.toCartesian() + m_targetTranslation;

	// Set the camera position to the target position + offset
	setTranslation(newTranslation);

	// Reset the delta after rotation and translation have been set
	m_deltaTranslation.zero();
	m_deltaRotation.zero();

	// Update VP matrix
	computeViewProjectionMatrix();
}

void Camera::setFov(const float newFov)
{
	m_fov = std::clamp(newFov, m_minFov, m_maxFov);
}

float Camera::getTargetDistance() const
{
	// Compute target distance
	const vec3f offset = getTranslation() - m_targetTranslation;

	// The length of the Offset vector gives us the distance from the camera to the target.
	return offset.length();
}

void Camera::setLookAt(const vec3f& newLookAt)
{
	m_targetTranslation = newLookAt;
}

ViewData* Camera::getViewData()
{
	m_viewData.width                   = m_width;
	m_viewData.height                  = m_height;
	m_viewData.fov                     = m_fov;
	m_viewData.minZ                    = m_minZ;
	m_viewData.maxZ                    = m_maxZ;
	m_viewData.target                  = m_targetTranslation;
	m_viewData.viewProjectionMatrix    = m_viewProjectionMatrix;
	m_viewData.cameraDirection         = getForwardVector();
	m_viewData.cameraTranslation       = getTranslation();
	return &m_viewData;
}

void Camera::setDefault()
{
	m_transform.translation = g_defaultCameraTranslation;
	m_spherical.phi         = g_defaultPhi;
	m_spherical.theta       = g_defaultTheta;
	m_zoom                  = g_defaultZoom;
	m_targetTranslation     = vec3f::zeroVector();
	computeViewProjectionMatrix();
}
