#include "Renderer/VulkanUtil.h"

namespace ale
{
void VulkanUtil::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
							 VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
							 VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory)
{

	auto &context = VulkanContext::getContext();
	auto device = context.getDevice();

	// 이미지 객체를 만드는데 사용되는 구조체
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D; // 이미지의 차원을 설정
	imageInfo.extent.width = width;			// 이미지의 너비 지정
	imageInfo.extent.height = height;		// 이미지의 높이 지정
	imageInfo.extent.depth = 1; // 이미지의 깊이 지정 (2D 이미지의 경우 depth는 1로 지정해야 함)
	imageInfo.mipLevels = mipLevels; // 생성할 mipLevel의 개수 지정
	imageInfo.arrayLayers = 1;		 // 생성할 이미지 레이어 수 (큐브맵의 경우 6개 생성)
	imageInfo.format = format; // 이미지의 포맷을 지정하며, 채널 구성과 각 채널의 비트 수를 정의
	imageInfo.tiling = tiling; // 이미지를 GPU 메모리에 배치할 때 메모리 레이아웃을 결정하는 설정 (CPU에서도 접근
							   // 가능하게 할꺼냐, GPU에만 접근 가능하게 최적화 할거냐 결정)
	imageInfo.initialLayout =
		VK_IMAGE_LAYOUT_UNDEFINED; // 이미지 초기 레이아웃 설정 (이미지가 메모리에 배치될 때 초기 상태를 정의)
	imageInfo.usage = usage;						   // 이미지의 사용 용도 결정
	imageInfo.samples = numSamples;					   // 멀티 샘플링을 위한 샘플 개수
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // 이미지의 큐 공유 모드 설정 (VK_SHARING_MODE_EXCLUSIVE: 한 번에
													   // 하나의 큐 패밀리에서만 접근 가능한 단일 큐 모드)

	// 이미지 객체 생성
	if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create image!");
	}

	// 이미지에 필요한 메모리 요구 사항을 조회
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	// 메모리 할당을 위한 구조체
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;										// 메모리 크기 설정
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties); // 메모리 유형과 속성 설정

	// 이미지를 위한 메모리 할당
	if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate image memory!");
	}

	// 이미지에 할당한 메모리 바인딩
	vkBindImageMemory(device, image, imageMemory, 0);
}

/*
	GPU와 buffer가 호환되는 메모리 유형중 properties에 해당하는 속성들을 갖는 메모리 유형 찾기
*/
uint32_t VulkanUtil::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	auto &context = VulkanContext::getContext();
	auto physicalDevice = context.getPhysicalDevice();

	// GPU에서 사용한 메모리 유형을 가져온다.
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		// typeFilter & (1 << i) : GPU의 메모리 유형중 버퍼와 호환되는 것인지 판단
		// memProperties.memoryTypes[i].propertyFlags & properties : GPU 메모리 유형의 속성이 properties와 일치하는지
		// 판단
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			// 해당 메모리 유형 반환
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

// 이미지 뷰 생성
VkImageView VulkanUtil::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
										uint32_t mipLevels)
{
	auto &context = VulkanContext::getContext();
	auto device = context.getDevice();

	// 이미지 뷰 정보 생성
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;								// 이미지 핸들
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;			// 이미지 타입
	viewInfo.format = format;							// 이미지 포맷
	viewInfo.subresourceRange.aspectMask = aspectFlags; // 이미지 형식 결정 (color / depth / stencil 등)
	viewInfo.subresourceRange.baseMipLevel = 0;			// 렌더링할 mipmap 단계 설정
	viewInfo.subresourceRange.levelCount =
		mipLevels; // baseMipLevel 기준으로 몇 개의 MipLevel을 더 사용할지 설정 (실제 mipmap 만드는 건 따로 해줘야함)
	viewInfo.subresourceRange.baseArrayLayer = 0; // ImageView가 참조하는 이미지 레이어의 시작 위치 정의
	viewInfo.subresourceRange.layerCount = 1;	  // 스왑 체인에서 설정한 이미지 레이어 개수

	// 이미지 뷰 생성
	VkImageView imageView;
	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create image view!");
	}

	return imageView;
}


void VulkanUtil::createCubeMapImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples,
                                 VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                                 VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory) {
	auto &context = VulkanContext::getContext();
    auto device = context.getDevice();

    // 큐브맵 이미지 객체를 만드는데 사용되는 구조체
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D; // 큐브맵은 2D 배열로 표현됩니다.
    imageInfo.extent.width = width;         // 이미지의 너비 지정
    imageInfo.extent.height = height;       // 이미지의 높이 지정
    imageInfo.extent.depth = 1;             // 이미지의 깊이는 1
    imageInfo.mipLevels = mipLevels;        // 생성할 mipLevel의 개수 지정
    imageInfo.arrayLayers = 6;              // 큐브맵의 경우 6개의 레이어 필요
    imageInfo.format = format;              // 이미지 포맷 지정
    imageInfo.tiling = tiling;              // 메모리 레이아웃
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // 초기 레이아웃
    imageInfo.usage = usage;                // 이미지의 용도 설정
    imageInfo.samples = numSamples;         // 샘플 개수
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // 큐 공유 모드
    imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT; // 큐브맵 호환 플래그 추가

    // 이미지 객체 생성
    if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create cube image!");
    }

    // 이미지에 필요한 메모리 요구 사항 조회
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device, image, &memRequirements);

    // 메모리 할당을 위한 구조체
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size; // 메모리 크기
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties); // 메모리 유형 설정

    // 이미지를 위한 메모리 할당
    if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate cube image memory!");
    }

    // 이미지에 메모리 바인딩
    vkBindImageMemory(device, image, imageMemory, 0);
}

VkImageView VulkanUtil::createCubeMapImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags,
											uint32_t mipLevels) {
	auto &context = VulkanContext::getContext();
    auto device = context.getDevice();

    // 큐브 맵 이미지 뷰 정보 생성
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;                              // 이미지 핸들
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;         // 큐브 맵 타입
    viewInfo.format = format;                            // 이미지 포맷
    viewInfo.subresourceRange.aspectMask = aspectFlags;  // 이미지 형식 결정 (color / depth / stencil 등)
    viewInfo.subresourceRange.baseMipLevel = 0;          // 렌더링할 mipmap 단계 설정
    viewInfo.subresourceRange.levelCount = mipLevels;    // 사용 가능한 모든 MipLevel
    viewInfo.subresourceRange.baseArrayLayer = 0;        // 첫 번째 레이어부터 시작
    viewInfo.subresourceRange.layerCount = 6;            // 큐브 맵은 6개의 레이어를 사용

    // 큐브 맵 이미지 뷰 생성
    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create cube image view!");
    }

    return imageView;
}

// depth image의 format 설정
VkFormat VulkanUtil::findDepthFormat()
{
	return findSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
							   VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

// Vulkan의 특정 format에 대해 GPU가 tiling의 features를 지원하는지 확인
VkFormat VulkanUtil::findSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling,
										 VkFormatFeatureFlags features)
{
	auto &context = VulkanContext::getContext();
	VkPhysicalDevice physicalDevice = context.getPhysicalDevice();

	// format 들에 대해 GPU가 tiling과 features를 지원하는지 확인
	for (VkFormat format : candidates)
	{
		// GPU가 format에 대해 지원하는 특성 가져오는 함수
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		// GPU가 지원하는 특성과 비교
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{ // VK_IMAGE_TILING_LINEAR의 특성 비교
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
		{ // VK_IMAGE_TILING_OPTIMAL의 특성 비교
			return format;
		}
	}
	throw std::runtime_error("failed to find supported format!");
}

// shader 파일인 SPIR-V 파일을 바이너리 형태로 읽어오는 함수
std::vector<char> VulkanUtil::readFile(const std::string &filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}
} // namespace ale