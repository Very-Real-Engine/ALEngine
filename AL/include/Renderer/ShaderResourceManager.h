#pragma once

/**
 * @file ShaderResourceManager.h
 * @brief 쉐이더 리소스 매니저 클래스
 */

#include "Core/Base.h"
#include "Renderer/Buffer.h"
#include "Renderer/Common.h"
#include "Renderer/Model.h"
#include "Renderer/VulkanContext.h"

namespace ale
{
/**
 * @class ShaderResourceManager
 * @brief 쉐이더 리소스 매니저 클래스
 */
class ShaderResourceManager
{
  public:
	/**
	 * @brief 기하 패스 쉐이더 리소스 매니저 생성
	 * @param model 모델
	 * @return std::unique_ptr<ShaderResourceManager> 기하 패스 쉐이더 리소스 매니저
	 */
	static std::unique_ptr<ShaderResourceManager> createGeometryPassShaderResourceManager(Model *model);
	/**
	 * @brief 조명 패스 쉐이더 리소스 매니저 생성
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 * @param positionImageView 위치 이미지 뷰
	 * @param normalImageView 노말 이미지 뷰
	 * @param albedoImageView 알베도 이미지 뷰
	 * @param pbrImageView PBR 이미지 뷰
	 * @param shadowMapImageViews 그림자 맵 이미지 뷰
	 * @param shadowMapSamplers 그림자 맵 샘플러
	 * @param shadowCubeMapImageViews 그림자 큐브 맵 이미지 뷰
	 * @param shadowCubeMapSampler 그림자 큐브 맵 샘플러
	 * @param backgroundImageView 배경 이미지 뷰
	 * @param backgroundSampler 배경 샘플러
	 * @return std::unique_ptr<ShaderResourceManager> 조명 패스 쉐이더 리소스 매니저
	 */
	static std::unique_ptr<ShaderResourceManager> createLightingPassShaderResourceManager(
		VkDescriptorSetLayout descriptorSetLayout, VkImageView positionImageView, VkImageView normalImageView,
		VkImageView albedoImageView, VkImageView pbrImageView, std::vector<VkImageView> &shadowMapImageViews,
		VkSampler shadowMapSamplers, std::vector<VkImageView> &shadowCubeMapImageViews, VkSampler shadowCubeMapSampler,
		VkImageView backgroundImageView, VkSampler backgroundSampler);
	/**
	 * @brief 그림자 맵 쉐이더 리소스 매니저 생성
	 * @return std::unique_ptr<ShaderResourceManager> 그림자 맵 쉐이더 리소스 매니저
	 */
	static std::unique_ptr<ShaderResourceManager> createShadowMapShaderResourceManager();
	/**
	 * @brief 그림자 큐브 맵 쉐이더 리소스 매니저 생성
	 * @return std::unique_ptr<ShaderResourceManager> 그림자 큐브 맵 쉐이더 리소스 매니저
	 */
	static std::unique_ptr<ShaderResourceManager> createShadowCubeMapShaderResourceManager();
	/**
	 * @brief 뷰포트 쉐이더 리소스 매니저 생성
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 * @param viewPortImageView 뷰포트 이미지 뷰
	 * @param viewPortSampler 뷰포트 샘플러
	 * @return std::unique_ptr<ShaderResourceManager> 뷰포트 쉐이더 리소스 매니저
	 */
	static std::unique_ptr<ShaderResourceManager> createViewPortShaderResourceManager(
		VkDescriptorSetLayout descriptorSetLayout, VkImageView viewPortImageView, VkSampler viewPortSampler);
	/**
	 * @brief 구면 맵 쉐이더 리소스 매니저 생성
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 * @param sphericalMapImageView 구면 맵 이미지 뷰
	 * @param sphericalMapSampler 구면 맵 샘플러
	 * @return std::unique_ptr<ShaderResourceManager> 구면 맵 쉐이더 리소스 매니저
	 */
	static std::unique_ptr<ShaderResourceManager> createSphericalMapShaderResourceManager(
		VkDescriptorSetLayout descriptorSetLayout, VkImageView sphericalMapImageView, VkSampler sphericalMapSampler);
	/**
	 * @brief 배경 쉐이더 리소스 매니저 생성
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 * @param skyboxImageView 스카이박스 이미지 뷰
	 * @param skyboxSampler 스카이박스 샘플러
	 * @return std::unique_ptr<ShaderResourceManager> 배경 쉐이더 리소스 매니저
	 */
	static std::unique_ptr<ShaderResourceManager> createBackgroundShaderResourceManager(
		VkDescriptorSetLayout descriptorSetLayout, VkImageView skyboxImageView, VkSampler skyboxSampler);
	/**
	 * @brief 콜라이더 쉐이더 리소스 매니저 생성
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 * @return std::unique_ptr<ShaderResourceManager> 콜라이더 쉐이더 리소스 매니저
	 */
	static std::unique_ptr<ShaderResourceManager> createColliderShaderResourceManager(
		VkDescriptorSetLayout descriptorSetLayout);

	static std::unique_ptr<ShaderResourceManager> createShadowMapShaderResourceManagerSSBO(
		VkDescriptorSetLayout descriptorSetLayout, std::vector<std::shared_ptr<StorageBuffer>> &ssbo);
	static std::unique_ptr<ShaderResourceManager> createShadowCubeMapShaderResourceManagerSSBO(
		VkDescriptorSetLayout descriptorSetLayout, std::vector<std::shared_ptr<StorageBuffer>> &ssbo);

	void changeShadowMapSSBO(std::vector<std::shared_ptr<StorageBuffer>> &ssbo);
	void changeShadowCubeMapSSBO(std::vector<std::shared_ptr<StorageBuffer>> &ssbo);

	/**
	 * @brief 조명 패스 쉐이더 리소스 매니저 초기화
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 * @param positionImageView 위치 이미지 뷰
	 * @param normalImageView 노말 이미지 뷰
	 * @param albedoImageView 알베도 이미지 뷰
	 * @param pbrImageView PBR 이미지 뷰
	 * @param shadowMapImageViews 그림자 맵 이미지 뷰
	 * @param shadowMapSamplers 그림자 맵 샘플러
	 * @param shadowCubeMapImageViews 그림자 큐브 맵 이미지 뷰
	 * @param shadowCubeMapSampler 그림자 큐브 맵 샘플러
	 * @param backgroundImageView 배경 이미지 뷰
	 * @param backgroundSampler 배경 샘플러
	 */
	void initLightingPassShaderResourceManager(VkDescriptorSetLayout descriptorSetLayout, VkImageView positionImageView,
											   VkImageView normalImageView, VkImageView albedoImageView,
											   VkImageView pbrImageView, std::vector<VkImageView> &shadowMapImageViews,
											   VkSampler shadowMapSamplers,
											   std::vector<VkImageView> &shadowCubeMapImageViews,
											   VkSampler shadowCubeMapSampler, VkImageView backgroundImageView,
											   VkSampler backgroundSampler);

	/**
	 * @brief 배경 쉐이더 리소스 매니저 초기화
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 * @param skyboxImageView 스카이박스 이미지 뷰
	 * @param skyboxSampler 스카이박스 샘플러
	 */
	void initBackgroundShaderResourceManager(VkDescriptorSetLayout descriptorSetLayout, VkImageView skyboxImageView,
											 VkSampler skyboxSampler);
	/**
	 * @brief 뷰포트 쉐이더 리소스 매니저 초기화
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 * @param viewPortImageView 뷰포트 이미지 뷰
	 * @param viewPortSampler 뷰포트 샘플러
	 */
	void initViewPortShaderResourceManager(VkDescriptorSetLayout descriptorSetLayout, VkImageView viewPortImageView,
										   VkSampler viewPortSampler);

	/**
	 * @brief 구면 맵 쉐이더 리소스 매니저 초기화
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 * @param sphericalMapImageView 구면 맵 이미지 뷰
	 * @param sphericalMapSampler 구면 맵 샘플러
	 */
	void initSphericalMapShaderResourceManager(VkDescriptorSetLayout descriptorSetLayout,
											   VkImageView sphericalMapImageView, VkSampler sphericalMapSampler);

	/**
	 * @brief 콜라이더 쉐이더 리소스 매니저 초기화
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 */
	void initColliderShaderResourceManager(VkDescriptorSetLayout descriptorSetLayout);
	~ShaderResourceManager() = default;

	/**
	 * @brief 정리
	 */
	void cleanup();

	/**
	 * @brief 유니폼 버퍼 반환
	 * @return std::vector<std::shared_ptr<UniformBuffer>> & 유니폼 버퍼
	 */
	std::vector<std::shared_ptr<UniformBuffer>> &getUniformBuffers()
	{
		return m_uniformBuffers;
	}
	/**
	 * @brief 레이어 인덱스 유니폼 버퍼 반환
	 * @return std::vector<std::shared_ptr<UniformBuffer>> & 레이어 인덱스 유니폼 버퍼
	 */
	std::vector<std::shared_ptr<UniformBuffer>> &getLayerIndexUniformBuffers()
	{
		return m_layerIndexUniformBuffers;
	}
	/**
	 * @brief 버텍스 유니폼 버퍼 반환
	 * @return std::vector<std::shared_ptr<UniformBuffer>> & 버텍스 유니폼 버퍼
	 */
	std::vector<std::shared_ptr<UniformBuffer>> &getVertexUniformBuffers()
	{
		return m_vertexUniformBuffers;
	}
	/**
	 * @brief 프래그먼트 유니폼 버퍼 반환
	 * @return std::vector<std::shared_ptr<UniformBuffer>> & 프래그먼트 유니폼 버퍼
	 */
	std::vector<std::shared_ptr<UniformBuffer>> &getFragmentUniformBuffers()
	{
		return m_fragmentUniformBuffers;
	}
	/**
	 * @brief 디스크립터 세트 반환
	 * @return std::vector<VkDescriptorSet> & 디스크립터 세트
	 */
	std::vector<VkDescriptorSet> &getDescriptorSets()
	{
		return descriptorSets;
	}
	/**
	 * @brief 디스크립터 세트 업데이트
	 * @param model 모델
	 * @param materials 재질
	 */
	void updateDescriptorSets(Model *model, std::vector<std::shared_ptr<Material>> materials);

  private:
	std::vector<std::shared_ptr<UniformBuffer>> m_uniformBuffers = {};
	std::vector<std::shared_ptr<UniformBuffer>> m_layerIndexUniformBuffers = {};
	std::vector<std::shared_ptr<UniformBuffer>> m_vertexUniformBuffers = {};
	std::vector<std::shared_ptr<UniformBuffer>> m_fragmentUniformBuffers = {};

	std::vector<VkDescriptorSet> descriptorSets = {};

	/**
	 * @brief 기하 패스 쉐이더 리소스 매니저 초기화
	 * @param model 모델
	 */
	void initGeometryPassShaderResourceManager(Model *model);
	/**
	 * @brief 기하 패스 유니폼 버퍼 생성
	 * @param model 모델
	 */
	void createGeometryPassUniformBuffers(Model *model);
	/**
	 * @brief 기하 패스 디스크립터 세트 생성
	 * @param model 모델
	 */
	void createGeometryPassDescriptorSets(Model *model);

	/**
	 * @brief 조명 패스 유니폼 버퍼 생성
	 */
	void createLightingPassUniformBuffers();
	/**
	 * @brief 조명 패스 디스크립터 세트 생성
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 * @param positionImageView 위치 이미지 뷰
	 * @param normalImageView 노말 이미지 뷰
	 * @param albedoImageView 알베도 이미지 뷰
	 * @param pbrImageView PBR 이미지 뷰
	 * @param shadowMapImageViews 그림자 맵 이미지 뷰
	 * @param shadowMapSamplers 그림자 맵 샘플러
	 * @param shadowCubeMapImageViews 그림자 큐브 맵 이미지 뷰
	 * @param shadowCubeMapSampler 그림자 큐브 맵 샘플러
	 * @param backgroundImageView 배경 이미지 뷰
	 * @param backgroundSampler 배경 샘플러
	 */
	void createLightingPassDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkImageView positionImageView,
										  VkImageView normalImageView, VkImageView albedoImageView,

										  VkImageView pbrImageView, std::vector<VkImageView> &shadowMapImageViews,
										  VkSampler shadowMapSamplers,
										  std::vector<VkImageView> &shadowCubeMapImageViews,
										  VkSampler shadowCubeMapSampler, VkImageView backgroundImageView,
										  VkSampler backgroundSampler);

	/**
	 * @brief 그림자 맵 쉐이더 리소스 매니저 초기화
	 */
	void initShadowMapShaderResourceManager();
	/**
	 * @brief 그림자 맵 유니폼 버퍼 생성
	 */
	void createShadowMapUniformBuffers();
	/**
	 * @brief 그림자 맵 디스크립터 세트 생성
	 */
	void createShadowMapDescriptorSets();

	/**
	 * @brief 그림자 큐브 맵 쉐이더 리소스 매니저 초기화
	 */
	void initShadowCubeMapShaderResourceManager();
	/**
	 * @brief 그림자 큐브 맵 유니폼 버퍼 생성
	 */
	void createShadowCubeMapUniformBuffers();
	/**
	 * @brief 그림자 큐브 맵 디스크립터 세트 생성
	 */
	void createShadowCubeMapDescriptorSets();

	/**
	 * @brief 뷰포트 디스크립터 세트 생성
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 * @param viewPortImageView 뷰포트 이미지 뷰
	 * @param viewPortSampler 뷰포트 샘플러
	 */
	void createViewPortDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkImageView viewPortImageView,
									  VkSampler viewPortSampler);

	/**
	 * @brief 구면 맵 유니폼 버퍼 생성
	 */
	void createSphericalMapUniformBuffers();
	/**
	 * @brief 구면 맵 디스크립터 세트 생성
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 * @param sphericalMapImageView 구면 맵 이미지 뷰
	 * @param sphericalMapSampler 구면 맵 샘플러
	 */
	void createSphericalMapDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkImageView sphericalMapImageView,
										  VkSampler sphericalMapSampler);

	/**
	 * @brief 배경 유니폼 버퍼 생성
	 */
	void createBackgroundUniformBuffers();
	/**
	 * @brief 배경 디스크립터 세트 생성
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 * @param skyboxImageView 스카이박스 이미지 뷰
	 * @param skyboxSampler 스카이박스 샘플러
	 */
	void createBackgroundDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkImageView skyboxImageView,
										VkSampler skyboxSampler);

	/**
	 * @brief 콜라이더 유니폼 버퍼 생성
	 */
	void createColliderUniformBuffers();
	/**
	 * @brief 콜라이더 디스크립터 세트 생성
	 * @param descriptorSetLayout 디스크립터 세트 레이아웃
	 */
	void createColliderDescriptorSets(VkDescriptorSetLayout descriptorSetLayout);

	void initShadowMapShaderResourceManagerSSBO(VkDescriptorSetLayout descriptorSetLayout,
												std::vector<std::shared_ptr<StorageBuffer>> &ssbo);
	void createShadowMapUniformBuffersSSBO();
	void createShadowMapDescriptorSetsSSBO(VkDescriptorSetLayout descriptorSetLayout,
										   std::vector<std::shared_ptr<StorageBuffer>> &ssbo);

	void initShadowCubeMapShaderResourceManagerSSBO(VkDescriptorSetLayout descriptorSetLayout,
													std::vector<std::shared_ptr<StorageBuffer>> &ssbo);
	void createShadowCubeMapUniformBuffersSSBO();
	void createShadowCubeMapDescriptorSetsSSBO(VkDescriptorSetLayout descriptorSetLayout,
											   std::vector<std::shared_ptr<StorageBuffer>> &ssbo);
};
} // namespace ale
