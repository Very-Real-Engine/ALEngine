#ifndef PIPELINE_H
#define PIPELINE_H

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/VulkanContext.h"
#include "Renderer/VulkanUtil.h"

namespace ale
{
class AL_API Pipeline
{
  public:
	static std::unique_ptr<Pipeline> createPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
	~Pipeline()
	{
	}
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

	void initPipeline(VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout);
	VkShaderModule createShaderModule(const std::vector<char> &code);
};
} // namespace ale

#endif