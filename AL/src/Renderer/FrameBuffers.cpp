#include "Renderer/FrameBuffers.h"

namespace ale
{

std::unique_ptr<FrameBuffers> FrameBuffers::createSwapChainFrameBuffers(SwapChain* swapChain, VkRenderPass renderPass) {
    std::unique_ptr<FrameBuffers> frameBuffers = std::unique_ptr<FrameBuffers>(new FrameBuffers());
    frameBuffers->initSwapChainFrameBuffers(swapChain, renderPass);
    return frameBuffers;
}


void FrameBuffers::cleanup() {
    auto& context = VulkanContext::getContext();
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

    for (auto framebuffer : framebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
}


void FrameBuffers::initSwapChainFrameBuffers(SwapChain* swapChain, VkRenderPass renderPass) {
    
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();
    VkFormat colorFormat = swapChain->getSwapChainImageFormat();
    VkExtent2D extent = swapChain->getSwapChainExtent();
    std::vector<VkImageView> swapChainImageViews = swapChain->getSwapChainImageViews();

    VulkanUtil::createImage(
        extent.width, extent.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, 
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        positionImage, positionImageMemory);
    positionImageView = VulkanUtil::createImageView(positionImage, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 1);

    VulkanUtil::createImage(
        extent.width, extent.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R16G16B16A16_SFLOAT, 
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        normalImage, normalImageMemory);
    normalImageView = VulkanUtil::createImageView(normalImage, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT, 1);

    VulkanUtil::createImage(
        extent.width, extent.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, 
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        albedoImage, albedoImageMemory);
    albedoImageView = VulkanUtil::createImageView(albedoImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 1);

    VulkanUtil::createImage(
        extent.width, extent.height, 1, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, 
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        pbrImage, pbrImageMemory);
    pbrImageView = VulkanUtil::createImageView(pbrImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, 1);

    VkFormat depthFormat = VulkanUtil::findDepthFormat();
    VulkanUtil::createImage(
        extent.width, extent.height, 1, VK_SAMPLE_COUNT_1_BIT, depthFormat, 
        VK_IMAGE_TILING_OPTIMAL, 
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
        depthImage, depthImageMemory);
    depthImageView = VulkanUtil::createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    framebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        std::array<VkImageView, 6> attachments = {
            positionImageView,
            normalImageView,
            albedoImageView,
            pbrImageView,
            depthImageView,
            swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

std::unique_ptr<FrameBuffers> FrameBuffers::createShadowMapFrameBuffers(VkRenderPass renderPass) {
    std::unique_ptr<FrameBuffers> frameBuffers = std::unique_ptr<FrameBuffers>(new FrameBuffers());
    frameBuffers->initShadowMapFrameBuffers(renderPass);
    return frameBuffers;
}

void FrameBuffers::initShadowMapFrameBuffers(VkRenderPass renderPass) {
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();

    // 고정된 크기 설정
    const uint32_t shadowMapWidth = 2048;
    const uint32_t shadowMapHeight = 2048;

    // 깊이 이미지 생성 (하나만 생성)
    VulkanUtil::createImage(
        shadowMapWidth, shadowMapHeight, 1, VK_SAMPLE_COUNT_1_BIT,
        VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);

    // 깊이 이미지 뷰 생성
    depthImageView = VulkanUtil::createImageView(
        depthImage, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    // 프레임버퍼 생성 (두 개)
    framebuffers.resize(2);

    for (size_t i = 0; i < 2; i++) {
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &depthImageView;
        framebufferInfo.width = shadowMapWidth;   // 고정된 width
        framebufferInfo.height = shadowMapHeight; // 고정된 height
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create shadow map framebuffer!");
        }
    }
}

std::unique_ptr<FrameBuffers> FrameBuffers::createShadowCubeMapFrameBuffers(VkRenderPass renderPass) {
    std::unique_ptr<FrameBuffers> frameBuffers = std::unique_ptr<FrameBuffers>(new FrameBuffers());
    frameBuffers->initShadowCubeMapFrameBuffers(renderPass);
    return frameBuffers;
}

void FrameBuffers::initShadowCubeMapFrameBuffers(VkRenderPass renderPass) {
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();

    // 고정된 크기 설정 (Cube Map의 크기)
    const uint32_t shadowMapSize = 2048;

    // Cube Map 깊이 이미지 생성
    VulkanUtil::createCubeMapImage(
        shadowMapSize, shadowMapSize, 1, VK_SAMPLE_COUNT_1_BIT,
        VK_FORMAT_D32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);

    depthImageView = VulkanUtil::createCubeMapImageView(depthImage, VK_FORMAT_D32_SFLOAT, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

    framebuffers.resize(2);
    for (size_t i = 0; i < 2; i++) {
        // 프레임버퍼 생성 (Cube Map 자체에 대해 단일 프레임버퍼 생성)
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &depthImageView;
        framebufferInfo.width = shadowMapSize;
        framebufferInfo.height = shadowMapSize;
        framebufferInfo.layers = 6;
        
        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create point light shadow map framebuffer!");
        }
    }
}
} // namespace ale