#pragma once

#include "Renderer/Camera.h"

namespace ale
{
/**
 * @class SceneCamera
 * @brief 씬(Scene) 내에서 사용되는 카메라 클래스.
 *        Camera 클래스를 상속하며, 향후 다양한 카메라 유형(정사영, 원근 등)으로 확장 가능.
 */
class SceneCamera : public Camera
{
  public:
	/// @brief SceneCamera 생성자.
	SceneCamera() = default;

	/// @brief  SceneCamera 소멸자.
	virtual ~SceneCamera() = default;

	/**
	 * @brief 씬 카메라의 위치와 회전을 업데이트합니다.
	 * @param pos 새로운 위치 벡터.
	 * @param rot 새로운 회전 벡터.
	 */
	void updateSceneCamera(alglm::vec3 &pos, alglm::vec3 &rot);
};
} // namespace ale
