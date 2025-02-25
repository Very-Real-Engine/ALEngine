#pragma once

/**
 * @file VulkanContext.h
 * @brief Vulkan 컨텍스트 클래스
 */

#include "Core/Base.h"
#include "Renderer/Common.h"

namespace ale
{
/**
 * @class VulkanContext
 * @brief Vulkan 컨텍스트 클래스
 */
class VulkanContext
{
  public:
	/**
	 * @brief Vulkan 컨텍스트 반환
	 * @return VulkanContext & Vulkan 컨텍스트
	 */
	static VulkanContext &getContext();
	~VulkanContext()
	{
	}
	/**
	 * @brief Vulkan 컨텍스트 정리
	 */
	void cleanup();
	/**
	 * @brief Vulkan 컨텍스트 초기화
	 * @param window GLFW 윈도우
	 */
	void initContext(GLFWwindow *window);
	/**
	 * @brief Vulkan 컨텍스트 표면 생성
	 * @param window GLFW 윈도우
	 */
	void createSurface(GLFWwindow *window);

	/**
	 * @brief Vulkan 인스턴스 반환
	 * @return VkInstance Vulkan 인스턴스
	 */
	VkInstance getInstance()
	{
		return instance;
	}
	/**
	 * @brief Vulkan 디버그 유틸리티 메시지 반환
	 * @return VkDebugUtilsMessengerEXT Vulkan 디버그 유틸리티 메시지
	 */
	VkDebugUtilsMessengerEXT getDebugMessenger()
	{
		return debugMessenger;
	}
	/**
	 * @brief Vulkan 물리적 장치 반환
	 * @return VkPhysicalDevice Vulkan 물리적 장치
	 */
	VkPhysicalDevice getPhysicalDevice()
	{
		return physicalDevice;
	}
	/**
	 * @brief Vulkan 디바이스 반환
	 * @return VkDevice Vulkan 디바이스
	 */
	VkDevice getDevice()
	{
		return device;
	}
	/**
	 * @brief Vulkan 그래픽스 큐 반환
	 * @return VkQueue Vulkan 그래픽스 큐
	 */
	VkQueue getGraphicsQueue()
	{
		return graphicsQueue;
	}
	/**
	 * @brief Vulkan 프리젠테이션 큐 반환
	 * @return VkQueue Vulkan 프리젠테이션 큐
	 */
	VkQueue getPresentQueue()
	{
		return presentQueue;
	}
	/**
	 * @brief Vulkan 커맨드 풀 반환
	 * @return VkCommandPool Vulkan 커맨드 풀
	 */
	VkCommandPool getCommandPool()
	{
		return commandPool;
	}
	/**
	 * @brief Vulkan 표면 반환
	 * @return VkSurfaceKHR Vulkan 표면
	 */
	VkSurfaceKHR getSurface()
	{
		return surface;
	}
	/**
	 * @brief Vulkan 최대 사용 가능 샘플 수 반환
	 * @return VkSampleCountFlagBits Vulkan 최대 사용 가능 샘플 수
	 */
	VkSampleCountFlagBits getMsaaSamples()
	{
		return msaaSamples;
	}
	/**
	 * @brief Vulkan 디스크립터 풀 반환
	 * @return VkDescriptorPool Vulkan 디스크립터 풀
	 */
	VkDescriptorPool getDescriptorPool()
	{
		return descriptorPool;
	}
	/**
	 * @brief Vulkan 큐 패밀리 인덱스 반환
	 * @return uint32_t Vulkan 큐 패밀리 인덱스
	 */
	uint32_t getQueueFamily();
	/**
	 * @brief Vulkan 기본 패스 디스크립터 세트 레이아웃 반환
	 * @return VkDescriptorSetLayout Vulkan 기본 패스 디스크립터 세트 레이아웃
	 */
	VkDescriptorSetLayout getGeometryPassDescriptorSetLayout()
	{
		return geometryPassDescriptorSetLayout;
	}
	/**
	 * @brief Vulkan 그림자 맵 디스크립터 세트 레이아웃 반환
	 * @return VkDescriptorSetLayout Vulkan 그림자 맵 디스크립터 세트 레이아웃
	 */
	VkDescriptorSetLayout getShadowMapDescriptorSetLayout()
	{
		return shadowMapDescriptorSetLayout;
	}
	/**
	 * @brief Vulkan 그림자 큐브 맵 디스크립터 세트 레이아웃 반환
	 * @return VkDescriptorSetLayout Vulkan 그림자 큐브 맵 디스크립터 세트 레이아웃
	 */
	VkDescriptorSetLayout getShadowCubeMapDescriptorSetLayout()
	{
		return shadowCubeMapDescriptorSetLayout;
	}
	/**
	 * @brief Vulkan 콜라이더 디스크립터 세트 레이아웃 반환
	 * @return VkDescriptorSetLayout Vulkan 콜라이더 디스크립터 세트 레이아웃
	 */
	VkDescriptorSetLayout getColliderDescriptorSetLayout()
	{
		return colliderDescriptorSetLayout;
	}

	/**
	 * @brief Vulkan 기본 패스 디스크립터 세트 레이아웃 설정
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 */
	void setGeometryPassDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout)
	{
		geometryPassDescriptorSetLayout = descriptorSetLayout;
	}
	/**
	 * @brief Vulkan 그림자 맵 디스크립터 세트 레이아웃 설정
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 */
	void setShadowMapDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout)
	{
		shadowMapDescriptorSetLayout = descriptorSetLayout;
	}
	/**
	 * @brief Vulkan 그림자 큐브 맵 디스크립터 세트 레이아웃 설정
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 */
	void setShadowCubeMapDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout)
	{
		shadowCubeMapDescriptorSetLayout = descriptorSetLayout;
	}
	/**
	 * @brief Vulkan 콜라이더 디스크립터 세트 레이아웃 설정
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 */
	void setColliderDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout)
	{
		colliderDescriptorSetLayout = descriptorSetLayout;
	}

  private:
	VulkanContext()
	{
	}
	VulkanContext(VulkanContext const &) = delete;
	void operator=(VulkanContext const &) = delete;

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkDevice device;
	VkCommandPool commandPool;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout geometryPassDescriptorSetLayout;
	VkDescriptorSetLayout shadowMapDescriptorSetLayout;
	VkDescriptorSetLayout shadowCubeMapDescriptorSetLayout;
	VkDescriptorSetLayout colliderDescriptorSetLayout;

	/**
	 * @brief Vulkan 인스턴스 생성
	 */
	void createInstance();
	/**
	 * @brief Vulkan 유효성 검사 레이어 지원 여부 확인
	 * @return bool Vulkan 유효성 검사 레이어 지원 여부
	 */
	bool checkValidationLayerSupport();
	/**
	 * @brief Vulkan 필요한 확장 프로그램 반환
	 * @return std::vector<const char *> Vulkan 필요한 확장 프로그램
	 */
	std::vector<const char *> getRequiredExtensions();
	/**
	 * @brief Vulkan 디버그 유틸리티 메시지 설정
	 */
	void setupDebugMessenger();
	/**
	 * @brief Vulkan 디버그 유틸리티 메시지 생성 정보 채우기
	 * @param createInfo 디버그 유틸리티 메시지 생성 정보
	 */
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo);
	/**
	 * @brief Vulkan 물리적 장치 선택
	 */
	void pickPhysicalDevice();
	/**
	 * @brief Vulkan 논리적 장치 생성
	 */
	void createLogicalDevice();
	/**
	 * @brief Vulkan 커맨드 풀 생성
	 */
	void createCommandPool();
	/**
	 * @brief Vulkan 물리적 장치 적합성 확인
	 * @param device Vulkan 물리적 장치
	 * @return bool Vulkan 물리적 장치 적합성
	 */
	bool isDeviceSuitable(VkPhysicalDevice device);
	/**
	 * @brief Vulkan 스왑 체인 지원 세부 정보 조회
	 * @param device Vulkan 물리적 장치
	 * @return SwapChainSupportDetails Vulkan 스왑 체인 지원 세부 정보
	 */
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	/**
	 * @brief Vulkan 최대 사용 가능 샘플 수 반환
	 * @return VkSampleCountFlagBits Vulkan 최대 사용 가능 샘플 수
	 */
	VkSampleCountFlagBits getMaxUsableSampleCount();
	/**
	 * @brief Vulkan 장치 확장 지원 여부 확인
	 * @param device Vulkan 물리적 장치
	 * @return bool Vulkan 장치 확장 지원 여부
	 */
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	/**
	 * @brief Vulkan 큐 패밀리 인덱스 찾기
	 * @param device Vulkan 물리적 장치
	 * @return QueueFamilyIndices Vulkan 큐 패밀리 인덱스
	 */
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	/**
	 * @brief Vulkan 디스크립터 풀 생성
	 */
	void createDescriptorPool();
	/**
	 * @brief Vulkan 디버그 콜백
	 * @param messageSeverity 메시지 심각도
	 * @param messageType 메시지 유형
	 * @param pCallbackData 콜백 데이터
	 * @param pUserData 사용자 데이터
	 * @return VkBool32 Vulkan 디버그 콜백
	 */
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
														VkDebugUtilsMessageTypeFlagsEXT messageType,
														const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
														void *pUserData);
};
} // namespace ale
