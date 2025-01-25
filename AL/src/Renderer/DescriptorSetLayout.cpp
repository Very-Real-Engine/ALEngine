#include "Renderer/DescriptorSetLayout.h"

namespace ale
{
std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createGeometryPassDescriptorSetLayout() {
    std::unique_ptr<DescriptorSetLayout> descriptorSetLayout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
    descriptorSetLayout->initGeometryPassDescriptorSetLayout();
    return descriptorSetLayout;
}


void DescriptorSetLayout::cleanup() {
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
}


void DescriptorSetLayout::initGeometryPassDescriptorSetLayout() {
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();

    VkDescriptorSetLayoutBinding vertexUBOLayoutBinding{};
    vertexUBOLayoutBinding.binding = 0;
    vertexUBOLayoutBinding.descriptorCount = 1;
    vertexUBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    vertexUBOLayoutBinding.pImmutableSamplers = nullptr;
    vertexUBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding vertexHeightMapBinding{};
    vertexHeightMapBinding.binding = 1;
    vertexHeightMapBinding.descriptorCount = 1;
    vertexHeightMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vertexHeightMapBinding.pImmutableSamplers = nullptr;
    vertexHeightMapBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutBinding fragmentUBOLayoutBinding{};
    fragmentUBOLayoutBinding.binding = 2;
    fragmentUBOLayoutBinding.descriptorCount = 1;
    fragmentUBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    fragmentUBOLayoutBinding.pImmutableSamplers = nullptr;
    fragmentUBOLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding normalMapBinding{};
    normalMapBinding.binding = 3;
    normalMapBinding.descriptorCount = 1;
    normalMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalMapBinding.pImmutableSamplers = nullptr;
    normalMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding roughnessMapBinding{};
    roughnessMapBinding.binding = 4;
    roughnessMapBinding.descriptorCount = 1;
    roughnessMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    roughnessMapBinding.pImmutableSamplers = nullptr;
    roughnessMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding metallicMapBinding{};
    metallicMapBinding.binding = 5;
    metallicMapBinding.descriptorCount = 1;
    metallicMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    metallicMapBinding.pImmutableSamplers = nullptr;
    metallicMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding aoMapBinding{};
    aoMapBinding.binding = 6;
    aoMapBinding.descriptorCount = 1;
    aoMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    aoMapBinding.pImmutableSamplers = nullptr;
    aoMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding albedoMapBinding{};
    albedoMapBinding.binding = 7;
    albedoMapBinding.descriptorCount = 1;
    albedoMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    albedoMapBinding.pImmutableSamplers = nullptr;
    albedoMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


    std::array<VkDescriptorSetLayoutBinding, 8> bindings = {vertexUBOLayoutBinding, vertexHeightMapBinding, fragmentUBOLayoutBinding, normalMapBinding, roughnessMapBinding, metallicMapBinding, aoMapBinding, albedoMapBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}


std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createLightingPassDescriptorSetLayout() {
    std::unique_ptr<DescriptorSetLayout> descriptorSetLayout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
    descriptorSetLayout->initLightingPassDescriptorSetLayout();
    return descriptorSetLayout;
}

void DescriptorSetLayout::initLightingPassDescriptorSetLayout() {
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();

    VkDescriptorSetLayoutBinding positionAttachmentBinding{};
    positionAttachmentBinding.binding = 0;
    positionAttachmentBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    positionAttachmentBinding.descriptorCount = 1;
    positionAttachmentBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    positionAttachmentBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding normalAttachmentBinding{};
    normalAttachmentBinding.binding = 1;
    normalAttachmentBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    normalAttachmentBinding.descriptorCount = 1;
    normalAttachmentBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    normalAttachmentBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding albedoAttachmentBinding{};
    albedoAttachmentBinding.binding = 2;
    albedoAttachmentBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    albedoAttachmentBinding.descriptorCount = 1;
    albedoAttachmentBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    albedoAttachmentBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding pbrAttachmentBinding{};
    pbrAttachmentBinding.binding = 3;
    pbrAttachmentBinding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    pbrAttachmentBinding.descriptorCount = 1;
    pbrAttachmentBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pbrAttachmentBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding lightingUBOBinding{};
    lightingUBOBinding.binding = 4;
    lightingUBOBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    lightingUBOBinding.descriptorCount = 1;
    lightingUBOBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    lightingUBOBinding.pImmutableSamplers = nullptr;

    // Shadow Map Sampler
    VkDescriptorSetLayoutBinding shadowMapBinding{};
    shadowMapBinding.binding = 5;
    shadowMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    shadowMapBinding.descriptorCount = 4;
    shadowMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    shadowMapBinding.pImmutableSamplers = nullptr;

    // Shadow Cube Map Sampler
    VkDescriptorSetLayoutBinding shadowCubeMapBinding{};
    shadowCubeMapBinding.binding = 6; // 새롭게 추가된 binding
    shadowCubeMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    shadowCubeMapBinding.descriptorCount = 4;
    shadowCubeMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    shadowCubeMapBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 7> bindings = {
        positionAttachmentBinding,
        normalAttachmentBinding,
        albedoAttachmentBinding,
        pbrAttachmentBinding,
        lightingUBOBinding,
        shadowMapBinding,
        shadowCubeMapBinding
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create Lighting Pass descriptor set layout!");
    }
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createShadowMapDescriptorSetLayout() {
    std::unique_ptr<DescriptorSetLayout> descriptorSetLayout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
    descriptorSetLayout->initShadowMapDescriptorSetLayout();
    return descriptorSetLayout;
}

void DescriptorSetLayout::initShadowMapDescriptorSetLayout() {  
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();

    // Uniform buffer binding for ShadowUniformBufferObject
    VkDescriptorSetLayoutBinding shadowUBOLayoutBinding{};
    shadowUBOLayoutBinding.binding = 0; // Matches shader binding
    shadowUBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    shadowUBOLayoutBinding.descriptorCount = 1;
    shadowUBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // Used in vertex shader
    shadowUBOLayoutBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 1> bindings = {shadowUBOLayoutBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shadow pass descriptor set layout!");
    }
}

std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::createShadowCubeMapDescriptorSetLayout() {
    std::unique_ptr<DescriptorSetLayout> descriptorSetLayout = std::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout());
    descriptorSetLayout->initShadowCubeMapDescriptorSetLayout();
    return descriptorSetLayout;
}

void DescriptorSetLayout::initShadowCubeMapDescriptorSetLayout() {
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();

    // Uniform buffer binding for ShadowUniformBufferObject
    VkDescriptorSetLayoutBinding shadowUBOLayoutBinding{};
    shadowUBOLayoutBinding.binding = 0; // Matches shader binding
    shadowUBOLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    shadowUBOLayoutBinding.descriptorCount = 1;
    shadowUBOLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // Used in vertex shader
    shadowUBOLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding layerIndexBinding{};   
    layerIndexBinding.binding = 1;
    layerIndexBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layerIndexBinding.descriptorCount = 1;
    layerIndexBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layerIndexBinding.pImmutableSamplers = nullptr;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {shadowUBOLayoutBinding, layerIndexBinding};

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shadow pass descriptor set layout!");
    }
}

} // namespace ale