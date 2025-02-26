#include "Renderer/Camera.h"
#include "ALpch.h"

namespace ale
{
Camera::Camera()
{
	updateProjMatrix();
	setViewMatrix(m_cameraPos, m_cameraFront, m_cameraUp);
}

Camera::Camera(float fov, float aspect, float _near, float _far)
{
	setProjMatrix(fov, aspect, _near, _far);
	setViewMatrix(m_cameraPos, m_cameraFront, m_cameraUp);
}

void Camera::setProjMatrix(float fov, float aspect, float _near, float _far)
{
	m_fov = fov;
	m_aspect = aspect;
	m_near = _near;
	m_far = _far;

	m_projection = alglm::perspective(m_fov, m_aspect, m_near, m_far);
}

void Camera::setViewMatrix(alglm::vec3 &pos, alglm::vec3 &dir, alglm::vec3 &up)
{
	m_view = alglm::lookAt(pos, pos + dir, up);
}

void Camera::setViewportSize(uint32_t width, uint32_t height)
{
	m_aspect = static_cast<float>(width) / static_cast<float>(height);
	updateProjMatrix();
}

void Camera::updateProjMatrix()
{
	m_projection = alglm::perspective(m_fov, m_aspect, m_near, m_far);
}

void Camera::updateViewMatrix()
{
	m_view = alglm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
}

void Camera::setPosition(alglm::vec3 &pos)
{
	m_cameraPos = pos;
}

void Camera::setRotation(alglm::vec3 &rot)
{
	auto &rotation = alglm::degrees(rot);
	m_CameraYaw = rotation.x;
	m_CameraPitch = rotation.y;

	if (m_CameraYaw < 0.0f)
		m_CameraYaw += 360.0f;
	if (m_CameraYaw > 360.0f)
		m_CameraYaw -= 360.0f;

	if (m_CameraPitch > 89.0f)
		m_CameraPitch = 89.0f;
	if (m_CameraPitch < -89.0f)
		m_CameraPitch = -89.0f;

	m_cameraFront = alglm::rotate(alglm::mat4(1.0f), alglm::radians(m_CameraYaw), alglm::vec3(0.0f, 1.0f, 0.0f)) *
					alglm::rotate(alglm::mat4(1.0f), alglm::radians(m_CameraPitch), alglm::vec3(1.0f, 0.0f, 0.0f)) *
					alglm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
}

void Camera::setAspectRatio(float ratio)
{
	m_aspect = ratio;
	updateProjMatrix();
}

void Camera::setFov(float fov)
{
	m_fov = fov;
	updateProjMatrix();
}

void Camera::setNear(float _near)
{
	m_near = _near;
	updateProjMatrix();
}

void Camera::setFar(float _far)
{
	m_far = _far;
	updateProjMatrix();
}

float Camera::getFov() const
{
	return m_fov;
}

float Camera::getNear() const
{
	return m_near;
}

float Camera::getFar() const
{
	return m_far;
}

alglm::vec3 &Camera::getPosition()
{
	return m_cameraPos;
}

const alglm::mat4 &Camera::getProjection() const
{
	return m_projection;
}

const alglm::mat4 &Camera::getView() const
{
	return m_view;
}

const Frustum &Camera::getFrustum()
{
	alglm::mat4 toWorldMatrix = alglm::inverse(m_view);

	// 0: center, 1: leftUp, 2: leftDown, 3: rightDown, 4: rightUp
	alglm::vec3 farPoint[5];
	alglm::vec3 nearPoint[5];

	// 0: Y, 1: X
	alglm::vec3 farXscale(0.0f);
	alglm::vec3 farYscale(0.0f);
	alglm::vec3 nearXscale(0.0f);
	alglm::vec3 nearYscale(0.0f);

	float frustumNear = m_near * 0.9f;
	float frustumFar = m_far * 1.1f;
	float fov = std::min(m_fov * 1.1f, 6.28f);

	nearPoint[0] = alglm::vec3(0.0f, 0.0f, -1.0f) * frustumNear;
	farPoint[0] = alglm::vec3(0.0f, 0.0f, -1.0f) * frustumFar;

	farYscale.y = frustumFar * tan(fov * 0.5f);
	farXscale.x = m_aspect * farYscale.y;

	nearYscale.y = frustumNear * tan(fov * 0.5f);
	nearXscale.x = m_aspect * nearYscale.y;

	farPoint[1] = toWorldMatrix * alglm::vec4((farPoint[0] + farYscale - farXscale), 1.0f);
	farPoint[2] = toWorldMatrix * alglm::vec4((farPoint[0] - farYscale - farXscale), 1.0f);
	farPoint[3] = toWorldMatrix * alglm::vec4((farPoint[0] - farYscale + farXscale), 1.0f);
	farPoint[4] = toWorldMatrix * alglm::vec4((farPoint[0] + farYscale + farXscale), 1.0f);

	nearPoint[1] = toWorldMatrix * alglm::vec4((nearPoint[0] + nearYscale - nearXscale), 1.0f);
	nearPoint[2] = toWorldMatrix * alglm::vec4((nearPoint[0] - nearYscale - nearXscale), 1.0f);
	nearPoint[3] = toWorldMatrix * alglm::vec4((nearPoint[0] - nearYscale + nearXscale), 1.0f);
	nearPoint[4] = toWorldMatrix * alglm::vec4((nearPoint[0] + nearYscale + nearXscale), 1.0f);

	m_frustum.plane[0] = FrustumPlane(nearPoint[1], nearPoint[2], nearPoint[3]);
	m_frustum.plane[1] = FrustumPlane(farPoint[4], farPoint[3], farPoint[2]);
	m_frustum.plane[2] = FrustumPlane(farPoint[1], farPoint[2], nearPoint[2]);
	m_frustum.plane[3] = FrustumPlane(nearPoint[4], nearPoint[3], farPoint[3]);
	m_frustum.plane[4] = FrustumPlane(farPoint[4], farPoint[1], nearPoint[1]);
	m_frustum.plane[5] = FrustumPlane(farPoint[2], farPoint[3], nearPoint[3]);

	return m_frustum;
}

} // namespace ale