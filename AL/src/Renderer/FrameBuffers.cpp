#include "Renderer/FrameBuffers.h"
#include "ImGui/ImGuiVulkanRenderer.h"

namespace ale
{

std::unique_ptr<FrameBuffers> FrameBuffers::createSwapChainFrameBuffers(SwapChain *swapChain, VkRenderPass renderPass)
{
	std::unique_ptr<FrameBuffers> frameBuffers = std::unique_ptr<FrameBuffers>(new FrameBuffers());
	frameBuffers->initSwapChainFrameBuffers(swapChain, renderPass);
	return frameBuffers;
}

std::unique_ptr<FrameBuffers> FrameBuffers::createFinalFrameBuffers(SwapChain *swapChain, VkRenderPass renderPass)
{
	std::unique_ptr<FrameBuffers> frameBuffers = std::unique_ptr<FrameBuffers>(new FrameBuffers());
	frameBuffers->initFinalFrameBuffers(swapChain, renderPass);
	return frameBuffers;
}

std::unique_ptr<FrameBuffers> FrameBuffers::createImGuiFrameBuffers(SwapChain *swapChain, VkRenderPass renderPass)
{
	std::unique_ptr<FrameBuffers> frameBuffers = std::unique_ptr<FrameBuffers>(new FrameBuffers());
	frameBuffers->initImGuiFrameBuffers(swapChain, renderPass);
	return frameBuffers;
}

void FrameBuffers::cleanup()
{
	auto &context = VulkanContext::getContext();
	VkDevice device = context.getDevice();

	vkDestroyImageView(device, depthImageView, nullptr);
	vkDestroyImage(device, depthImage, nullptr);
	vkFreeMemory(device, depthImageMemory, nullptr);

	vkDestroyImageView(device, positionImageView, nullptr);
	vkDestroyImage(device, positionImage, nullptr);
	vkFreeMemory(device, positionImageMemory, nullptr);

	vkDestroyImageView(device, normalImageView, nullptr);
	vkDestroyImage(device, normalImage, nullptr);
	vkFreeMemory(device, normalImageMemory, nullptr);

	vkDestroyImageView(device, albedoImageView, nullptr);
	vkDestroyImage(device, albedoImage, nullptr);
	vkFreeMemory(device, albedoImageMemory, nullptr);

	for (auto framebuffer : framebuffers)
	{
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}
}

void FrameBuffers::initSwapChainFrameBuffers(SwapChain *swapChain, VkRenderPass renderPass)
{

	auto &context = VulkanContext::getContext();
	VkDevice device = context.getDevice();
	VkFormat colorFormat = swapChain->getSwapChainImageFormat();
	VkExtent2D extent = swapChain->getSwapChainExtent();
	std::vector<VkImageView> swapChainImageViews = swapChain->getSwapChainImageViews();

	VulkanUtil::createImage(extent.width, extent.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT,
							VK_IMAGE_TILING_OPTIMAL,
							VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, positionImage, positionImageMemory);
	positionImageView =
		VulkanUtil::createImageView(positionImage, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 1);

	VulkanUtil::createImage(extent.width, extent.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT,
							VK_IMAGE_TILING_OPTIMAL,
							VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, normalImage, normalImageMemory);
	normalImageView =
		VulkanUtil::createImageView(normalImage, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 1);

	VulkanUtil::createImage(extent.width, extent.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM,
							VK_IMAGE_TILING_OPTIMAL,
							VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, albedoImage, albedoImageMemory);
	albedoImageView = VulkanUtil::createImageView(albedoImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 1);

	VkFormat depthFormat = VulkanUtil::findDepthFormat();
	VulkanUtil::createImage(extent.width, extent.height, 1, VK_SAMPLE_COUNT_1_BIT, depthFormat, VK_IMAGE_TILING_OPTIMAL,
							VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
							depthImage, depthImageMemory);
	depthImageView = VulkanUtil::createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

	framebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		std::array<VkImageView, 5> attachments = {positionImageView, normalImageView, albedoImageView, depthImageView,
												  swapChainImageViews[i]};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void FrameBuffers::initFinalFrameBuffers(SwapChain *swapChain, VkRenderPass renderPass)
{

	auto &context = VulkanContext::getContext();
	VkDevice device = context.getDevice();
	VkFormat colorFormat = swapChain->getSwapChainImageFormat();
	VkExtent2D extent = swapChain->getSwapChainExtent();
	std::vector<VkImageView> swapChainImageViews = swapChain->getSwapChainImageViews();

	VulkanUtil::createImage(extent.width, extent.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT,
							VK_IMAGE_TILING_OPTIMAL,
							VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, positionImage, positionImageMemory);
	positionImageView =
		VulkanUtil::createImageView(positionImage, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 1);

	VulkanUtil::createImage(extent.width, extent.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT,
							VK_IMAGE_TILING_OPTIMAL,
							VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, normalImage, normalImageMemory);
	normalImageView =
		VulkanUtil::createImageView(normalImage, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 1);

	VulkanUtil::createImage(extent.width, extent.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM,
							VK_IMAGE_TILING_OPTIMAL,
							VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, albedoImage, albedoImageMemory);
	albedoImageView = VulkanUtil::createImageView(albedoImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 1);

	VkFormat depthFormat = VulkanUtil::findDepthFormat();
	VulkanUtil::createImage(extent.width, extent.height, 1, VK_SAMPLE_COUNT_1_BIT, depthFormat, VK_IMAGE_TILING_OPTIMAL,
							VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
							depthImage, depthImageMemory);
	depthImageView = VulkanUtil::createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

	VulkanUtil::createImage(extent.width, extent.height, 1, VK_SAMPLE_COUNT_1_BIT, colorFormat, VK_IMAGE_TILING_OPTIMAL,
							VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT |
								VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, finalImage, finalMemory);
	finalImageView = VulkanUtil::createImageView(finalImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);

	framebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		std::array<VkImageView, 5> attachments = {positionImageView, normalImageView, albedoImageView, depthImageView,
												  finalImageView};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void FrameBuffers::initImGuiFrameBuffers(SwapChain *swapChain, VkRenderPass renderPass)
{
	auto &context = VulkanContext::getContext();
	VkDevice device = context.getDevice();
	VkExtent2D extent = swapChain->getSwapChainExtent();
	std::vector<VkImageView> swapChainImageViews = swapChain->getSwapChainImageViews();

	// Framebuffers 크기를 스왑체인 이미지 뷰 개수에 맞게 설정
	framebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++)
	{
		// Framebuffer에 필요한 첨부물 설정 (스왑체인 이미지 뷰만 사용)
		std::array<VkImageView, 1> attachments = {
			swapChainImageViews[i] // RenderPass의 첨부와 연결된 이미지 뷰
		};

		// Framebuffer 생성 정보 정의
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass; // ImGui RenderPass와 연결
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = extent.width;	// 스왑체인의 폭
		framebufferInfo.height = extent.height; // 스왑체인의 높이
		framebufferInfo.layers = 1;				// 단일 레이어

		// Framebuffer 생성
		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create ImGui framebuffer!");
		}
	}
}

} // namespace ale