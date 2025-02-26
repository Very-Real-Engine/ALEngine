#pragma once

/**
 * @file SwapChain.h
 * @brief 스왑 체인 클래스
 */

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/VulkanContext.h"
#include "Renderer/VulkanUtil.h"

namespace ale
{
/**
 * @class SwapChain
 * @brief 스왑 체인 클래스
 */
class SwapChain
{
  public:
	/**
	 * @brief 스왑 체인 생성
	 * @param window GLFW 윈도우
	 * @return std::unique_ptr<SwapChain> 스왑 체인
	 */
	static std::unique_ptr<SwapChain> createSwapChain(GLFWwindow *window);

	~SwapChain() = default;

	/**
	 * @brief 스왑 체인 정리
	 */
	void cleanup();
	/**
	 * @brief 스왑 체인 재생성
	 */
	void recreateSwapChain();

	/**
	 * @brief 스왑 체인 반환
	 * @return VkSwapchainKHR 스왑 체인
	 */
	VkSwapchainKHR getSwapChain()
	{
		return swapChain;
	}
	/**
	 * @brief 스왑 체인 이미지 반환
	 * @return std::vector<VkImage> 스왑 체인 이미지
	 */
	std::vector<VkImage> &getSwapChainImages()
	{
		return swapChainImages;
	}
	/**
	 * @brief 스왑 체인 이미지 포맷 반환
	 * @return VkFormat 스왑 체인 이미지 포맷
	 */
	VkFormat getSwapChainImageFormat()
	{
		return swapChainImageFormat;
	}
	/**
	 * @brief 스왑 체인 확장 반환
	 * @return VkExtent2D 스왑 체인 확장
	 */
	VkExtent2D getSwapChainExtent()
	{
		return swapChainExtent;
	}
	/**
	 * @brief 스왑 체인 이미지 뷰 반환
	 * @return std::vector<VkImageView> 스왑 체인 이미지 뷰
	 */
	std::vector<VkImageView> &getSwapChainImageViews()
	{
		return swapChainImageViews;
	}

  private:
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;

	GLFWwindow *window;
	VkDevice device;
	VkPhysicalDevice physicalDevice;
	VkSurfaceKHR surface;

	/**
	 * @brief 스왑 체인 초기화
	 * @param window GLFW 윈도우
	 */
	void initSwapChain(GLFWwindow *window);
	/**
	 * @brief 스왑 체인 생성
	 */
	void createSwapChain();
	/**
	 * @brief 스왑 체인 이미지 뷰 생성
	 */
	void createImageViews();
	/**
	 * @brief 큐 패밀리 인덱스 찾기
	 * @param device 물리적 장치
	 * @return QueueFamilyIndices 큐 패밀리 인덱스
	 */
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	/**
	 * @brief 스왑 체인 지원 세부 정보 조회
	 * @param device 물리적 장치
	 * @return SwapChainSupportDetails 스왑 체인 지원 세부 정보
	 */
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	/**
	 * @brief 스왑 체인 표면 포맷 선택
	 * @param availableFormats 사용 가능한 표면 포맷
	 * @return VkSurfaceFormatKHR 스왑 체인 표면 포맷
	 */
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
	/**
	 * @brief 스왑 체인 표시 모드 선택
	 * @param availablePresentModes 사용 가능한 표시 모드
	 * @return VkPresentModeKHR 스왑 체인 표시 모드
	 */
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
	/**
	 * @brief 스왑 체인 확장 선택
	 * @param capabilities 스왑 체인 기능
	 * @return VkExtent2D 스왑 체인 확장
	 */
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
};

} // namespace ale
