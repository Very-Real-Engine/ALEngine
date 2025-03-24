#pragma once

#include "Core/Base.h"
#include "Core/Timestep.h"
#include "Scene/CullTree.h"
#include <alglm/include/alglm.h>
#include <glm/glm.hpp>

namespace ale
{
/**
 * @class Camera
 * @brief 3D 그래픽스에서 사용되는 카메라 클래스.
 */
class Camera
{
  public:
	/// @brief Camera 생성자.
	Camera();

	/**
	 * @brief FOV(시야각), 화면 비율, 근평면 및 원평면을 설정하는 생성자.
	 * @param fov 시야각(Field of View).
	 * @param aspect 화면 비율(Aspect Ratio).
	 * @param near 근평면(Near Plane).
	 * @param far 원평면(Far Plane).
	 */
	Camera(float fov, float aspect, float near, float far);

	/// @brief 가상 소멸자.
	virtual ~Camera() = default;

	/**
	 * @brief 카메라의 화면 비율을 설정합니다.
	 * @param ratio 새로운 화면 비율(Aspect Ratio).
	 */
	void setAspectRatio(float ratio);

	/**
	 * @brief 카메라의 FOV(시야각)를 설정합니다.
	 * @param fov 새로운 시야각(FOV).
	 */
	void setFov(float fov);

	/**
	 * @brief 카메라의 근평면(Near Plane)을 설정합니다.
	 * @param _near 새로운 근평면 값.
	 */
	void setNear(float _near);

	/**
	 * @brief 카메라의 원평면(Far Plane)을 설정합니다.
	 * @param _far 새로운 원평면 값.
	 */
	void setFar(float _far);

	/**
	 * @brief 투영 행렬을 설정합니다.
	 * @param fov 시야각(FOV).
	 * @param aspect 화면 비율(Aspect Ratio).
	 * @param _near 근평면(Near Plane).
	 * @param _far 원평면(Far Plane).
	 */
	void setProjMatrix(float fov, float aspect, float _near, float _far);

	/**
	 * @brief 뷰 행렬을 설정합니다.
	 * @param pos 카메라 위치.
	 * @param dir 카메라 방향 벡터.
	 * @param up 상단 방향 벡터.
	 */
	void setViewMatrix(alglm::vec3 &pos, alglm::vec3 &dir, alglm::vec3 &up);

	/**
	 * @brief 뷰포트 크기를 설정합니다.
	 * @param width 뷰포트의 너비.
	 * @param height 뷰포트의 높이.
	 */
	void setViewportSize(uint32_t width, uint32_t height);

	/**
	 * @brief 카메라의 위치를 설정합니다.
	 * @param pos 새로운 위치 벡터.
	 */
	void setPosition(alglm::vec3 &pos);

	/**
	 * @brief 카메라의 회전값을 설정합니다.
	 * @param rot 새로운 회전 벡터.
	 */
	void setRotation(alglm::vec3 &rot);

	/**
	 * @brief 현재 카메라의 시야각(FOV)을 반환합니다.
	 * @return float 현재 FOV 값.
	 */
	float getFov() const;

	/**
	 * @brief 현재 카메라의 근평면(Near Plane) 값을 반환합니다.
	 * @return float 현재 근평면 값.
	 */
	float getNear() const;

	/**
	 * @brief 현재 카메라의 원평면(Far Plane) 값을 반환합니다.
	 * @return float 현재 원평면 값.
	 */
	float getFar() const;

	/**
	 * @brief 현재 카메라의 위치를 반환합니다.
	 * @return alglm::vec3& 카메라 위치 벡터.
	 */
	alglm::vec3 &getPosition();

	/**
	 * @brief 현재 카메라의 프러스텀(Frustum) 데이터를 반환합니다.
	 * @return const Frustum& 프러스텀 참조.
	 */
	const Frustum &getFrustum();

	/**
	 * @brief 현재 카메라의 투영 행렬을 반환합니다.
	 * @return const alglm::mat4& 투영 행렬.
	 */
	const alglm::mat4 &getProjection() const;

	/**
	 * @brief 현재 카메라의 뷰 행렬을 반환합니다.
	 * @return const alglm::mat4& 뷰 행렬.
	 */
	const alglm::mat4 &getView() const;

	/// @brief 카메라의 투영 행렬을 갱신합니다.
	void updateProjMatrix();

	/// @brief 카메라의 뷰 행렬을 갱신합니다.
	void updateViewMatrix();

  protected:
	alglm::mat4 m_projection = alglm::mat4(1.0f);
	alglm::mat4 m_view;

	alglm::vec3 m_cameraPos{0.0f, 0.0f, 10.0f};
	alglm::vec3 m_cameraFront{0.0f, 0.0f, -1.0f};
	alglm::vec3 m_cameraUp{0.0f, 1.0f, 0.0f};

	float m_CameraPitch{0.0f};
	float m_CameraYaw{0.0f};

	Frustum m_frustum;
	float m_fov = alglm::radians(45.0f);
	float m_near = 0.001f;
	float m_aspect = (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT;
	float m_far = 1000.0f;
};
} // namespace ale
