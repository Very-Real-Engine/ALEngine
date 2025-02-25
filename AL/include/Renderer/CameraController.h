#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include "Core/Base.h"
#include "Core/Timestep.h"
#include "Renderer/Camera.h"
#include "Renderer/Common.h"

#include "Events/AppEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

namespace ale
{
class CameraController
{
  public:
	CameraController();

	void onUpdate(Timestep ts);
	void onEvent(Event &e);
	void onResize();
	const Camera &getCamera() const;
	Camera &getCamera();
	void setCamera(VkExtent2D swapChainExtent, float fov, float _near, float _far);

	static CameraController &get();
	alglm::mat4 getViewMatrix();
	alglm::mat4 getProjMatrix(VkExtent2D swapChainExtent);
	alglm::vec3 &getPosition()
	{
		return m_Camera.getPosition();
	}

  private:
	bool onMousePressed(MouseButtonPressedEvent &e);
	bool onMouseReleased(MouseButtonReleasedEvent &e);
	bool onWindowResized(WindowResizeEvent &e);
	bool onMouseMoved(MouseMovedEvent &e);

  private:
	Camera m_Camera;
	alglm::vec3 m_CameraPos{0.0f, 0.0f, 10.0f};
	alglm::vec2 m_prevMousePos{0.0f, 0.0f};
	bool m_CameraControl{false};
	float m_AspectRatio;
	alglm::vec3 m_CameraFront{0.0f, 0.0f, -1.0f};
	alglm::vec3 m_CameraUp{0.0f, 1.0f, 0.0f};
	const float m_Speed{0.05f};
	const float m_RotSpeed{0.8f};
	float m_CameraPitch{0.0f};
	float m_CameraYaw{0.0f};

	static CameraController *s_Instance;
};

} // namespace ale

#endif