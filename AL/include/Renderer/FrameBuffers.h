#ifndef FRAMEBUFFERS_H
#define FRAMEBUFFERS_H

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/SwapChain.h"
#include "Renderer/VulkanContext.h"

namespace ale
{
class FrameBuffers
{
  public:
	static std::unique_ptr<FrameBuffers> createViewPortFrameBuffers(alglm::vec2 viewPortSize, VkRenderPass renderPass);
	static std::unique_ptr<FrameBuffers> createImGuiFrameBuffers(SwapChain *swapChain, VkRenderPass renderPass);
	static std::unique_ptr<FrameBuffers> createShadowMapFrameBuffers(VkRenderPass renderPass);
	static std::unique_ptr<FrameBuffers> createShadowCubeMapFrameBuffers(VkRenderPass renderPass);
	static std::unique_ptr<FrameBuffers> createSphericalMapFrameBuffers(VkRenderPass renderPass);
	static std::unique_ptr<FrameBuffers> createBackgroundFrameBuffers(alglm::vec2 viewPortSize,
																	  VkRenderPass renderPass);

	~FrameBuffers() = default;

	void cleanup();

	void initViewPortFrameBuffers(alglm::vec2 viewPortSize, VkRenderPass renderPass);
	void initImGuiFrameBuffers(SwapChain *swapChain, VkRenderPass renderPass);
	void initShadowMapFrameBuffers(VkRenderPass renderPass);
	void initShadowCubeMapFrameBuffers(VkRenderPass renderPass);
	void initSphericalMapFrameBuffers(VkRenderPass renderPass);
	void initBackgroundFrameBuffers(alglm::vec2 viewPortSize, VkRenderPass renderPass);

	VkFramebuffer &getSphericalMapFramebuffer()
	{
		return framebuffers[0];
	}

	VkImageView &getSphericalMapImageView()
	{
		return sphericalMapImageView;
	}

	std::vector<VkFramebuffer> &getFramebuffers()

	{
		return framebuffers;
	}

	VkImage &getDepthImage()
	{
		return depthImage;
	}
	VkImageView &getDepthImageView()
	{
		return depthImageView;
	}
	VkImageView &getPositionImageView()
	{
		return positionImageView;
	}
	VkImageView &getNormalImageView()
	{
		return normalImageView;
	}
	VkImageView &getAlbedoImageView()
	{
		return albedoImageView;
	}
	VkImageView &getPbrImageView()
	{
		return pbrImageView;
	}
	VkImageView &getViewPortImageView()
	{
		return viewPortImageView;
	}
	VkImage &getViewPortImage()
	{
		return viewPortImage;
	}
	VkImageView &getBackgroundImageView()
	{
		return backgroundImageView;
	}
	VkImage &getBackgroundImage()
	{
		return backgroundImage;
	}

  private:
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkImage positionImage;
	VkDeviceMemory positionImageMemory;
	VkImageView positionImageView;

	VkImage normalImage;
	VkDeviceMemory normalImageMemory;
	VkImageView normalImageView;

	VkImage albedoImage;
	VkDeviceMemory albedoImageMemory;
	VkImageView albedoImageView;

	VkImage pbrImage;
	VkDeviceMemory pbrImageMemory;
	VkImageView pbrImageView;

	VkImage viewPortImage;
	VkDeviceMemory viewPortImageMemory;
	VkImageView viewPortImageView;

	VkImage sphericalMapImage;
	VkDeviceMemory sphericalMapImageMemory;
	VkImageView sphericalMapImageView;

	VkImage backgroundImage;
	VkDeviceMemory backgroundImageMemory;
	VkImageView backgroundImageView;

	std::vector<VkFramebuffer> framebuffers;
};
} // namespace ale

#endif