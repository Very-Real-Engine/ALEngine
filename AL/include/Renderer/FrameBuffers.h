#ifndef FRAMEBUFFERS_H
#define FRAMEBUFFERS_H

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/SwapChain.h"
#include "Renderer/VulkanContext.h"

namespace ale
{
class AL_API FrameBuffers
{
public:
	static std::unique_ptr<FrameBuffers> createSwapChainFrameBuffers(SwapChain* swapChain, VkRenderPass renderPass);
	~FrameBuffers() {}
	void cleanup();

	void initSwapChainFrameBuffers(SwapChain* swapChain, VkRenderPass renderPass);

	std::vector<VkFramebuffer>& getFramebuffers() { return framebuffers; }
	VkImageView& getDepthImageView() { return depthImageView; }
	VkImageView& getPositionImageView() { return positionImageView; }
	VkImageView& getNormalImageView() { return normalImageView; }
	VkImageView& getAlbedoImageView() { return albedoImageView; }

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

	std::vector<VkFramebuffer> framebuffers;
};
} // namespace ale

#endif