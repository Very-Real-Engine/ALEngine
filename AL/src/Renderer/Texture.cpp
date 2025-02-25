#include "Renderer/Texture.h"

namespace ale
{

std::shared_ptr<Texture> Texture::createTexture(std::string path, bool flipVertically)
{
	std::shared_ptr<Texture> texture = std::shared_ptr<Texture>(new Texture());
	texture->initTexture(path, flipVertically);
	return texture;
}

std::shared_ptr<Texture> Texture::createMaterialTexture(std::string path, bool flipVertically)
{
	std::shared_ptr<Texture> texture = std::shared_ptr<Texture>(new Texture());
	texture->initMaterialTexture(path, flipVertically);
	return texture;
}

void Texture::cleanup()
{
	auto &context = VulkanContext::getContext();
	auto device = context.getDevice();

	if (textureSampler != VK_NULL_HANDLE)
	{
		vkDestroySampler(device, textureSampler, nullptr);
		textureSampler = VK_NULL_HANDLE;
	}
	if (textureImageView != VK_NULL_HANDLE)
	{
		vkDestroyImageView(device, textureImageView, nullptr);
		textureImageView = VK_NULL_HANDLE;
	}
	m_imageBuffer->cleanup();
}

void Texture::initTexture(std::string path, bool flipVertically)
{
	loadTexture(path, flipVertically);
}

void Texture::initMaterialTexture(std::string path, bool flipVertically)
{
	loadMaterialTexture(path, flipVertically);
}

void Texture::loadTexture(std::string path, bool flipVertically)
{
	m_imageBuffer = ImageBuffer::createImageBuffer(path, flipVertically);
	if (!m_imageBuffer)
	{
		m_imageBuffer = ImageBuffer::createDefaultImageBuffer(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		mipLevels = m_imageBuffer->getMipLevels();
		createDefaultTextureImageView();
		createDefaultTextureSampler();
	}
	else
	{
		mipLevels = m_imageBuffer->getMipLevels();
		createTextureImageView();
		createTextureSampler();
	}
}

void Texture::loadMaterialTexture(std::string path, bool flipVertically)
{
	m_imageBuffer = ImageBuffer::createMaterialImageBuffer(path, flipVertically);
	if (!m_imageBuffer)
	{
		m_imageBuffer = ImageBuffer::createDefaultImageBuffer(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		mipLevels = m_imageBuffer->getMipLevels();
		createDefaultTextureImageView();
		createDefaultTextureSampler();
	}
	else
	{
		mipLevels = m_imageBuffer->getMipLevels();
		createMaterialTextureImageView();
		createTextureSampler();
	}
}

// 텍스처 이미지 뷰 생성
void Texture::createTextureImageView()
{
	// SRGB 총 4바이트 포맷으로 된 이미지 뷰 생성
	textureImageView = VulkanUtil::createImageView(m_imageBuffer->getImage(), VK_FORMAT_R8G8B8A8_SRGB,
												   VK_IMAGE_ASPECT_COLOR_BIT, m_imageBuffer->getMipLevels());
}

void Texture::createMaterialTextureImageView()
{
	textureImageView = VulkanUtil::createImageView(m_imageBuffer->getImage(), VK_FORMAT_R8G8B8A8_UNORM,
												   VK_IMAGE_ASPECT_COLOR_BIT, m_imageBuffer->getMipLevels());
}

// 텍스처를 위한 샘플러 생성 함수
void Texture::createTextureSampler()
{
	auto &context = VulkanContext::getContext();
	auto device = context.getDevice();
	auto physicalDevice = context.getPhysicalDevice();

	// GPU의 속성 정보를 가져오는 함수
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	// 샘플러 생성시 필요한 구조체
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;				   // 확대시 필터링 적용 설정 (현재 선형 보간 필터링 적용)
	samplerInfo.minFilter = VK_FILTER_LINEAR;				   // 축소시 필터링 적용 설정 (현재 선형 보간 필터링 적용)
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT; // 텍스처 좌표계의 U축(너비)에서 범위를 벗어난 경우
															   // 래핑 모드 설정 (현재 반복 설정)
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT; // 텍스처 좌표계의 V축(높이)에서 범위를 벗어난 경우
															   // 래핑 모드 설정 (현재 반복 설정)
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT; // 텍스처 좌표계의 W축(깊이)에서 범위를 벗어난 경우
															   // 래핑 모드 설정 (현재 반복 설정)
	samplerInfo.anisotropyEnable =
		VK_TRUE; // 이방성 필터링 적용 여부 설정 (경사진 곳이나 먼 곳의 샘플을 늘려 좀 더 정확한 값을 가져오는 방법)
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; // GPU가 지원하는 최대의 이방성 필터링 제한 설정
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // 래핑 모드가 VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER
																// 일때 텍스처 외부 색 지정
	samplerInfo.unnormalizedCoordinates =
		VK_FALSE; // VK_TRUE로 설정시 텍스처 좌표가 0 ~ 1의 정규화된 좌표가 아닌 실제 텍셀의 좌표 범위로 바뀜
	samplerInfo.compareEnable =
		VK_FALSE; // 비교 연산 사용할지 결정 (보통 쉐도우 맵같은 경우 깊이 비교 샘플링에서 사용됨)
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;			// 비교 연산에 사용할 연산 지정
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // mipmap 사용시 mipmap 간 보간 방식 결정 (현재 선형 보간)
	samplerInfo.minLod = 0.0f; // 최소 level을 0으로 설정 (가장 높은 해상도의 mipmap 을 사용가능하게 허용)
	samplerInfo.maxLod = static_cast<float>(
		m_imageBuffer->getMipLevels()); // 최대 level을 mipLevel로 설정 (VK_LOD_CLAMP_NONE 설정시 제한 해제)
	samplerInfo.mipLodBias =
		0.0f; // Mipmap 레벨 오프셋(Bias)을 설정
			  // Mipmap을 일부러 더 높은(더 큰) 레벨로 사용하거나 낮은(더 작은) 레벨로 사용하고 싶을 때 사용.

	// 샘플러 생성
	if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture sampler!");
	}
}

std::shared_ptr<Texture> Texture::createDefaultTexture(glm::vec4 color)
{
	std::shared_ptr<Texture> texture = std::shared_ptr<Texture>(new Texture());
	texture->initDefaultTexture(color);
	return texture;
}

void Texture::initDefaultTexture(glm::vec4 color)
{
	m_imageBuffer = ImageBuffer::createDefaultImageBuffer(color);
	mipLevels = m_imageBuffer->getMipLevels();
	createDefaultTextureImageView();
	createDefaultTextureSampler();
}

void Texture::createDefaultTextureImageView()
{
	auto &context = VulkanContext::getContext();
	auto device = context.getDevice();

	textureImageView = VulkanUtil::createImageView(m_imageBuffer->getImage(),
												   VK_FORMAT_R8G8B8A8_UNORM, // Default Texture는 UNORM 포맷 사용
												   VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);

	if (!textureImageView)
	{
		throw std::runtime_error("Failed to create default texture image view!");
	}
}

void Texture::createDefaultTextureSampler()
{
	auto &context = VulkanContext::getContext();
	auto device = context.getDevice();
	auto physicalDevice = context.getPhysicalDevice();

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(mipLevels);
	samplerInfo.mipLodBias = 0.0f;

	if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create default texture sampler!");
	}
}

std::shared_ptr<Texture> Texture::createDefaultSingleChannelTexture(float value)
{
	std::shared_ptr<Texture> texture = std::shared_ptr<Texture>(new Texture());
	texture->initDefaultSingleChannelTexture(value);
	return texture;
}

void Texture::initDefaultSingleChannelTexture(float value)
{
	m_imageBuffer = ImageBuffer::createDefaultSingleChannelImageBuffer(value);
	mipLevels = m_imageBuffer->getMipLevels();
	createDefaultSingleChannelTextureImageView();
	createDefaultSingleChannelTextureSampler();
}

void Texture::createDefaultSingleChannelTextureImageView()
{
	auto &context = VulkanContext::getContext();
	auto device = context.getDevice();

	textureImageView = VulkanUtil::createImageView(m_imageBuffer->getImage(),
												   VK_FORMAT_R8_UNORM, // 단일 채널 포맷
												   VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);

	if (!textureImageView)
	{
		throw std::runtime_error("Failed to create default single channel texture image view!");
	}
}

void Texture::createDefaultSingleChannelTextureSampler()
{
	auto &context = VulkanContext::getContext();
	auto device = context.getDevice();
	auto physicalDevice = context.getPhysicalDevice();

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast<float>(mipLevels);
	samplerInfo.mipLodBias = 0.0f;

	if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create default single channel texture sampler!");
	}
}

VkSampler Texture::createShadowMapSampler()
{
	auto &context = VulkanContext::getContext();
	auto device = context.getDevice();
	auto physicalDevice = context.getPhysicalDevice();

	// GPU의 속성 정보를 가져오는 구조체
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	// 샘플러 생성 정보
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;							// 확대 시 선형 필터링
	samplerInfo.minFilter = VK_FILTER_LINEAR;							// 축소 시 선형 필터링
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER; // U축 클램핑
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER; // V축 클램핑
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER; // W축 클램핑
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;		// 경계를 흰색으로 설정
	samplerInfo.anisotropyEnable = VK_FALSE;							// 이방성 필터링 비활성화
	samplerInfo.maxAnisotropy = 1.0f;									// 이방성 필터링 값 (비활성화 상태)
	samplerInfo.compareEnable = VK_TRUE;								// 비교 샘플링 활성화 (쉐도우 맵)
	samplerInfo.compareOp = VK_COMPARE_OP_LESS;							// 깊이 비교 연산 설정
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;				// Mipmap 보간 모드
	samplerInfo.minLod = 0.0f;											// 최소 LOD
	samplerInfo.maxLod = 0.0f;											// 최대 LOD (Mipmap 미사용)
	samplerInfo.mipLodBias = 0.0f;										// Mipmap Bias 비활성화
	samplerInfo.unnormalizedCoordinates = VK_FALSE;						// 정규화된 텍스처 좌표 사용

	VkSampler depthMapSampler;
	// 샘플러 생성
	if (vkCreateSampler(device, &samplerInfo, nullptr, &depthMapSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create shadow map sampler in Texture::createShadowMapSampler");
	}
	return depthMapSampler;
}

VkSampler Texture::createShadowCubeMapSampler()
{
	auto &context = VulkanContext::getContext();
	auto device = context.getDevice();
	auto physicalDevice = context.getPhysicalDevice();

	// GPU의 속성 정보를 가져오는 구조체
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	// 큐브맵 샘플러 생성 정보
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;							// 확대 시 선형 필터링
	samplerInfo.minFilter = VK_FILTER_LINEAR;							// 축소 시 선형 필터링
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;	// U축 클램핑
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;	// V축 클램핑
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;	// W축 클램핑
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;		// 경계 색상
	samplerInfo.anisotropyEnable = VK_TRUE;								// 이방성 필터링 활성화
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; // 디바이스에서 지원하는 최대 이방성 값
	samplerInfo.compareEnable = VK_TRUE;								// 깊이 비교 샘플링 활성화
	samplerInfo.compareOp = VK_COMPARE_OP_LESS;							// 깊이 비교 연산
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;				// Mipmap 보간 모드
	samplerInfo.minLod = 0.0f;											// 최소 LOD
	samplerInfo.maxLod = 0.0f;											// 최대 LOD (Mipmap 미사용)
	samplerInfo.mipLodBias = 0.0f;										// Mipmap Bias 비활성화
	samplerInfo.unnormalizedCoordinates = VK_FALSE;						// 정규화된 텍스처 좌표 사용

	VkSampler cubeMapSampler;
	if (vkCreateSampler(device, &samplerInfo, nullptr, &cubeMapSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create cube map sampler in Texture::createCubeMapSampler");
	}

	return cubeMapSampler;
}

VkSampler Texture::createSphericalMapSampler()
{
	auto &context = VulkanContext::getContext();
	auto device = context.getDevice();
	auto physicalDevice = context.getPhysicalDevice();

	// **GPU 속성 정보 가져오기 (최대 이방성 필터링 지원 값)**
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	// **Spherical Map 샘플러 생성 정보**
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;							// 확대 시 선형 필터링
	samplerInfo.minFilter = VK_FILTER_LINEAR;							// 축소 시 선형 필터링
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;	// U축 클램핑
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;	// V축 클램핑
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;	// W축 클램핑
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;		// 경계 색상
	samplerInfo.anisotropyEnable = VK_TRUE;								// 이방성 필터링 활성화
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; // 최대 이방성 값
	samplerInfo.compareEnable = VK_FALSE;								// 깊이 비교 샘플링 비활성화 (Shadow Map이 아님)
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;						// 비교 연산은 사용하지 않음
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;				// Mipmap 보간 모드
	samplerInfo.minLod = 0.0f;											// 최소 LOD
	samplerInfo.maxLod = VK_LOD_CLAMP_NONE;								// 최대 LOD
	samplerInfo.mipLodBias = 0.0f;										// Mipmap Bias 비활성화
	samplerInfo.unnormalizedCoordinates = VK_FALSE;						// 정규화된 텍스처 좌표 사용

	VkSampler sphericalMapSampler;
	if (vkCreateSampler(device, &samplerInfo, nullptr, &sphericalMapSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create spherical map sampler in Texture::createSphericalMapSampler");
	}
	return sphericalMapSampler;
}

VkSampler Texture::createBackgroundSampler()
{
	auto &context = VulkanContext::getContext();
	auto device = context.getDevice();
	auto physicalDevice = context.getPhysicalDevice();

	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physicalDevice, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;							// 확대 시 선형 필터링
	samplerInfo.minFilter = VK_FILTER_LINEAR;							// 축소 시 선형 필터링
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;	// U축 클램핑
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;	// V축 클램핑
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;	// W축 클램핑
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;		// 경계 색상
	samplerInfo.anisotropyEnable = VK_TRUE;								// 이방성 필터링 활성화
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy; // 디바이스에서 지원하는 최대 이방성 값
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;				// Mipmap 보간 모드
	samplerInfo.minLod = 0.0f;											// 최소 LOD
	samplerInfo.maxLod = VK_LOD_CLAMP_NONE;								// 최대 LOD
	samplerInfo.mipLodBias = 0.0f;										// Mipmap Bias 비활성화
	samplerInfo.unnormalizedCoordinates = VK_FALSE;						// 정규화된 텍스처 좌표 사용

	VkSampler backgroundSampler;
	if (vkCreateSampler(device, &samplerInfo, nullptr, &backgroundSampler) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create background sampler!");
	}

	return backgroundSampler;
}

std::shared_ptr<Texture> Texture::createTextureFromMemory(const aiTexture *aiTexture)
{
	std::shared_ptr<Texture> texture = std::shared_ptr<Texture>(new Texture());
	texture->initTextureFromMemory(aiTexture);
	return texture;
}

void Texture::initTextureFromMemory(const aiTexture *aiTexture)
{
	m_imageBuffer = ImageBuffer::createImageBufferFromMemory(aiTexture);
	mipLevels = m_imageBuffer->getMipLevels();
	createMaterialTextureImageView();
	createTextureSampler();
}

std::shared_ptr<Texture> Texture::createHDRTexture(std::string path)
{
	std::shared_ptr<Texture> texture = std::shared_ptr<Texture>(new Texture());
	texture->initHDRTexture(path);
	return texture;
}

void Texture::initHDRTexture(std::string path)
{
	m_imageBuffer = ImageBuffer::createHDRImageBuffer(path);
	if (!m_imageBuffer)
	{
		m_imageBuffer = ImageBuffer::createDefaultImageBuffer(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
		mipLevels = m_imageBuffer->getMipLevels();
		createDefaultTextureImageView();
		createDefaultTextureSampler();
	}
	else
	{
		mipLevels = m_imageBuffer->getMipLevels();
		createHDRTextureImageView();
		createTextureSampler();
	}
}

void Texture::createHDRTextureImageView()
{
	auto &context = VulkanContext::getContext();
	auto device = context.getDevice();

	textureImageView = VulkanUtil::createImageView(m_imageBuffer->getImage(), VK_FORMAT_R32G32B32A32_SFLOAT,
												   VK_IMAGE_ASPECT_COLOR_BIT, mipLevels);
}

} // namespace ale