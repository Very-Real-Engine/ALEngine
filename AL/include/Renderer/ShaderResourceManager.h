#ifndef SHADERRESOURCEMANAGER_H
#define SHADERRESOURCEMANAGER_H

#include "Core/Base.h"
#include "Renderer/Buffer.h"
#include "Renderer/Common.h"
#include "Renderer/Model.h"
#include "Renderer/VulkanContext.h"

namespace ale
{
class AL_API ShaderResourceManager
{
public:
	static std::unique_ptr<ShaderResourceManager> createGeometryPassShaderResourceManager(Model* model);
	static std::unique_ptr<ShaderResourceManager> createLightingPassShaderResourceManager(VkDescriptorSetLayout descriptorSetLayout, 
	VkImageView positionImageView, VkImageView normalImageView, VkImageView albedoImageView, 
	VkImageView pbrImageView, std::vector<VkImageView> &shadowMapImageViews, VkSampler shadowMapSamplers,
	std::vector<VkImageView> &shadowCubeMapImageViews, VkSampler shadowCubeMapSampler);
	static std::unique_ptr<ShaderResourceManager> createShadowMapShaderResourceManager();
	static std::unique_ptr<ShaderResourceManager> createShadowCubeMapShaderResourceManager();
	~ShaderResourceManager() {}
	void cleanup();

	std::vector<std::shared_ptr<UniformBuffer>>& getUniformBuffers() { return m_uniformBuffers; }
	std::vector<std::shared_ptr<UniformBuffer>>& getLayerIndexUniformBuffers() { return m_layerIndexUniformBuffers; }
	std::vector<std::shared_ptr<UniformBuffer>>& getVertexUniformBuffers() { return m_vertexUniformBuffers; }
	std::vector<std::shared_ptr<UniformBuffer>>& getFragmentUniformBuffers() { return m_fragmentUniformBuffers; }
	std::vector<VkDescriptorSet>& getDescriptorSets() { return descriptorSets; }

	void updateDescriptorSets(Model* model, std::vector<std::shared_ptr<Material>> materials);
private:
	std::vector<std::shared_ptr<UniformBuffer>> m_uniformBuffers = {};
	std::vector<std::shared_ptr<UniformBuffer>> m_layerIndexUniformBuffers = {};
	std::vector<std::shared_ptr<UniformBuffer>> m_vertexUniformBuffers = {};
	std::vector<std::shared_ptr<UniformBuffer>> m_fragmentUniformBuffers = {};

	std::vector<VkDescriptorSet> descriptorSets = {};
	
	void initGeometryPassShaderResourceManager(Model* model);
	void createGeometryPassUniformBuffers(Model* model);
	void createGeometryPassDescriptorSets(Model* model);



	void initLightingPassShaderResourceManager(VkDescriptorSetLayout descriptorSetLayout, 
	VkImageView positionImageView, VkImageView normalImageView, VkImageView albedoImageView, 
	VkImageView pbrImageView, std::vector<VkImageView> &shadowMapImageViews, VkSampler shadowMapSamplers,
	std::vector<VkImageView> &shadowCubeMapImageViews, VkSampler shadowCubeMapSampler);
	void createLightingPassUniformBuffers();
	void createLightingPassDescriptorSets(VkDescriptorSetLayout descriptorSetLayout, 
	VkImageView positionImageView, VkImageView normalImageView, VkImageView albedoImageView, 
	VkImageView pbrImageView, std::vector<VkImageView> &shadowMapImageViews, VkSampler shadowMapSamplers,
	std::vector<VkImageView> &shadowCubeMapImageViews, VkSampler shadowCubeMapSampler);

	void initShadowMapShaderResourceManager();
	void createShadowMapUniformBuffers();
	void createShadowMapDescriptorSets();

	void initShadowCubeMapShaderResourceManager();
	void createShadowCubeMapUniformBuffers();
	void createShadowCubeMapDescriptorSets();
};

} // namespace ale

#endif