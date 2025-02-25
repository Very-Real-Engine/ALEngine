#ifndef PIPELINE_H
#define PIPELINE_H

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/VulkanContext.h"
#include "Renderer/VulkanUtil.h"

namespace ale
{
class Pipeline
{
  public:
	static std::unique_ptr<Pipeline> createGeometryPassPipeline(VkRenderPass renderPass,
																VkDescriptorSetLayout descriptorSetLayout);
	static std::unique_ptr<Pipeline> createLightingPassPipeline(VkRenderPass renderPass,
																VkDescriptorSetLayout descriptorSetLayout);
	static std::unique_ptr<Pipeline> createShadowMapPipeline(VkRenderPass renderPass,
															 VkDescriptorSetLayout descriptorSetLayout);
	static std::unique_ptr<Pipeline> createShadowCubeMapPipeline(VkRenderPass renderPass,
																 VkDescriptorSetLayout descriptorSetLayout);
	static std::unique_ptr<Pipeline> createSphericalMapPipeline(VkRenderPass renderPass,
																VkDescriptorSetLayout descriptorSetLayout);
	static std::unique_ptr<Pipeline> createBackgroundPipeline(VkRenderPass renderPass,
															  VkDescriptorSetLayout descriptorSetLayout);
	static std::unique_ptr<Pipeline> createColliderPipeline(VkRenderPass renderPass,
															VkDescriptorSetLayout descriptorSetLayout);

	void initGeometryPassPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
	void initLightingPassPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
	void initShadowMapPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
	void initShadowCubeMapPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
	void initSphericalMapPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
	void initBackgroundPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
	void initColliderPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);

	~Pipeline() = default;

	void cleanup();

	VkPipeline getPipeline()
	{
		return pipeline;
	}
	VkPipelineLayout getPipelineLayout()
	{
		return pipelineLayout;
	}

  private:
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;

	VkShaderModule createShaderModule(const std::vector<char> &code);
};
} // namespace ale

#endif