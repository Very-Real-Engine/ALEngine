#include "Renderer/FrameBuffers.h"
#include "ImGui/ImGuiVulkanRenderer.h"

namespace ale
{

std::unique_ptr<FrameBuffers> FrameBuffers::createViewPortFrameBuffers(glm::vec2 viewPortSize, VkRenderPass renderPass)
{
	std::unique_ptr<FrameBuffers> frameBuffers = std::unique_ptr<FrameBuffers>(new FrameBuffers());
	frameBuffers->initViewPortFrameBuffers(viewPortSize, renderPass);
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

	vkDestroyImageView(device, pbrImageView, nullptr);
	vkDestroyImage(device, pbrImage, nullptr);
	vkFreeMemory(device, pbrImageMemory, nullptr);

	vkDestroyImageView(device, viewPortImageView, nullptr);
	vkDestroyImage(device, viewPortImage, nullptr);
	vkFreeMemory(device, viewPortImageMemory, nullptr);

	vkDestroyImageView(device, sphericalMapImageView, nullptr);
	vkDestroyImage(device, sphericalMapImage, nullptr);
	vkFreeMemory(device, sphericalMapImageMemory, nullptr);

	vkDestroyImageView(device, backgroundImageView, nullptr);
	vkDestroyImage(device, backgroundImage, nullptr);
	vkFreeMemory(device, backgroundImageMemory, nullptr);

	for (auto framebuffer : framebuffers)
	{
		vkDestroyFramebuffer(device, framebuffer, nullptr);
	}
}

void FrameBuffers::initViewPortFrameBuffers(glm::vec2 viewPortSize, VkRenderPass renderPass)
{
	auto &context = VulkanContext::getContext();
	VkDevice device = context.getDevice();

	VkExtent2D extent = {static_cast<uint32_t>(viewPortSize.x), static_cast<uint32_t>(viewPortSize.y)};

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

	VulkanUtil::createImage(extent.width, extent.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM,
							VK_IMAGE_TILING_OPTIMAL,
							VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, pbrImage, pbrImageMemory);
	pbrImageView = VulkanUtil::createImageView(pbrImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 1);

	VkFormat depthFormat = VulkanUtil::findDepthFormat();
	VulkanUtil::createImage(extent.width, extent.height, 1, VK_SAMPLE_COUNT_1_BIT, depthFormat, VK_IMAGE_TILING_OPTIMAL,
							VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
							depthImage, depthImageMemory);
	depthImageView = VulkanUtil::createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

	VulkanUtil::createImage(extent.width, extent.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM,
							VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, viewPortImage, viewPortImageMemory);
	viewPortImageView =
		VulkanUtil::createImageView(viewPortImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 1);

	framebuffers.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		std::array<VkImageView, 6> attachments = {positionImageView, normalImageView, albedoImageView,
												  pbrImageView,		 depthImageView,  viewPortImageView};

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

std::unique_ptr<FrameBuffers> FrameBuffers::createShadowMapFrameBuffers(VkRenderPass renderPass)
{
	std::unique_ptr<FrameBuffers> frameBuffers = std::unique_ptr<FrameBuffers>(new FrameBuffers());
	frameBuffers->initShadowMapFrameBuffers(renderPass);
	return frameBuffers;
}

void FrameBuffers::initShadowMapFrameBuffers(VkRenderPass renderPass)
{
	auto &context = VulkanContext::getContext();
	VkDevice device = context.getDevice();

	// 고정된 크기 설정
	const uint32_t shadowMapWidth = 2048;
	const uint32_t shadowMapHeight = 2048;

	// 깊이 이미지 생성 (하나만 생성)
	VulkanUtil::createImage(shadowMapWidth, shadowMapHeight, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_D32_SFLOAT,
							VK_IMAGE_TILING_OPTIMAL,
							VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);

	// 깊이 이미지 뷰 생성
	depthImageView = VulkanUtil::createImageView(depthImage, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

	// 프레임버퍼 생성 (두 개)
	framebuffers.resize(2);

	for (size_t i = 0; i < 2; i++)
	{
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &depthImageView;
		framebufferInfo.width = shadowMapWidth;	  // 고정된 width
		framebufferInfo.height = shadowMapHeight; // 고정된 height
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create shadow map framebuffer!");
		}
	}
}

std::unique_ptr<FrameBuffers> FrameBuffers::createShadowCubeMapFrameBuffers(VkRenderPass renderPass)
{
	std::unique_ptr<FrameBuffers> frameBuffers = std::unique_ptr<FrameBuffers>(new FrameBuffers());
	frameBuffers->initShadowCubeMapFrameBuffers(renderPass);
	return frameBuffers;
}

void FrameBuffers::initShadowCubeMapFrameBuffers(VkRenderPass renderPass)
{
	auto &context = VulkanContext::getContext();
	VkDevice device = context.getDevice();

	// 고정된 크기 설정 (Cube Map의 크기)
	const uint32_t shadowMapSize = 2048;

	// Cube Map 깊이 이미지 생성
	VulkanUtil::createCubeMapImage(shadowMapSize, shadowMapSize, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_D32_SFLOAT,
								   VK_IMAGE_TILING_OPTIMAL,
								   VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
								   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);

	depthImageView = VulkanUtil::createCubeMapImageView(depthImage, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

	framebuffers.resize(2);
	for (size_t i = 0; i < 2; i++)
	{
		// 프레임버퍼 생성 (Cube Map 자체에 대해 단일 프레임버퍼 생성)
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &depthImageView;
		framebufferInfo.width = shadowMapSize;
		framebufferInfo.height = shadowMapSize;
		framebufferInfo.layers = 6;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create point light shadow map framebuffer!");
		}
	}
}

std::unique_ptr<FrameBuffers> FrameBuffers::createSphericalMapFrameBuffers(VkRenderPass renderPass)
{
	std::unique_ptr<FrameBuffers> frameBuffers = std::unique_ptr<FrameBuffers>(new FrameBuffers());
	frameBuffers->initSphericalMapFrameBuffers(renderPass);
	return frameBuffers;
}

void FrameBuffers::initSphericalMapFrameBuffers(VkRenderPass renderPass)
{
	auto &context = VulkanContext::getContext();
	VkDevice device = context.getDevice();

	const uint32_t sphericalMapSize = 2048;

	VulkanUtil::createCubeMapImage(sphericalMapSize, sphericalMapSize, 1, VK_SAMPLE_COUNT_1_BIT,
								   VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
								   VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
								   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, sphericalMapImage, sphericalMapImageMemory);

	sphericalMapImageView = VulkanUtil::createCubeMapImageView(sphericalMapImage, VK_FORMAT_R16G16B16A16_SFLOAT,
															   VK_IMAGE_ASPECT_COLOR_BIT, 1);

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.pAttachments = &sphericalMapImageView;
	framebufferInfo.width = sphericalMapSize;
	framebufferInfo.height = sphericalMapSize;
	framebufferInfo.layers = 6;

	framebuffers.resize(1);
	if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[0]) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create spherical map framebuffer!");
	}
}

std::unique_ptr<FrameBuffers> FrameBuffers::createBackgroundFrameBuffers(glm::vec2 viewPortSize,
																		 VkRenderPass renderPass)
{
	std::unique_ptr<FrameBuffers> frameBuffers = std::unique_ptr<FrameBuffers>(new FrameBuffers());
	frameBuffers->initBackgroundFrameBuffers(viewPortSize, renderPass);
	return frameBuffers;
}

void FrameBuffers::initBackgroundFrameBuffers(glm::vec2 viewPortSize, VkRenderPass renderPass)
{
	auto &context = VulkanContext::getContext();
	VkDevice device = context.getDevice();

	VkExtent2D extent = {static_cast<uint32_t>(viewPortSize.x), static_cast<uint32_t>(viewPortSize.y)};

	VulkanUtil::createImage(extent.width, extent.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT,
							VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, backgroundImage, backgroundImageMemory);

	backgroundImageView =
		VulkanUtil::createImageView(backgroundImage, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 1);

	framebuffers.resize(MAX_FRAMES_IN_FLIGHT);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = &backgroundImageView;
		framebufferInfo.width = extent.width;
		framebufferInfo.height = extent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to create background framebuffer!");
		}
	}
}

std::unique_ptr<FrameBuffers> FrameBuffers::createColliderFrameBuffers(glm::vec2 viewPortSize, VkRenderPass renderPass,
																	   VkImageView viewPortImageView)
{
	std::unique_ptr<FrameBuffers> frameBuffers = std::unique_ptr<FrameBuffers>(new FrameBuffers());
	frameBuffers->initColliderFrameBuffers(viewPortSize, renderPass, viewPortImageView);
	return frameBuffers;
}

void FrameBuffers::initColliderFrameBuffers(glm::vec2 viewPortSize, VkRenderPass renderPass,
											VkImageView viewPortImageView)
{
	auto &context = VulkanContext::getContext();
	VkDevice device = context.getDevice();

	framebuffers.resize(1);
	std::array<VkImageView, 1> attachments = {viewPortImageView};

	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderPass;
	framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	framebufferInfo.pAttachments = attachments.data();
	framebufferInfo.width = static_cast<uint32_t>(viewPortSize.x);
	framebufferInfo.height = static_cast<uint32_t>(viewPortSize.y);
	framebufferInfo.layers = 1;

	if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[0]) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create collider framebuffer!");
	}
}
} // namespace ale