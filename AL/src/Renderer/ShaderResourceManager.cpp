#include "Renderer/ShaderResourceManager.h"

namespace ale
{
void ShaderResourceManager::cleanup()
{
	if (!m_uniformBuffers.empty())
	{
		for (size_t i = 0; i < m_uniformBuffers.size(); i++)
		{
			m_uniformBuffers[i]->cleanup();
		}
	}
	m_uniformBuffers.clear();

	if (!m_layerIndexUniformBuffers.empty())
	{
		for (size_t i = 0; i < m_layerIndexUniformBuffers.size(); i++)
		{
			m_layerIndexUniformBuffers[i]->cleanup();
		}
	}
	m_layerIndexUniformBuffers.clear();

	if (!m_vertexUniformBuffers.empty())

	{
		for (size_t i = 0; i < m_vertexUniformBuffers.size(); i++)
		{
			m_vertexUniformBuffers[i]->cleanup();
		}
	}
	m_vertexUniformBuffers.clear();

	if (!m_fragmentUniformBuffers.empty())
	{
		for (size_t i = 0; i < m_fragmentUniformBuffers.size(); i++)
		{
			m_fragmentUniformBuffers[i]->cleanup();
		}
	}
	m_fragmentUniformBuffers.clear();

	auto &context = VulkanContext::getContext();
	VkDevice device = context.getDevice();
	VkDescriptorPool descriptorPool = context.getDescriptorPool();

	if (!descriptorSets.empty())
	{
		for (auto &descriptorSet : descriptorSets)
		{
			if (descriptorSet != VK_NULL_HANDLE)
			{
				VkResult result = vkFreeDescriptorSets(device, descriptorPool, 1, &descriptorSet);
				if (result != VK_SUCCESS)
				{
					std::cerr << "Warning: Failed to free descriptor set!" << std::endl;
				}
				descriptorSet = VK_NULL_HANDLE;
			}
		}
		descriptorSets.clear();
	}
}

std::unique_ptr<ShaderResourceManager> ShaderResourceManager::createGeometryPassShaderResourceManager(Model *model)

{
	std::unique_ptr<ShaderResourceManager> shaderResourceManager =
		std::unique_ptr<ShaderResourceManager>(new ShaderResourceManager());
	shaderResourceManager->initGeometryPassShaderResourceManager(model);
	return shaderResourceManager;
}

void ShaderResourceManager::initGeometryPassShaderResourceManager(Model *model)
{
	createGeometryPassUniformBuffers(model);
	createGeometryPassDescriptorSets(model);
}

void ShaderResourceManager::createGeometryPassUniformBuffers(Model *model)
{
	size_t meshCount = model->getMeshCount();
	if (meshCount == 0)
	{
		throw std::runtime_error("failed to create uniform buffers!");
	}
	// 유니폼 버퍼에 저장 될 구조체의 크기
	VkDeviceSize vertexBufferSize = sizeof(GeometryPassVertexUniformBufferObject);
	VkDeviceSize fragmentBufferSize = sizeof(GeometryPassFragmentUniformBufferObject);

	// 각 요소들을 동시에 처리 가능한 최대 프레임 수만큼 만들어 둔다.
	m_vertexUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT * meshCount);
	m_fragmentUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT * meshCount);

	for (size_t i = 0; i < meshCount; i++)
	{
		for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++)
		{
			m_vertexUniformBuffers[i * MAX_FRAMES_IN_FLIGHT + j] = UniformBuffer::createUniformBuffer(vertexBufferSize);
			m_fragmentUniformBuffers[i * MAX_FRAMES_IN_FLIGHT + j] =
				UniformBuffer::createUniformBuffer(fragmentBufferSize);
		}
	}
}

void ShaderResourceManager::createGeometryPassDescriptorSets(Model *model)
{
	auto &context = VulkanContext::getContext();
	VkDevice device = context.getDevice();
	VkDescriptorPool descriptorPool = context.getDescriptorPool();
	VkDescriptorSetLayout descriptorSetLayout = context.getGeometryPassDescriptorSetLayout();

	size_t meshCount = model->getMeshCount();
	std::vector<std::shared_ptr<Mesh>> &meshes = model->getMeshes();
	std::vector<std::shared_ptr<Material>> &materials = model->getMaterials();

	if (meshCount == 0)
	{
		throw std::runtime_error("failed to create descriptor sets!");
	}
	// 디스크립터 셋 레이아웃 벡터 생성 (기존 만들어놨던 디스크립터 셋 레이아웃 객체 이용)
	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT * meshCount, descriptorSetLayout);

	// 디스크립터 셋 할당에 필요한 정보를 설정하는 구조체
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool; // 디스크립터 셋을 할당할 디스크립터 풀 지정
	allocInfo.descriptorSetCount =
		static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * meshCount); // 할당할 디스크립터 셋 개수 지정
	allocInfo.pSetLayouts = layouts.data();						 // 할당할 디스크립터 셋 의 레이아웃을 정의하는 배열

	descriptorSets.resize(MAX_FRAMES_IN_FLIGHT * meshCount); // 디스크립터 셋을 저장할 벡터 크기 설정

	// 디스크립터 풀에 디스크립터 셋 할당
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < meshCount; i++)
	{
		for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++)
		{
			auto &mesh = meshes[i];
			auto &material = materials[i];
			size_t index = i * MAX_FRAMES_IN_FLIGHT + j;

			// Vertex Uniform Buffer
			VkDescriptorBufferInfo vertexBufferInfo{};
			vertexBufferInfo.buffer = m_vertexUniformBuffers[index]->getBuffer();
			vertexBufferInfo.offset = 0;
			vertexBufferInfo.range = sizeof(GeometryPassVertexUniformBufferObject);

			// Fragment Uniform Buffer
			VkDescriptorBufferInfo fragmentBufferInfo{};
			fragmentBufferInfo.buffer = m_fragmentUniformBuffers[index]->getBuffer();
			fragmentBufferInfo.offset = 0;
			fragmentBufferInfo.range = sizeof(GeometryPassFragmentUniformBufferObject);

			Albedo &albedo = material->getAlbedo();
			NormalMap &normalMap = material->getNormalMap();
			Roughness &roughness = material->getRoughness();
			Metallic &metallic = material->getMetallic();
			AOMap &aoMap = material->getAOMap();
			HeightMap &heightMap = material->getHeightMap();

			std::array<VkDescriptorImageInfo, 6> imageInfos{
				VkDescriptorImageInfo{normalMap.normalTexture->getSampler(), normalMap.normalTexture->getImageView(),
									  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
				VkDescriptorImageInfo{roughness.roughnessTexture->getSampler(),
									  roughness.roughnessTexture->getImageView(),
									  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
				VkDescriptorImageInfo{metallic.metallicTexture->getSampler(), metallic.metallicTexture->getImageView(),
									  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
				VkDescriptorImageInfo{aoMap.aoTexture->getSampler(), aoMap.aoTexture->getImageView(),
									  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
				VkDescriptorImageInfo{albedo.albedoTexture->getSampler(), albedo.albedoTexture->getImageView(),
									  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
				VkDescriptorImageInfo{heightMap.heightTexture->getSampler(), heightMap.heightTexture->getImageView(),
									  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}};

			// Descriptor Writes
			std::array<VkWriteDescriptorSet, 8> descriptorWrites{};

			// Vertex UBO
			descriptorWrites[0] = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSets[index], 0,	   0, 1,
								   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	   nullptr, &vertexBufferInfo,	   nullptr};

			// Vertex Height Map
			descriptorWrites[1] = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
								   nullptr,
								   descriptorSets[index],
								   1,
								   0,
								   1,
								   VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
								   &imageInfos[5],
								   nullptr,
								   nullptr};

			// Fragment UBO
			descriptorWrites[2] = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSets[index], 2,	   0, 1,
								   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	   nullptr, &fragmentBufferInfo,   nullptr};

			for (size_t k = 0; k < 5; k++)
			{
				descriptorWrites[3 + k] = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
										   nullptr,
										   descriptorSets[index],
										   static_cast<uint32_t>(3 + k),
										   0,
										   1,
										   VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
										   &imageInfos[k],
										   nullptr,
										   nullptr};
			}
			// Descriptor Set 업데이트
			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
								   nullptr);
		}
	}
}

void ShaderResourceManager::updateDescriptorSets(Model *model, std::vector<std::shared_ptr<Material>> materials)
{
	auto &context = VulkanContext::getContext();
	VkDevice device = context.getDevice();

	size_t meshCount = model->getMeshCount();
	std::vector<std::shared_ptr<Mesh>> &meshes = model->getMeshes();

	if (meshCount == 0)
	{
		throw std::runtime_error("No meshes found in the model!");
	}
	vkDeviceWaitIdle(device);

	for (size_t i = 0; i < meshCount; i++)
	{
		for (size_t j = 0; j < MAX_FRAMES_IN_FLIGHT; j++)
		{
			auto &mesh = meshes[i];
			auto &material = materials[i];
			size_t index = i * MAX_FRAMES_IN_FLIGHT + j;

			// Vertex Uniform Buffer
			VkDescriptorBufferInfo vertexBufferInfo{};
			vertexBufferInfo.buffer = m_vertexUniformBuffers[index]->getBuffer();
			vertexBufferInfo.offset = 0;
			vertexBufferInfo.range = sizeof(GeometryPassVertexUniformBufferObject);

			// Fragment Uniform Buffer
			VkDescriptorBufferInfo fragmentBufferInfo{};
			fragmentBufferInfo.buffer = m_fragmentUniformBuffers[index]->getBuffer();
			fragmentBufferInfo.offset = 0;
			fragmentBufferInfo.range = sizeof(GeometryPassFragmentUniformBufferObject);

			Albedo &albedo = material->getAlbedo();
			NormalMap &normalMap = material->getNormalMap();
			Roughness &roughness = material->getRoughness();
			Metallic &metallic = material->getMetallic();
			AOMap &aoMap = material->getAOMap();
			HeightMap &heightMap = material->getHeightMap();

			std::array<VkDescriptorImageInfo, 6> imageInfos{
				VkDescriptorImageInfo{normalMap.normalTexture->getSampler(), normalMap.normalTexture->getImageView(),
									  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
				VkDescriptorImageInfo{roughness.roughnessTexture->getSampler(),
									  roughness.roughnessTexture->getImageView(),
									  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
				VkDescriptorImageInfo{metallic.metallicTexture->getSampler(), metallic.metallicTexture->getImageView(),
									  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
				VkDescriptorImageInfo{aoMap.aoTexture->getSampler(), aoMap.aoTexture->getImageView(),
									  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
				VkDescriptorImageInfo{albedo.albedoTexture->getSampler(), albedo.albedoTexture->getImageView(),
									  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL},
				VkDescriptorImageInfo{heightMap.heightTexture->getSampler(), heightMap.heightTexture->getImageView(),
									  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL}};

			// Descriptor Writes
			std::array<VkWriteDescriptorSet, 8> descriptorWrites{};

			// Vertex UBO
			descriptorWrites[0] = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSets[index], 0,	   0, 1,
								   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	   nullptr, &vertexBufferInfo,	   nullptr};

			// Vertex Height Map
			descriptorWrites[1] = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
								   nullptr,
								   descriptorSets[index],
								   1,
								   0,
								   1,
								   VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
								   &imageInfos[5],
								   nullptr,
								   nullptr};

			// Fragment UBO
			descriptorWrites[2] = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, descriptorSets[index], 2,	   0, 1,
								   VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	   nullptr, &fragmentBufferInfo,   nullptr};

			for (size_t k = 0; k < 5; k++)
			{
				descriptorWrites[3 + k] = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
										   nullptr,
										   descriptorSets[index],
										   static_cast<uint32_t>(3 + k),
										   0,
										   1,
										   VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
										   &imageInfos[k],
										   nullptr,
										   nullptr};
			}
			// Descriptor Set 업데이트
			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
								   nullptr);
		}
	}
}

std::unique_ptr<ShaderResourceManager> ShaderResourceManager::createLightingPassShaderResourceManager(
	VkDescriptorSetLayout descriptorSetLayout, VkImageView positionImageView, VkImageView normalImageView,
	VkImageView albedoImageView, VkImageView pbrImageView, std::vector<VkImageView> &shadowMapImageViews,
	VkSampler shadowMapSamplers, std::vector<VkImageView> &shadowCubeMapImageViews, VkSampler shadowCubeMapSampler,
	VkImageView backgroundImageView, VkSampler backgroundSampler)
{
	std::unique_ptr<ShaderResourceManager> shaderResourceManager =

		std::unique_ptr<ShaderResourceManager>(new ShaderResourceManager());
	shaderResourceManager->initLightingPassShaderResourceManager(
		descriptorSetLayout, positionImageView, normalImageView, albedoImageView, pbrImageView, shadowMapImageViews,
		shadowMapSamplers, shadowCubeMapImageViews, shadowCubeMapSampler, backgroundImageView, backgroundSampler);
	return shaderResourceManager;
}

void ShaderResourceManager::initLightingPassShaderResourceManager(
	VkDescriptorSetLayout descriptorSetLayout, VkImageView positionImageView, VkImageView normalImageView,
	VkImageView albedoImageView, VkImageView pbrImageView, std::vector<VkImageView> &shadowMapImageViews,
	VkSampler shadowMapSamplers, std::vector<VkImageView> &shadowCubeMapImageViews, VkSampler shadowCubeMapSampler,
	VkImageView backgroundImageView, VkSampler backgroundSampler)
{
	createLightingPassUniformBuffers();

	createLightingPassDescriptorSets(descriptorSetLayout, positionImageView, normalImageView, albedoImageView,
									 pbrImageView, shadowMapImageViews, shadowMapSamplers, shadowCubeMapImageViews,
									 shadowCubeMapSampler, backgroundImageView, backgroundSampler);
}

void ShaderResourceManager::createLightingPassUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(LightingPassUniformBufferObject);

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(VulkanContext::getContext().getPhysicalDevice(), &deviceProperties);
	if (bufferSize > deviceProperties.limits.maxUniformBufferRange)
	{
		throw std::runtime_error("LightingPassUniformBufferObject size exceeds maxUniformBufferRange!");
	}

	m_fragmentUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		m_fragmentUniformBuffers[i] = UniformBuffer::createUniformBuffer(bufferSize);
	}
}

void ShaderResourceManager::createLightingPassDescriptorSets(
	VkDescriptorSetLayout descriptorSetLayout, VkImageView positionImageView, VkImageView normalImageView,
	VkImageView albedoImageView, VkImageView pbrImageView, std::vector<VkImageView> &shadowMapImageViews,
	VkSampler shadowMapSamplers, std::vector<VkImageView> &shadowCubeMapImageViews, VkSampler shadowCubeMapSampler,
	VkImageView backgroundImageView, VkSampler backgroundSampler)
{
	auto &context = VulkanContext::getContext();
	VkDevice device = context.getDevice();
	VkDescriptorPool descriptorPool = context.getDescriptorPool();

	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{

		VkDescriptorImageInfo positionImageInfo{};
		positionImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		positionImageInfo.imageView = positionImageView;
		positionImageInfo.sampler = VK_NULL_HANDLE;

		VkDescriptorImageInfo normalImageInfo{};
		normalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		normalImageInfo.imageView = normalImageView;
		normalImageInfo.sampler = VK_NULL_HANDLE;

		VkDescriptorImageInfo albedoImageInfo{};
		albedoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		albedoImageInfo.imageView = albedoImageView;
		albedoImageInfo.sampler = VK_NULL_HANDLE;

		VkDescriptorImageInfo pbrImageInfo{};
		pbrImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		pbrImageInfo.imageView = pbrImageView;
		pbrImageInfo.sampler = VK_NULL_HANDLE;

		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_fragmentUniformBuffers[i]->getBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(LightingPassUniformBufferObject);

		VkDescriptorImageInfo backgroundImageInfo{};
		backgroundImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		backgroundImageInfo.imageView = backgroundImageView;
		backgroundImageInfo.sampler = backgroundSampler;

		std::array<VkDescriptorImageInfo, 4> shadowMapInfos{};
		for (size_t j = 0; j < shadowMapImageViews.size(); ++j)

		{
			shadowMapInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			shadowMapInfos[j].imageView = shadowMapImageViews[j];
			shadowMapInfos[j].sampler = shadowMapSamplers;
		}

		if (shadowMapImageViews.size() < 4)
		{
			for (size_t j = shadowMapImageViews.size(); j < 4; ++j)
			{
				shadowMapInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				shadowMapInfos[j].imageView = VK_NULL_HANDLE;
				shadowMapInfos[j].sampler = VK_NULL_HANDLE;
			}
		}

		std::array<VkDescriptorImageInfo, 4> shadowCubeMapInfos{};
		for (size_t j = 0; j < 4; ++j)
		{
			shadowCubeMapInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			shadowCubeMapInfos[j].imageView = shadowCubeMapImageViews[j];
			shadowCubeMapInfos[j].sampler = shadowCubeMapSampler;
		}

		if (shadowCubeMapImageViews.size() < 4)
		{
			for (size_t j = shadowCubeMapImageViews.size(); j < 4; ++j)
			{
				shadowCubeMapInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				shadowCubeMapInfos[j].imageView = VK_NULL_HANDLE;
				shadowCubeMapInfos[j].sampler = VK_NULL_HANDLE;
			}
		}

		std::array<VkWriteDescriptorSet, 8> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pImageInfo = &positionImageInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &normalImageInfo;

		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = descriptorSets[i];
		descriptorWrites[2].dstBinding = 2;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pImageInfo = &albedoImageInfo;

		descriptorWrites[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[3].dstSet = descriptorSets[i];
		descriptorWrites[3].dstBinding = 3;
		descriptorWrites[3].descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		descriptorWrites[3].descriptorCount = 1;
		descriptorWrites[3].pImageInfo = &pbrImageInfo;

		descriptorWrites[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[4].dstSet = descriptorSets[i];
		descriptorWrites[4].dstBinding = 4;
		descriptorWrites[4].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[4].descriptorCount = 1;
		descriptorWrites[4].pBufferInfo = &bufferInfo;

		// Shadow Map Descriptor
		descriptorWrites[5].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[5].dstSet = descriptorSets[i];
		descriptorWrites[5].dstBinding = 5;
		descriptorWrites[5].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[5].descriptorCount = static_cast<uint32_t>(shadowMapInfos.size());
		descriptorWrites[5].pImageInfo = shadowMapInfos.data();

		descriptorWrites[6].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[6].dstSet = descriptorSets[i];
		descriptorWrites[6].dstBinding = 6;
		descriptorWrites[6].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[6].descriptorCount = static_cast<uint32_t>(shadowCubeMapInfos.size());
		descriptorWrites[6].pImageInfo = shadowCubeMapInfos.data();

		descriptorWrites[7].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[7].dstSet = descriptorSets[i];
		descriptorWrites[7].dstBinding = 7;
		descriptorWrites[7].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[7].descriptorCount = 1;
		descriptorWrites[7].pImageInfo = &backgroundImageInfo;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
							   nullptr);
	}
}

std::unique_ptr<ShaderResourceManager> ShaderResourceManager::createShadowMapShaderResourceManager()
{
	std::unique_ptr<ShaderResourceManager> shaderResourceManager =
		std::unique_ptr<ShaderResourceManager>(new ShaderResourceManager());
	shaderResourceManager->initShadowMapShaderResourceManager();
	return shaderResourceManager;
}

void ShaderResourceManager::initShadowMapShaderResourceManager()
{
	createShadowMapUniformBuffers();
	createShadowMapDescriptorSets();
}

void ShaderResourceManager::createShadowMapUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(ShadowMapUniformBufferObject);
	m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		m_uniformBuffers[i] = UniformBuffer::createUniformBuffer(bufferSize);
	}
}

void ShaderResourceManager::createShadowMapDescriptorSets()
{
	auto &context = VulkanContext::getContext();
	VkDevice device = context.getDevice();
	VkDescriptorPool descriptorPool = context.getDescriptorPool();
	VkDescriptorSetLayout descriptorSetLayout = context.getShadowMapDescriptorSetLayout();

	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate shadow map descriptor sets!");
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_uniformBuffers[i]->getBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(ShadowMapUniformBufferObject);

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSets[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
	}
}

std::unique_ptr<ShaderResourceManager> ShaderResourceManager::createShadowCubeMapShaderResourceManager()
{
	std::unique_ptr<ShaderResourceManager> shaderResourceManager =
		std::unique_ptr<ShaderResourceManager>(new ShaderResourceManager());
	shaderResourceManager->initShadowCubeMapShaderResourceManager();
	return shaderResourceManager;
}

void ShaderResourceManager::initShadowCubeMapShaderResourceManager()
{
	createShadowCubeMapUniformBuffers();
	createShadowCubeMapDescriptorSets();
}

void ShaderResourceManager::createShadowCubeMapUniformBuffers()
{
	VkDeviceSize uniformBufferSize = sizeof(ShadowCubeMapUniformBufferObject);
	VkDeviceSize layerIndexBufferSize = sizeof(ShadowCubeMapLayerIndex);
	m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	m_layerIndexUniformBuffers.resize(MAX_FRAMES_IN_FLIGHT * 6);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		m_uniformBuffers[i] = UniformBuffer::createUniformBuffer(uniformBufferSize);
		for (size_t j = 0; j < 6; j++)
		{
			m_layerIndexUniformBuffers[i * 6 + j] = UniformBuffer::createUniformBuffer(layerIndexBufferSize);
		}
	}
}

void ShaderResourceManager::createShadowCubeMapDescriptorSets()
{
	auto &context = VulkanContext::getContext();
	VkDevice device = context.getDevice();
	VkDescriptorPool descriptorPool = context.getDescriptorPool();
	VkDescriptorSetLayout descriptorSetLayout = context.getShadowCubeMapDescriptorSetLayout();

	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT * 6, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * 6);
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(MAX_FRAMES_IN_FLIGHT * 6);
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate shadow cube map descriptor sets!");
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		for (size_t j = 0; j < 6; j++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = m_uniformBuffers[i]->getBuffer();
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(ShadowCubeMapUniformBufferObject);

			VkDescriptorBufferInfo layerIndexBufferInfo{};
			layerIndexBufferInfo.buffer = m_layerIndexUniformBuffers[i * 6 + j]->getBuffer();
			layerIndexBufferInfo.offset = 0;
			layerIndexBufferInfo.range = sizeof(ShadowCubeMapLayerIndex);

			std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = descriptorSets[i * 6 + j];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &bufferInfo;

			descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[1].dstSet = descriptorSets[i * 6 + j];
			descriptorWrites[1].dstBinding = 1;
			descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[1].descriptorCount = 1;
			descriptorWrites[1].pBufferInfo = &layerIndexBufferInfo;

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
								   nullptr);
		}
	}
}

std::unique_ptr<ShaderResourceManager> ShaderResourceManager::createViewPortShaderResourceManager(
	VkDescriptorSetLayout descriptorSetLayout, VkImageView viewPortImageView, VkSampler viewPortSampler)
{
	std::unique_ptr<ShaderResourceManager> shaderResourceManager =
		std::unique_ptr<ShaderResourceManager>(new ShaderResourceManager());
	shaderResourceManager->initViewPortShaderResourceManager(descriptorSetLayout, viewPortImageView, viewPortSampler);
	return shaderResourceManager;
}

void ShaderResourceManager::initViewPortShaderResourceManager(VkDescriptorSetLayout descriptorSetLayout,
															  VkImageView viewPortImageView, VkSampler viewPortSampler)
{
	createViewPortDescriptorSets(descriptorSetLayout, viewPortImageView, viewPortSampler);
}

void ShaderResourceManager::createViewPortDescriptorSets(VkDescriptorSetLayout descriptorSetLayout,
														 VkImageView viewPortImageView, VkSampler viewPortSampler)
{
	auto &context = VulkanContext::getContext();
	VkDevice device = context.getDevice();
	VkDescriptorPool descriptorPool = context.getDescriptorPool();

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptorSetLayout;

	descriptorSets.resize(1);
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)

	{
		throw std::runtime_error("Failed to allocate view port descriptor set!");
	}

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageView = viewPortImageView;
	imageInfo.sampler = viewPortSampler;
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSets[0];
	descriptorWrite.dstBinding = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount = 1;

	descriptorWrite.pImageInfo = &imageInfo;

	vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
}

std::unique_ptr<ShaderResourceManager> ShaderResourceManager::createSphericalMapShaderResourceManager(
	VkDescriptorSetLayout descriptorSetLayout, VkImageView sphericalMapImageView, VkSampler sphericalMapSampler)
{
	std::unique_ptr<ShaderResourceManager> shaderResourceManager =
		std::unique_ptr<ShaderResourceManager>(new ShaderResourceManager());
	shaderResourceManager->initSphericalMapShaderResourceManager(descriptorSetLayout, sphericalMapImageView,
																 sphericalMapSampler);
	return shaderResourceManager;
}

void ShaderResourceManager::initSphericalMapShaderResourceManager(VkDescriptorSetLayout descriptorSetLayout,
																  VkImageView sphericalMapImageView,
																  VkSampler sphericalMapSampler)
{
	createSphericalMapUniformBuffers();
	createSphericalMapDescriptorSets(descriptorSetLayout, sphericalMapImageView, sphericalMapSampler);
}

void ShaderResourceManager::createSphericalMapUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(SphericalMapUniformBufferObject);
	m_uniformBuffers.resize(6);
	for (size_t i = 0; i < 6; i++)
	{
		m_uniformBuffers[i] = UniformBuffer::createUniformBuffer(bufferSize);
	}
}

void ShaderResourceManager::createSphericalMapDescriptorSets(VkDescriptorSetLayout descriptorSetLayout,
															 VkImageView sphericalMapImageView,
															 VkSampler sphericalMapSampler)
{
	auto &context = VulkanContext::getContext();
	VkDevice device = context.getDevice();
	VkDescriptorPool descriptorPool = context.getDescriptorPool();

	// **🔹 Descriptor Set 6개 할당 (각 큐브맵 레이어별)**
	std::vector<VkDescriptorSetLayout> layouts(6, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(6);
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate spherical map descriptor sets!");
	}

	// **🔹 Descriptor Set 업데이트**
	for (size_t i = 0; i < 6; i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_uniformBuffers[i]->getBuffer(); // **각 레이어에 맞는 UBO 설정**
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(SphericalMapUniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageView = sphericalMapImageView;
		imageInfo.sampler = sphericalMapSampler;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
							   nullptr);
	}
}

std::unique_ptr<ShaderResourceManager> ShaderResourceManager::createBackgroundShaderResourceManager(
	VkDescriptorSetLayout descriptorSetLayout, VkImageView skyboxImageView, VkSampler skyboxSampler)
{
	std::unique_ptr<ShaderResourceManager> shaderResourceManager =
		std::unique_ptr<ShaderResourceManager>(new ShaderResourceManager());
	shaderResourceManager->initBackgroundShaderResourceManager(descriptorSetLayout, skyboxImageView, skyboxSampler);
	return shaderResourceManager;
}

void ShaderResourceManager::initBackgroundShaderResourceManager(VkDescriptorSetLayout descriptorSetLayout,
																VkImageView skyboxImageView, VkSampler skyboxSampler)
{
	createBackgroundUniformBuffers();
	createBackgroundDescriptorSets(descriptorSetLayout, skyboxImageView, skyboxSampler);
}

void ShaderResourceManager::createBackgroundUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(BackgroundUniformBufferObject);
	m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		m_uniformBuffers[i] = UniformBuffer::createUniformBuffer(bufferSize);
	}
}

void ShaderResourceManager::createBackgroundDescriptorSets(VkDescriptorSetLayout descriptorSetLayout,
														   VkImageView skyboxImageView, VkSampler skyboxSampler)
{
	auto &context = VulkanContext::getContext();
	VkDevice device = context.getDevice();
	VkDescriptorPool descriptorPool = context.getDescriptorPool();

	std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate Background descriptor sets!");
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_uniformBuffers[i]->getBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(BackgroundUniformBufferObject);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageView = skyboxImageView;
		imageInfo.sampler = skyboxSampler;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0,
							   nullptr);
	}
}

} // namespace ale