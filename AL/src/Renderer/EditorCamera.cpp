#include "Renderer/EditorCamera.h"

#include "Core/App.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"

namespace ale
{

EditorCamera::EditorCamera(float fov, float aspect, float _near, float _far) : Camera(fov, aspect, _near, _far) {};

void EditorCamera::onUpdate(Timestep ts)
{
	if (!m_CameraControl)
		return;

	if (Input::isKeyPressed(Key::W))
	{
		m_cameraPos += m_cameraFront * m_Speed * ts.getMiliSeconds().count();
	}
	if (Input::isKeyPressed(Key::S))
	{
		m_cameraPos -= m_cameraFront * m_Speed * ts.getMiliSeconds().count();
	}

	auto cameraRight = alglm::normalize(alglm::cross(m_cameraUp, -m_cameraFront));
	if (Input::isKeyPressed(Key::A))
	{
		m_cameraPos -= cameraRight * m_Speed * ts.getMiliSeconds().count();
	}
	if (Input::isKeyPressed(Key::D))
	{
		m_cameraPos += cameraRight * m_Speed * ts.getMiliSeconds().count();
	}

	updateView();
}

void EditorCamera::onEvent(Event &e)
{
	EventDispatcher dispatcher(e);

	dispatcher.dispatch<MouseButtonPressedEvent>(AL_BIND_EVENT_FN(EditorCamera::onMousePressed));
	dispatcher.dispatch<MouseButtonReleasedEvent>(AL_BIND_EVENT_FN(EditorCamera::onMouseReleased));
	dispatcher.dispatch<MouseMovedEvent>(AL_BIND_EVENT_FN(EditorCamera::onMouseMoved));
	dispatcher.dispatch<WindowResizeEvent>(AL_BIND_EVENT_FN(EditorCamera::onWindowResized));
}

bool EditorCamera::onMousePressed(MouseButtonPressedEvent &e)
{
	if (e.getMouseButton() == Mouse::ButtonRight)
	{
		m_CameraControl = true;
		m_prevMousePos = Input::getMousePosition();
	}
	return false;
}

bool EditorCamera::onMouseReleased(MouseButtonReleasedEvent &e)
{
	m_CameraControl = false;
	return false;
}

bool EditorCamera::onWindowResized(WindowResizeEvent &e)
{
	return false;
}

bool EditorCamera::onMouseMoved(MouseMovedEvent &e)
{
	if (m_CameraControl)
	{
		alglm::vec2 pos = alglm::vec2(e.getX(), e.getY());
		alglm::vec2 deltaPos = pos - m_prevMousePos;

		// set camera rotation
		m_CameraYaw -= deltaPos.x * m_RotSpeed;
		m_CameraPitch -= deltaPos.y * m_RotSpeed;

		if (m_CameraYaw < 0.0f)
			m_CameraYaw += 360.0f;
		if (m_CameraYaw > 360.0f)
			m_CameraYaw -= 360.0f;

		if (m_CameraPitch > 89.0f)
			m_CameraPitch = 89.0f;
		if (m_CameraPitch < -89.0f)
			m_CameraPitch = -89.0f;
		m_prevMousePos = pos;
	}
	return false;
}

void EditorCamera::updateView()
{
	m_cameraFront = alglm::rotate(alglm::mat4(1.0f), alglm::radians(m_CameraYaw), alglm::vec3(0.0f, 1.0f, 0.0f)) *
					alglm::rotate(alglm::mat4(1.0f), alglm::radians(m_CameraPitch), alglm::vec3(1.0f, 0.0f, 0.0f)) *
					alglm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
	// m_View = alglm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
	// setViewMatrix(m_cameraPos, m_cameraFront, m_cameraUp);
	updateViewMatrix();
}

} // namespace ale