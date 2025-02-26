#pragma once

/**
 * @file Pipeline.h
 * @brief 파이프라인
 */

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/VulkanContext.h"
#include "Renderer/VulkanUtil.h"

namespace ale
{
/**
 * @class Pipeline
 * @brief 파이프라인
 */
class Pipeline
{
  public:
	/**
	 * @brief 기하 파이프라인 생성
	 * @param renderPass 렌더 패스
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 * @return std::unique_ptr<Pipeline> 기하 파이프라인
	 */
	static std::unique_ptr<Pipeline> createGeometryPassPipeline(VkRenderPass renderPass,
																VkDescriptorSetLayout descriptorSetLayout);
	/**
	 * @brief 라이팅 파이프라인 생성
	 * @param renderPass 렌더 패스
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 * @return std::unique_ptr<Pipeline> 라이팅 파이프라인
	 */
	static std::unique_ptr<Pipeline> createLightingPassPipeline(VkRenderPass renderPass,
																VkDescriptorSetLayout descriptorSetLayout);
	/**
	 * @brief 그림자 맵 파이프라인 생성
	 * @param renderPass 렌더 패스
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 * @return std::unique_ptr<Pipeline> 그림자 맵 파이프라인
	 */
	static std::unique_ptr<Pipeline> createShadowMapPipeline(VkRenderPass renderPass,
															 VkDescriptorSetLayout descriptorSetLayout);
	/**
	 * @brief 그림자 큐브 맵 파이프라인 생성
	 * @param renderPass 렌더 패스
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 * @return std::unique_ptr<Pipeline> 그림자 큐브 맵 파이프라인
	 */
	static std::unique_ptr<Pipeline> createShadowCubeMapPipeline(VkRenderPass renderPass,
																 VkDescriptorSetLayout descriptorSetLayout);
	/**
	 * @brief 구면 맵 파이프라인 생성
	 * @param renderPass 렌더 패스
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 * @return std::unique_ptr<Pipeline> 구면 맵 파이프라인
	 */
	static std::unique_ptr<Pipeline> createSphericalMapPipeline(VkRenderPass renderPass,
																VkDescriptorSetLayout descriptorSetLayout);
	/**
	 * @brief 배경 파이프라인 생성
	 * @param renderPass 렌더 패스
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 * @return std::unique_ptr<Pipeline> 배경 파이프라인
	 */
	static std::unique_ptr<Pipeline> createBackgroundPipeline(VkRenderPass renderPass,
															  VkDescriptorSetLayout descriptorSetLayout);
	/**
	 * @brief 콜라이더 파이프라인 생성
	 * @param renderPass 렌더 패스
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 * @return std::unique_ptr<Pipeline> 콜라이더 파이프라인
	 */
	static std::unique_ptr<Pipeline> createColliderPipeline(VkRenderPass renderPass,
															VkDescriptorSetLayout descriptorSetLayout);

	/**
	 * @brief 기하 파이프라인 초기화
	 * @param renderPass 렌더 패스
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 */
	void initGeometryPassPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
	/**
	 * @brief 라이팅 파이프라인 초기화
	 * @param renderPass 렌더 패스
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 */
	void initLightingPassPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
	/**
	 * @brief 그림자 맵 파이프라인 초기화
	 * @param renderPass 렌더 패스
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 */
	void initShadowMapPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
	/**
	 * @brief 그림자 큐브 맵 파이프라인 초기화
	 * @param renderPass 렌더 패스
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 */
	void initShadowCubeMapPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
	/**
	 * @brief 구면 맵 파이프라인 초기화
	 * @param renderPass 렌더 패스
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 */
	void initSphericalMapPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
	/**
	 * @brief 배경 파이프라인 초기화
	 * @param renderPass 렌더 패스
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 */
	void initBackgroundPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
	/**
	 * @brief 콜라이더 파이프라인 초기화
	 * @param renderPass 렌더 패스
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 */
	void initColliderPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);

	~Pipeline() = default;

	/**
	 * @brief 파이프라인 정리
	 */
	void cleanup();

	/**
	 * @brief 파이프라인 반환
	 * @return VkPipeline 파이프라인
	 */
	VkPipeline getPipeline()
	{
		return pipeline;
	}
	/**
	 * @brief 파이프라인 레이아웃 반환
	 * @return VkPipelineLayout 파이프라인 레이아웃
	 */
	VkPipelineLayout getPipelineLayout()
	{
		return pipelineLayout;
	}

  private:
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

	/**
	 * @brief 쉐이더 모듈 생성
	 * @param code 쉐이더 코드
	 * @return VkShaderModule 쉐이더 모듈
	 */
	VkShaderModule createShaderModule(const std::vector<char> &code);
};
} // namespace ale
