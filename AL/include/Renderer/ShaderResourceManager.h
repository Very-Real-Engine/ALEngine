#ifndef SHADERRESOURCEMANAGER_H
#define SHADERRESOURCEMANAGER_H

#include "Core/Base.h"
#include "Renderer/Buffer.h"
#include "Renderer/Common.h"
#include "Renderer/Model.h"
#include "Renderer/VulkanContext.h"

namespace ale
{
class ShaderResourceManager
{
  public:
	static std::unique_ptr<ShaderResourceManager> createGeometryPassShaderResourceManager(Model *model);
	static std::unique_ptr<ShaderResourceManager> createLightingPassShaderResourceManager(
		VkDescriptorSetLayout descriptorSetLayout, VkImageView positionImageView, VkImageView normalImageView,
		VkImageView albedoImageView, VkImageView pbrImageView, std::vector<VkImageView> &shadowMapImageViews,
		VkSampler shadowMapSamplers, std::vector<VkImageView> &shadowCubeMapImageViews, VkSampler shadowCubeMapSampler,
		VkImageView backgroundImageView, VkSampler backgroundSampler);
	static std::unique_ptr<ShaderResourceManager> createShadowMapShaderResourceManager();
	static std::unique_ptr<ShaderResourceManager> createShadowCubeMapShaderResourceManager();
	static std::unique_ptr<ShaderResourceManager> createViewPortShaderResourceManager(
		VkDescriptorSetLayout descriptorSetLayout, VkImageView viewPortImageView, VkSampler viewPortSampler);
	static std::unique_ptr<ShaderResourceManager> createSphericalMapShaderResourceManager(
		VkDescriptorSetLayout descriptorSetLayout, VkImageView sphericalMapImageView, VkSampler sphericalMapSampler);
	static std::unique_ptr<ShaderResourceManager> createBackgroundShaderResourceManager(
		VkDescriptorSetLayout descriptorSetLayout, VkImageView skyboxImageView, VkSampler skyboxSampler);
	static std::unique_ptr<ShaderResourceManager> createColliderShaderResourceManager(
		VkDescriptorSetLayout descriptorSetLayout);

	void initLightingPassShaderResourceManager(VkDescriptorSetLayout descriptorSetLayout, VkImageView positionImageView,
											   VkImageView normalImageView, VkImageView albedoImageView,
											   VkImageView pbrImageView, std::vector<VkImageView> &shadowMapImageViews,
											   VkSampler shadowMapSamplers,
											   std::vector<VkImageView> &shadowCubeMapImageViews,
											   VkSampler shadowCubeMapSampler, VkImageView backgroundImageView,
											   VkSampler backgroundSampler);

	void initBackgroundShaderResourceManager(VkDescriptorSetLayout descriptorSetLayout, VkImageView skyboxImageView,
											 VkSampler skyboxSampler);

	void initViewPortShaderResourceManager(VkDescriptorSetLayout descriptorSetLayout, VkImageView viewPortImageView,
										   VkSampler viewPortSampler);

	void initSphericalMapShaderResourceManager(VkDescriptorSetLayout descriptorSetLayout,
											   VkImageView sphericalMapImageView, VkSampler sphericalMapSampler);

	void initColliderShaderResourceManager(VkDescriptorSetLayout descriptorSetLayout);
	~ShaderResourceManager() = default;

	void cleanup();

	std::vector<std::shared_ptr<UniformBuffer>> &getUniformBuffers()
	{
		return m_uniformBuffers;
	}
	std::vector<std::shared_ptr<UniformBuffer>> &getLayerIndexUniformBuffers()
	{
		return m_layerIndexUniformBuffers;
	}
	std::vector<std::shared_ptr<UniformBuffer>> &getVertexUniformBuffers()
	{
		return m_vertexUniformBuffers;
	}
	std::vector<std::shared_ptr<UniformBuffer>> &getFragmentUniformBuffers()
	{
		return m_fragmentUniformBuffers;
	}
	std::vector<VkDescriptorSet> &getDescriptorSets()
	{
		return descriptorSets;
	}
	void updateDescriptorSets(Model *model, std::vector<std::shared_ptr<Material>> materials);

  private:
	std::vector<std::shared_ptr<UniformBuffer>> m_uniformBuffers = {};
	std::vector<std::shared_ptr<UniformBuffer>> m_layerIndexUniformBuffers = {};
	std::vector<std::shared_ptr<UniformBuffer>> m_vertexUniformBuffers = {};
	std::vector<std::shared_ptr<UniformBuffer>> m_fragmentUniformBuffers = {};

	std::vector<VkDescriptorSet> descriptorSets = {};

	void initGeometryPassShaderResourceManager(Model *model);
	void createGeometryPassUniformBuffers(Model *model);
	void createGeometryPassDescriptorSets(Model *model);

	void createLightingPassUniformBuffers();
	void createLightingPassDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkImageView positionImageView,
										  VkImageView normalImageView, VkImageView albedoImageView,

										  VkImageView pbrImageView, std::vector<VkImageView> &shadowMapImageViews,
										  VkSampler shadowMapSamplers,
										  std::vector<VkImageView> &shadowCubeMapImageViews,
										  VkSampler shadowCubeMapSampler, VkImageView backgroundImageView,
										  VkSampler backgroundSampler);

	void initShadowMapShaderResourceManager();
	void createShadowMapUniformBuffers();
	void createShadowMapDescriptorSets();

	void initShadowCubeMapShaderResourceManager();
	void createShadowCubeMapUniformBuffers();
	void createShadowCubeMapDescriptorSets();

	void createViewPortDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkImageView viewPortImageView,
									  VkSampler viewPortSampler);

	void createSphericalMapUniformBuffers();
	void createSphericalMapDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkImageView sphericalMapImageView,
										  VkSampler sphericalMapSampler);

	void createBackgroundUniformBuffers();
	void createBackgroundDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, VkImageView skyboxImageView,
										VkSampler skyboxSampler);

	void createColliderUniformBuffers();
	void createColliderDescriptorSets(VkDescriptorSetLayout descriptorSetLayout);
};
} // namespace ale

#endif