#ifndef RENDERPASS_H
#define RENDERPASS_H

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/VulkanContext.h"
#include "Renderer/VulkanUtil.h"

namespace ale
{
class AL_API RenderPass
{
  public:
	static std::unique_ptr<RenderPass> createRenderPass(VkFormat swapChainImageFormat);

	~RenderPass()
	{
	}
	void cleanup();

	VkRenderPass getRenderPass()
	{
		return renderPass;
	}

  private:
	VkRenderPass renderPass;

	void initRenderPass(VkFormat swapChainImageFormat);
};
} // namespace ale

#endif