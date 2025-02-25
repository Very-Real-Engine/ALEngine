#pragma once

#include "Core/Base.h"
#include "Core/Timestep.h"
#include "Renderer/Camera.h"
#include "Renderer/Common.h"

#include "Events/AppEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

namespace ale
{
/**
 * @class EditorCamera
 * @brief 에디터에서 사용하는 카메라 클래스.
 *        사용자의 입력(마우스, 키보드)에 따라 이동 및 회전이 가능하며,
 *        Camera 클래스를 상속하여 구현됨.
 */
class EditorCamera : public Camera
{
  public:
	/// @brief EditorCamera 생성자.
	EditorCamera() = default;

	/**
	 * @brief FOV(시야각), 화면 비율, 근평면 및 원평면을 설정하는 생성자.
	 * @param fov 시야각(Field of View).
	 * @param aspect 화면 비율(Aspect Ratio).
	 * @param _near 근평면(Near Plane).
	 * @param _far 원평면(Far Plane).
	 */
	EditorCamera(float fov, float aspect, float _near, float _far);

	/// @brief EditorCamera 가상 소멸자.
	virtual ~EditorCamera() = default;

	/**
	 * @brief 카메라를 업데이트합니다.
	 * @param ts 프레임 간 시간(Timestep).
	 */
	void onUpdate(Timestep ts);

	/**
	 * @brief 이벤트를 처리합니다.
	 * @param e 이벤트 객체.
	 */
	void onEvent(Event &e);

  private:
	/**
	 * @brief 마우스 버튼이 눌렸을 때 호출됩니다.
	 * @param e 마우스 버튼 이벤트 객체.
	 * @return true 이벤트가 처리됨.
	 * @return false 이벤트가 처리되지 않음.
	 */
	bool onMousePressed(MouseButtonPressedEvent &e);
	
	/**
	 * @brief 마우스 버튼이 떼어졌을 때 호출됩니다.
	 * @param e 마우스 버튼 이벤트 객체.
	 * @return true 이벤트가 처리됨.
	 * @return false 이벤트가 처리되지 않음.
	 */
	bool onMouseReleased(MouseButtonReleasedEvent &e);
	
	/**
	 * @brief 창 크기가 변경되었을 때 호출됩니다.
	 * @param e 창 크기 변경 이벤트 객체.
	 * @return true 이벤트가 처리됨.
	 * @return false 이벤트가 처리되지 않음.
	 */
	bool onWindowResized(WindowResizeEvent &e);

	/**
	 * @brief 마우스가 이동했을 때 호출됩니다.
	 * @param e 마우스 이동 이벤트 객체.
	 * @return true 이벤트가 처리됨.
	 * @return false 이벤트가 처리되지 않음.
	 */
	bool onMouseMoved(MouseMovedEvent &e);

	/// @brief 카메라의 뷰 행렬을 업데이트합니다.
	void updateView();

  private:
	glm::vec2 m_prevMousePos{0.0f, 0.0f};
	bool m_CameraControl{false};
	const float m_Speed{0.005f};
	const float m_RotSpeed{0.8f};
};

} // namespace ale