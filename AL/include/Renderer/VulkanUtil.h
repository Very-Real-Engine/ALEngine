#pragma once

/**
 * @file VulkanUtil.h
 * @brief Vulkan 유틸리티 클래스
 */

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/VulkanContext.h"
#include <imgui/imgui.h>

namespace ale
{
/**
 * @class VulkanUtil
 * @brief Vulkan 유틸리티 클래스
 */
class VulkanUtil
{
  public:
	/**
	 * @brief 이미지 생성
	 *
	 * @param width 이미지 너비
	 * @param height 이미지 높이
	 * @param mipLevels 이미지 미프 레벨
	 * @param numSamples 이미지 샘플 수
	 * @param format 이미지 포맷
	 * @param tiling 이미지 타일링
	 * @param usage 이미지 사용 방법
	 * @param properties 이미지 속성
	 * @param image 이미지
	 * @param imageMemory 이미지 메모리
	 */
	static void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
							VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
							VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);
	/**
	 * @brief 메모리 타입 찾기
	 *
	 * @param typeFilter 메모리 타입 필터
	 * @param properties 메모리 속성
	 * @return uint32_t 메모리 타입
	 */
	static uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	/**
	 * @brief 이미지 뷰 생성
	 *
	 * @param image 이미지
	 * @param format 이미지 포맷
	 * @param aspectFlags 이미지 측면 플래그
	 * @param mipLevels 이미지 미프 레벨
	 * @return VkImageView 이미지 뷰
	 */
	static VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
									   uint32_t mipLevels);

	/**
	 * @brief 깊이 포맷 찾기
	 * @return VkFormat 깊이 포맷
	 */
	static VkFormat findDepthFormat();
	/**
	 * @brief 지원되는 포맷 찾기
	 * @param candidates 포맷 후보
	 * @param tiling 타일링
	 * @param features 포맷 기능 플래그
	 * @return VkFormat 지원되는 포맷
	 */
	static VkFormat findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
										VkFormatFeatureFlags features);
	/**
	 * @brief 파일 읽기
	 *
	 * @param filename 파일 이름
	 * @return std::vector<char> 파일 데이터
	 */
	static std::vector<char> readFile(const std::string &filename);
	/**
	 * @brief 이미지 메모리 배리어 삽입
	 *
	 * @param cmdbuffer 커맨드 버퍼
	 * @param image 이미지
	 * @param srcAccessMask 소스 접근 마스크
	 * @param dstAccessMask 대상 접근 마스크
	 * @param oldImageLayout 이전 이미지 레이아웃
	 * @param newImageLayout 새로운 이미지 레이아웃
	 * @param srcStageMask 소스 스테이지 마스크
	 * @param dstStageMask 대상 스테이지 마스크
	 * @param subresourceRange 서브리소스 범위
	 */
	static void insertImageMemoryBarrier(VkCommandBuffer cmdbuffer, VkImage image, VkAccessFlags srcAccessMask,
										 VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout,
										 VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask,
										 VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange);

	/**
	 * @brief 싱글 타임 커맨드 버퍼 시작
	 *
	 * @param device 디바이스
	 * @param commandpool 커맨드 풀
	 * @return VkCommandBuffer 싱글 타임 커맨드 버퍼
	 */
	static VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandpool);
	/**
	 * @brief 싱글 타임 커맨드 버퍼 종료
	 *
	 * @param device 디바이스
	 * @param queue 큐
	 * @param commandPool 커맨드 풀
	 * @param commandBuffer 커맨드 버퍼
	 */
	static void endSingleTimeCommands(VkDevice device, VkQueue queue, VkCommandPool commandPool,
									  VkCommandBuffer commandBuffer);

	/**
	 * @brief 샘플러 생성
	 * @return VkSampler 샘플러
	 */
	static VkSampler createSampler();
	/**
	 * @brief 아이콘 텍스쳐 생성
	 * @param device 디바이스
	 * @param descriptorpool 디스크립터 풀
	 * @param imageView 이미지 뷰
	 * @param sampler 샘플러
	 * @return ImTextureID 아이콘 텍스쳐
	 */
	static ImTextureID createIconTexture(VkDevice device, VkDescriptorPool descriptorpool, VkImageView imageView,
										 VkSampler sampler);

	/**
	 * @brief 큐브 맵 이미지 생성
	 *
	 * @param width 이미지 너비
	 * @param height 이미지 높이
	 * @param mipLevels 이미지 미프 레벨
	 * @param numSamples 이미지 샘플 수
	 * @param format 이미지 포맷
	 * @param tiling 이미지 타일링
	 * @param usage 이미지 사용 방법
	 * @param properties 이미지 속성
	 * @param image 이미지
	 * @param imageMemory 이미지 메모리
	 */
	static void createCubeMapImage(uint32_t width, uint32_t height, uint32_t mipLevels,
								   VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling,
								   VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage &image,
								   VkDeviceMemory &imageMemory);
	/**
	 * @brief 큐브 맵 이미지 뷰 생성
	 *
	 * @param image 이미지
	 * @param format 이미지 포맷
	 * @param aspectFlags 이미지 측면 플래그
	 * @param mipLevels 이미지 미프 레벨
	 * @return VkImageView 큐브 맵 이미지 뷰
	 */
	static VkImageView createCubeMapImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
											  uint32_t mipLevels);

	/**
	 * @brief 아이콘 디스크립터 세트 레이아웃 생성
	 *
	 * @param device 디바이스
	 * @param descriptorPool 디스크립터 풀
	 * @return VkDescriptorSetLayout 아이콘 디스크립터 세트 레이아웃
	 */
	static VkDescriptorSetLayout createIconDescriptorSetLayout(VkDevice device, VkDescriptorPool descriptorPool);
	/**
	 * @brief 아이콘 디스크립터 세트 생성
	 *
	 * @param device 디바이스
	 * @param descriptorPool 디스크립터 풀
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 * @param imageView 이미지 뷰
	 * @param sampler 샘플러
	 * @return VkDescriptorSet 아이콘 디스크립터 세트
	 */
	static VkDescriptorSet createIconDescriptorSet(VkDevice device, VkDescriptorPool descriptorPool,
												   VkDescriptorSetLayout descriptorSetLayout, VkImageView imageView,
												   VkSampler sampler);
};
} // namespace ale
