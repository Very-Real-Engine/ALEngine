#pragma once

/**
 * @file CommandBuffers.h
 * @brief 커맨드 버퍼 클래스
 */

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/VulkanContext.h"

namespace ale
{
/**
 * @brief 커맨드 버퍼 클래스
 */
class CommandBuffers
{
  public:
	/**
	 * @brief 커맨드 버퍼 생성
	 * @return 커맨드 버퍼
	 */
	static std::unique_ptr<CommandBuffers> createCommandBuffers();

	/**
	 * @brief 커맨드 버퍼 소멸자
	 */
	~CommandBuffers() = default;
	/**
	 * @brief 커맨드 버퍼 정리
	 */
	void cleanup();
	/**
	 * @brief 커맨드 버퍼 반환
	 * @return 커맨드 버퍼
	 */
	std::vector<VkCommandBuffer> &getCommandBuffers()
	{
		return commandBuffers;
	}

  private:
	std::vector<VkCommandBuffer> commandBuffers;
	/**
	 * @brief 커맨드 버퍼 초기화
	 */
	void initCommandBuffers();
};

} // namespace ale
