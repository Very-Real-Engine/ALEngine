#pragma once

/**
 * @file SyncObjects.h
 * @brief 동기화 객체 클래스
 */

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/VulkanContext.h"

namespace ale
{
/**
 * @class SyncObjects
 * @brief 동기화 객체 클래스
 */
class SyncObjects
{
  public:
	/**
	 * @brief 동기화 객체 생성
	 * @return std::unique_ptr<SyncObjects> 동기화 객체
	 */
	static std::unique_ptr<SyncObjects> createSyncObjects();
	/**
	 * @brief 동기화 객체 정리
	 */
	~SyncObjects() = default;
	/**
	 * @brief 동기화 객체 정리
	 */
	void cleanup();
	/**
	 * @brief 이미지 사용 가능 세마포어 반환
	 * @return std::vector<VkSemaphore> 이미지 사용 가능 세마포어
	 */
	std::vector<VkSemaphore> &getImageAvailableSemaphores()
	{
		return imageAvailableSemaphores;
	}
	/**
	 * @brief 렌더 완료 세마포어 반환
	 * @return std::vector<VkSemaphore> 렌더 완료 세마포어
	 */
	std::vector<VkSemaphore> &getRenderFinishedSemaphores()
	{
		return renderFinishedSemaphores;
	}
	/**
	 * @brief 렌더 완료 세마포어 반환
	 * @return std::vector<VkFence> 렌더 완료 세마포어
	 */
	std::vector<VkFence> &getInFlightFences()
	{
		return inFlightFences;
	}

  private:
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	/**
	 * @brief 동기화 객체 초기화
	 */
	void initSyncObjects();
};
} // namespace ale
