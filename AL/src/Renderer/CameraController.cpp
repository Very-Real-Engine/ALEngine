#include "Renderer/CameraController.h"
#include "Core/Log.h"

#include "Core/App.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include <GLFW/glfw3.h>

namespace ale
{
CameraController *CameraController::s_Instance = nullptr;

CameraController::CameraController()
{
	s_Instance = this;

	// m_AspectRatio = WINDOW_WIDTH / (float)WINDOW_HEIGHT;
	// m_Camera.setProjMatrix(alglm::radians(45.0f), m_AspectRatio, 0.01f, 100.0f);
}

void CameraController::onUpdate(Timestep ts)
{
	if (!m_CameraControl)
		return;

	if (Input::isKeyPressed(Key::W))
	{
		m_CameraPos += m_CameraFront * m_Speed * ts.getMiliSeconds().count();
	}
	if (Input::isKeyPressed(Key::S))
	{
		m_CameraPos -= m_CameraFront * m_Speed * ts.getMiliSeconds().count();
	}

	auto cameraRight = alglm::normalize(alglm::cross(m_CameraUp, -m_CameraFront));
	if (Input::isKeyPressed(Key::A))
	{
		m_CameraPos -= cameraRight * m_Speed * ts.getMiliSeconds().count();
	}
	if (Input::isKeyPressed(Key::D))
	{
		m_CameraPos += cameraRight * m_Speed * ts.getMiliSeconds().count();
	}
	m_Camera.setPosition(m_CameraPos);
}

void CameraController::onEvent(Event &e)
{
	EventDispatcher dispatcher(e);

	dispatcher.dispatch<MouseButtonPressedEvent>(AL_BIND_EVENT_FN(CameraController::onMousePressed));
	dispatcher.dispatch<MouseButtonReleasedEvent>(AL_BIND_EVENT_FN(CameraController::onMouseReleased));
	dispatcher.dispatch<MouseMovedEvent>(AL_BIND_EVENT_FN(CameraController::onMouseMoved));
	dispatcher.dispatch<WindowResizeEvent>(AL_BIND_EVENT_FN(CameraController::onWindowResized));
}

void CameraController::onResize()
{
}

const Camera &CameraController::getCamera() const
{
	return m_Camera;
}

Camera &CameraController::getCamera()
{
	return m_Camera;
}

void CameraController::setCamera(VkExtent2D swapChainExtent, float fov, float _near, float _far)
{
	// m_AspectRatio = swapChainExtent.width / (float)swapChainExtent.height;
	// m_Camera.setProjMatrix(fov, m_AspectRatio, _near, _far);
}

alglm::mat4 CameraController::getViewMatrix()
{
	m_CameraFront = alglm::rotate(alglm::mat4(1.0f), alglm::radians(m_CameraYaw), alglm::vec3(0.0f, 1.0f, 0.0f)) *
					alglm::rotate(alglm::mat4(1.0f), alglm::radians(m_CameraPitch), alglm::vec3(1.0f, 0.0f, 0.0f)) *
					alglm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
	m_CameraPos = m_Camera.getPosition();
	return alglm::lookAt(m_CameraPos, m_CameraPos + m_CameraFront, m_CameraUp);
}

alglm::mat4 CameraController::getProjMatrix(VkExtent2D swapChainExtent)
{
	return alglm::perspective(alglm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f,
							  100.0f);
}

bool CameraController::onMousePressed(MouseButtonPressedEvent &e)
{
	if (e.getMouseButton() == Mouse::ButtonRight)
	{
		m_CameraControl = true;
		m_prevMousePos = Input::getMousePosition();
	}
	return false;
}

bool CameraController::onMouseReleased(MouseButtonReleasedEvent &e)
{
	m_CameraControl = false;
	return false;
}

bool CameraController::onWindowResized(WindowResizeEvent &e)
{
	return false;
}

bool CameraController::onMouseMoved(MouseMovedEvent &e)
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

CameraController &CameraController::get()
{
	return *s_Instance;
}

} // namespace ale