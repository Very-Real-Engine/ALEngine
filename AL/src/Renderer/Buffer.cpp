#include "Renderer/Buffer.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace ale
{
void Buffer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
						  VkBuffer &buffer, VkDeviceMemory &bufferMemory)
{
	// 버퍼 객체를 생성하기 위한 구조체 (GPU 메모리에 데이터 저장 공간을 할당하는 데 필요한 설정을 정의)
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;	  // 버퍼의 크기 지정
	bufferInfo.usage = usage; // 버퍼의 용도 지정
	bufferInfo.sharingMode =
		VK_SHARING_MODE_EXCLUSIVE; // 버퍼를 하나의 큐 패밀리에서 쓸지, 여러 큐 패밀리에서 공유할지 설정 (현재 단일 큐
								   // 패밀리에서 사용하도록 설정) 여러 큐 패밀리에서 공유하는 모드 사용시 추가 설정 필요
	// [버퍼 생성]
	// 버퍼를 생성하지만 할당은 안되어있는 상태로 만들어짐
	if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create buffer!");
	}

	// [버퍼에 메모리 할당]
	// 메모리 할당 요구사항 조회
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

	// 메모리 할당을 위한 구조체
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size; // 할당할 메모리 크기
	// 메모리 요구사항 설정 (GPU 메모리 유형 중 buffer와 호환되고 properties 속성들과 일치하는 것 찾아 저장)
	// 메모리 유형 - GPU 메모리는 구역마다 유형이 다르다. (memoryTypeBits는 buffer가 호환되는 GPU의 메모리 유형이 전부
	// 담겨있음) 메모리 유형의 속성 - 메모리 유형마다 특성을 가지고 있음
	allocInfo.memoryTypeIndex = VulkanUtil::findMemoryType(memRequirements.memoryTypeBits, properties);

	// 버퍼 메모리 할당
	if (vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	// 버퍼 객체에 할당된 메모리를 바인딩 (4번째 매개변수는 할당할 메모리의 offset)
	vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
}

// srcBuffer 에서 dstBuffer 로 데이터 복사
void Buffer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferCopy copyRegion{}; // 복사할 버퍼 영역을 지정 (크기, src 와 dst의 시작 offset 등)
	copyRegion.size = size;	   // 복사할 버퍼 크기 설정
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion); // 커맨드 버퍼에 복사 명령 기록

	endSingleTimeCommands(commandBuffer);
}

// 한 번만 실행할 커맨드 버퍼 생성 및 기록 시작
VkCommandBuffer Buffer::beginSingleTimeCommands()
{
	// 커맨드 버퍼 할당을 위한 구조체
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level =
		VK_COMMAND_BUFFER_LEVEL_PRIMARY;   // PRIMARY LEVEL 등록 (해당 커맨드 버퍼가 큐에 단독으로 제출될 수 있음)
	allocInfo.commandPool = m_commandPool; // 커맨드 풀 지정
	allocInfo.commandBufferCount = 1;	   // 커맨드 버퍼 개수 지정

	// 커맨드 버퍼 생성
	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

	// 커맨드 버퍼 기록을 위한 정보 객체
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // 커맨드 버퍼를 1번만 제출

	// GPU에 필요한 작업을 모두 커맨드 버퍼에 기록하기 시작
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

// 한 번만 실행할 커맨드 버퍼 기록 중지 및 큐에 커맨드 버퍼 제출
void Buffer::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	// 커맨드 버퍼 기록 중지
	vkEndCommandBuffer(commandBuffer);

	// 복사 커맨드 버퍼 제출 정보 객체 생성
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;			 // 커맨드 버퍼 개수
	submitInfo.pCommandBuffers = &commandBuffer; // 커맨드 버퍼 등록

	vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE); // 커맨드 버퍼 큐에 제출
	vkQueueWaitIdle(m_graphicsQueue);								// 그래픽스 큐 작업 종료 대기

	vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer); // 커맨드 버퍼 제거
}

std::unique_ptr<VertexBuffer> VertexBuffer::createVertexBuffer(std::vector<Vertex> &vertices)
{
	std::unique_ptr<VertexBuffer> vertexBuffer = std::unique_ptr<VertexBuffer>(new VertexBuffer());
	vertexBuffer->initVertexBuffer(vertices);
	return vertexBuffer;
}

void VertexBuffer::cleanup()
{
	if (m_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(m_device, m_buffer, nullptr);
		m_buffer = VK_NULL_HANDLE;
	}
	if (m_bufferMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(m_device, m_bufferMemory, nullptr);
		m_bufferMemory = VK_NULL_HANDLE;
	}
}

void VertexBuffer::bind(VkCommandBuffer commandBuffer)
{
	VkBuffer buffers[] = {m_buffer};
	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void VertexBuffer::initVertexBuffer(std::vector<Vertex> &vertices)
{
	auto &context = VulkanContext::getContext();
	m_device = context.getDevice();
	m_physicalDevice = context.getPhysicalDevice();
	m_commandPool = context.getCommandPool();
	m_graphicsQueue = context.getGraphicsQueue();

	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
				 stagingBufferMemory);

	void *data;
	vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);

	vkUnmapMemory(m_device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_buffer, m_bufferMemory);
	copyBuffer(stagingBuffer, m_buffer, bufferSize);

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

std::unique_ptr<IndexBuffer> IndexBuffer::createIndexBuffer(std::vector<uint32_t> &indices)
{
	std::unique_ptr<IndexBuffer> indexBuffer = std::unique_ptr<IndexBuffer>(new IndexBuffer());
	indexBuffer->initIndexBuffer(indices);
	return indexBuffer;
}

void IndexBuffer::cleanup()
{
	if (m_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(m_device, m_buffer, nullptr);
		m_buffer = VK_NULL_HANDLE;
	}
	if (m_bufferMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(m_device, m_bufferMemory, nullptr);
		m_bufferMemory = VK_NULL_HANDLE;
	}
}

void IndexBuffer::bind(VkCommandBuffer commandBuffer)
{
	vkCmdBindIndexBuffer(commandBuffer, m_buffer, 0, VK_INDEX_TYPE_UINT32);
}

void IndexBuffer::initIndexBuffer(std::vector<uint32_t> &indices)
{
	auto &context = VulkanContext::getContext();
	m_device = context.getDevice();
	m_physicalDevice = context.getPhysicalDevice();
	m_commandPool = context.getCommandPool();
	m_graphicsQueue = context.getGraphicsQueue();

	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
	m_indexCount = static_cast<uint32_t>(indices.size());

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
				 stagingBufferMemory);

	void *data;
	vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(m_device, stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_buffer, m_bufferMemory);
	copyBuffer(stagingBuffer, m_buffer, bufferSize);

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

std::unique_ptr<ImageBuffer> ImageBuffer::createImageBuffer(std::string path, bool flipVertically)
{
	std::unique_ptr<ImageBuffer> imageBuffer = std::unique_ptr<ImageBuffer>(new ImageBuffer());
	if (!imageBuffer->initImageBuffer(path, flipVertically))
	{
		return nullptr;
	}
	return imageBuffer;
}

std::unique_ptr<ImageBuffer> ImageBuffer::createMaterialImageBuffer(std::string path, bool flipVertically)
{
	std::unique_ptr<ImageBuffer> imageBuffer = std::unique_ptr<ImageBuffer>(new ImageBuffer());
	if (!imageBuffer->initMaterialImageBuffer(path, flipVertically))
	{
		return nullptr;
	}
	return imageBuffer;
}

std::unique_ptr<ImageBuffer> ImageBuffer::createImageBufferFromMemory(const aiTexture *texture)
{
	std::unique_ptr<ImageBuffer> imageBuffer = std::unique_ptr<ImageBuffer>(new ImageBuffer());
	imageBuffer->initImageBufferFromMemory(texture);
	return imageBuffer;
}

void ImageBuffer::cleanup()
{
	if (textureImage != VK_NULL_HANDLE)
	{
		vkDestroyImage(m_device, textureImage, nullptr);
		textureImage = VK_NULL_HANDLE;
	}
	if (textureImageMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(m_device, textureImageMemory, nullptr);
		textureImageMemory = VK_NULL_HANDLE;
	}
}

bool ImageBuffer::initImageBuffer(std::string path, bool flipVertically)
{
	auto &context = VulkanContext::getContext();
	m_device = context.getDevice();
	m_physicalDevice = context.getPhysicalDevice();
	m_commandPool = context.getCommandPool();
	m_graphicsQueue = context.getGraphicsQueue();

	int texWidth, texHeight, texChannels;
	stbi_set_flip_vertically_on_load(flipVertically);
	stbi_uc *pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
		return false;

	mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
				 stagingBufferMemory);

	void *data;
	vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(m_device, stagingBufferMemory);

	stbi_image_free(pixels);
	VulkanUtil::createImage(
		texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
						  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
	copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);

	generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_SRGB, texWidth, texHeight, mipLevels);
	return true;
}

bool ImageBuffer::initMaterialImageBuffer(std::string path, bool flipVertically)
{
	auto &context = VulkanContext::getContext();
	m_device = context.getDevice();
	m_physicalDevice = context.getPhysicalDevice();
	m_commandPool = context.getCommandPool();
	m_graphicsQueue = context.getGraphicsQueue();

	int texWidth, texHeight, texChannels;
	stbi_set_flip_vertically_on_load(flipVertically);
	stbi_uc *pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
		return false;

	mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
				 stagingBufferMemory);

	void *data;
	vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(m_device, stagingBufferMemory);

	stbi_image_free(pixels);

	VulkanUtil::createImage(
		texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED,
						  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
	copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);

	generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, mipLevels);
	return true;
}

void ImageBuffer::initImageBufferFromMemory(const aiTexture *texture)
{
	auto &context = VulkanContext::getContext();
	m_device = context.getDevice();
	m_physicalDevice = context.getPhysicalDevice();
	m_commandPool = context.getCommandPool();
	m_graphicsQueue = context.getGraphicsQueue();

	int texWidth, texHeight, texChannels;
	unsigned char *pixels = nullptr;

	if (texture->mHeight == 0)
	{
		// Compressed texture (e.g., JPEG/PNG)
		pixels =
			stbi_load_from_memory(reinterpret_cast<const stbi_uc *>(texture->pcData), static_cast<int>(texture->mWidth),
								  &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		if (!pixels)
		{
			throw std::runtime_error("Failed to decode compressed texture!");
		}
	}
	else
	{
		// Uncompressed RAW texture
		texWidth = texture->mWidth;
		texHeight = texture->mHeight;
		texChannels = 4; // Assume RGBA for uncompressed textures
		pixels = reinterpret_cast<unsigned char *>(texture->pcData);
	}

	mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	VkDeviceSize imageSize = texWidth * texHeight * 4; // RGBA: 4 bytes per pixel
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
				 stagingBufferMemory);

	void *data;
	vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(m_device, stagingBufferMemory);

	stbi_image_free(pixels);

	VulkanUtil::createImage(
		texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED,
						  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
	copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);

	generateMipmaps(textureImage, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, mipLevels);
}

std::unique_ptr<ImageBuffer> ImageBuffer::createDefaultImageBuffer(alglm::vec4 color)
{
	std::unique_ptr<ImageBuffer> imageBuffer = std::unique_ptr<ImageBuffer>(new ImageBuffer());
	imageBuffer->initDefaultImageBuffer(color);
	return imageBuffer;
}

void ImageBuffer::initDefaultImageBuffer(alglm::vec4 color)
{
	auto &context = VulkanContext::getContext();
	m_device = context.getDevice();
	m_physicalDevice = context.getPhysicalDevice();
	m_commandPool = context.getCommandPool();
	m_graphicsQueue = context.getGraphicsQueue();

	// 1. 스테이징 버퍼 생성 및 데이터 복사
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VkDeviceSize bufferSize = 4; // RGBA 1픽셀

	Buffer::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
						 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
						 stagingBufferMemory);

	void *data;
	vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
	uint8_t pixel[4] = {static_cast<uint8_t>(color.r * 255), static_cast<uint8_t>(color.g * 255),
						static_cast<uint8_t>(color.b * 255), static_cast<uint8_t>(color.a * 255)};
	memcpy(data, pixel, static_cast<size_t>(bufferSize));
	vkUnmapMemory(m_device, stagingBufferMemory);

	// 2. VulkanUtil을 사용하여 Default Image 생성
	mipLevels = 1; // Default Texture는 mipmap이 필요 없음

	VulkanUtil::createImage(1, 1, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL,
							VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED,
						  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);

	copyBufferToImage(stagingBuffer, textureImage, 1, 1);

	transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

std::unique_ptr<ImageBuffer> ImageBuffer::createDefaultSingleChannelImageBuffer(float value)
{
	std::unique_ptr<ImageBuffer> imageBuffer = std::unique_ptr<ImageBuffer>(new ImageBuffer());
	imageBuffer->initDefaultSingleChannelImageBuffer(value);
	return imageBuffer;
}

void ImageBuffer::initDefaultSingleChannelImageBuffer(float value)
{
	auto &context = VulkanContext::getContext();
	m_device = context.getDevice();
	m_physicalDevice = context.getPhysicalDevice();
	m_commandPool = context.getCommandPool();
	m_graphicsQueue = context.getGraphicsQueue();

	// 1. 스테이징 버퍼 생성 및 데이터 복사
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	VkDeviceSize bufferSize = 1; // 단일 R 채널 1픽셀

	Buffer::createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
						 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
						 stagingBufferMemory);

	void *data;
	vkMapMemory(m_device, stagingBufferMemory, 0, bufferSize, 0, &data);
	uint8_t pixel = static_cast<uint8_t>(value * 255); // 0.0 ~ 1.0 값을 0 ~ 255로 변환
	memcpy(data, &pixel, sizeof(pixel));
	vkUnmapMemory(m_device, stagingBufferMemory);

	// 2. VulkanUtil을 사용하여 단일 채널 이미지 생성
	mipLevels = 1; // Default Texture는 mipmap이 필요 없음

	VulkanUtil::createImage(1, 1, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R8_UNORM, VK_IMAGE_TILING_OPTIMAL,
							VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
							VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	transitionImageLayout(textureImage, VK_FORMAT_R8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED,
						  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);

	copyBufferToImage(stagingBuffer, textureImage, 1, 1);

	transitionImageLayout(textureImage, VK_FORMAT_R8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels);

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);
}

// 이미지 레이아웃, 접근 권한을 변경할 수 있는 베리어를 커맨드 버퍼에 기록
void ImageBuffer::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout,
										VkImageLayout newLayout, uint32_t mipLevels)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands(); // 커맨드 버퍼 생성 및 기록 시작

	// 베리어 생성을 위한 구조체
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;						   // src 단계까지의 이미지 레이아웃
	barrier.newLayout = newLayout;						   // src 단계 이후 적용시킬 새로운 이미지 레이아웃
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // 베리어 전환 적용 후 리소스 소유권을 넘겨줄 src 큐 패밀리
														   // (현재는 동일 큐 패밀리에서 실행)
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // 리소스 소유권을 받을 dst 큐 패밀리로 dst 큐패밀리에는 큐
														   // 전체에 동기화가 적용 (현재는 동일 큐 패밀리에서 실행)
	barrier.image = image;								   // 배리어 적용할 이미지 객체
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // 전환 작업의 적용 대상을 color bit 으로 설정
	barrier.subresourceRange.baseMipLevel = 0;						 // 전환 작업을 시작할 miplevel
	barrier.subresourceRange.levelCount = mipLevels;				 // 전환 작업을 적용할 miplevel의 개수
	barrier.subresourceRange.baseArrayLayer = 0;					 // 전환 작업을 시작할 레이어 인덱스
	barrier.subresourceRange.layerCount = 1;						 // 전환 작업을 적용할 레이어 개수

	VkPipelineStageFlags sourceStage;	   // 파이프라인의 sourceStage 단계가 끝나면 배리어 전환 실행
	VkPipelineStageFlags destinationStage; // destinationStage 단계는 배리어 전환이 끝날때까지 대기

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		// 이미지 복사 전에 이미지 레이아웃, 접근 권한 변경
		barrier.srcAccessMask = 0;							  // 접근 제한 x
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // 쓰기 권한 필요

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;   // Vulkan의 파이프라인에서 가장 상단에 위치한 첫 번째 단계로,
														   // 어떠한 작업도 진행되지 않은 상태
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT; // 데이터 복사 단계
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		// 이미지 복사가 완료되고 읽기를 수행하기 위해 Fragment shader 작업 대기
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT; // 쓰기 권한 필요
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;	  // 읽기 권한 필요

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;			  // 데이터 복사 단계
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // Fragment shader 단계
	}
	else
	{
		throw std::invalid_argument("unsupported layout transition!");
	}

	// 베리어를 커맨드 버퍼에 기록
	vkCmdPipelineBarrier(commandBuffer,					// 베리어를 기록할 커맨드 버퍼
						 sourceStage, destinationStage, // sourceStage 단계가 끝나면 베리어 작업 시작, 베리어 작업이
														// 끝나기 전에 destinationStage에 돌입한 다른 작업들 모두 대기
						 0,								// 의존성 플래그
						 0, nullptr,					// 메모리 베리어 (개수 + 베리어 포인터)
						 0, nullptr,					// 버퍼 베리어   (개수 + 베리어 포인터)
						 1, &barrier					// 이미지 베리어 (개수 + 베리어 포인터)
	);

	endSingleTimeCommands(commandBuffer); // 커맨드 버퍼 기록 종료
}

// 커맨드 버퍼 제출을 통해 버퍼 -> 이미지 데이터 복사
void ImageBuffer::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	// 커맨드 버퍼 생성 및 기록 시작
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	// 버퍼 -> 이미지 복사를 위한 정보
	VkBufferImageCopy region{};
	region.bufferOffset = 0;	  // 복사할 버퍼의 시작 위치 offset
	region.bufferRowLength = 0;	  // 저장될 공간의 row 당 픽셀 수 (0으로 하면 이미지 너비에 자동으로 맞춰진다.)
	region.bufferImageHeight = 0; // 저장될 공간의 col 당 픽셀 수 (0으로 하면 이미지 높이에 자동으로 맞춰진다.)
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // 이미지의 데이터 타입 (현재는 컬러값을 복사)
	region.imageSubresource.mipLevel = 0;							// 이미지의 miplevel 설정
	region.imageSubresource.baseArrayLayer = 0; // 이미지의 시작 layer 설정 (cubemap과 같은 경우 여러 레이어 존재)
	region.imageSubresource.layerCount = 1;		// 이미지 layer 개수
	region.imageOffset = {0, 0, 0};				// 이미지의 저장할 시작 위치
	region.imageExtent = {						// 이미지의 저장할 너비, 높이, 깊이
						  width, height, 1};

	// 커맨드 버퍼에 버퍼 -> 이미지로 데이터 복사하는 명령 기록
	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	// 커맨드 버퍼 기록 종료 및 제출
	endSingleTimeCommands(commandBuffer);
}

void ImageBuffer::generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight,
								  uint32_t mipLevels)
{
	// 이미지 포맷이 선형 필터링을 사용한 Blit 작업을 지원하는지 확인
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(m_physicalDevice, imageFormat, &formatProperties);

	if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		throw std::runtime_error("texture image format does not support linear blitting!");
	}

	// 커맨드 버퍼 생성 및 기록 시작
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	// 베리어 생성
	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	// miplevel 0 ~ mipLevels
	for (uint32_t i = 1; i < mipLevels; i++)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;			  // 해당 mipmap에 대한 barrier 설정
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; // 데이터 쓰기에 적합한 레이아웃
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL; // 데이터 읽기에 적합한 레이아웃
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;	  // 쓰기 권한 on
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;	  // 읽기 권한 on

		// 파이프라인 베리어 설정 (GPU 특정 작업간의 동기화 설정)
		// 이전 단계의 mipmap 복사가 끝나야, 다음 단계 mipmap 복사가 시작되게 베리어 설정
		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
							 nullptr, 0, nullptr, 1, &barrier);

		// blit 작업시 소스와 대상의 복사 범위를 지정하는 구조체
		VkImageBlit blit{};
		blit.srcOffsets[0] = {0, 0, 0};
		blit.srcOffsets[1] = {mipWidth, mipHeight, 1}; // 소스의 범위 설정
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1; // 소스의 mipLevel 설정
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = {0, 0, 0};
		blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1,
							  1}; // 대상의 범위 설정
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i; // 대상의 mipLevel 설정
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		// 소스 miplevel 을 대상 miplevel로 설정에 맞게 복사
		vkCmdBlitImage(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, image,
					   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
					   VK_FILTER_LINEAR); // 선형적으로 복사

		// shader 단계에서 사용하기 전에 Bllit 단계가 끝나기를 기다림
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
							 nullptr, 0, nullptr, 1, &barrier);

		if (mipWidth > 1)
			mipWidth /= 2;
		if (mipHeight > 1)
			mipHeight /= 2;
	}

	// 마지막 단계 miplevel 처리
	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
						 nullptr, 0, nullptr, 1, &barrier);

	endSingleTimeCommands(commandBuffer);
}

std::shared_ptr<UniformBuffer> UniformBuffer::createUniformBuffer(VkDeviceSize buffersize)
{
	std::shared_ptr<UniformBuffer> uniformBuffer = std::shared_ptr<UniformBuffer>(new UniformBuffer());
	uniformBuffer->initUniformBuffer(buffersize);
	return uniformBuffer;
}

void UniformBuffer::cleanup()
{
	if (m_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(m_device, m_buffer, nullptr);
		m_buffer = VK_NULL_HANDLE;
	}
	if (m_bufferMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(m_device, m_bufferMemory, nullptr);
		m_bufferMemory = VK_NULL_HANDLE;
	}
}

void UniformBuffer::updateUniformBuffer(void *data, VkDeviceSize size)
{
	memcpy(m_mappedMemory, data, size);
}

void UniformBuffer::initUniformBuffer(VkDeviceSize buffersize)
{
	auto &context = VulkanContext::getContext();
	m_device = context.getDevice();
	m_physicalDevice = context.getPhysicalDevice();
	m_commandPool = context.getCommandPool();
	m_graphicsQueue = context.getGraphicsQueue();

	createBuffer(buffersize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_buffer, m_bufferMemory);
	vkMapMemory(m_device, m_bufferMemory, 0, buffersize, 0, &m_mappedMemory);
}

std::unique_ptr<ImageBuffer> ImageBuffer::createHDRImageBuffer(std::string path)
{
	std::unique_ptr<ImageBuffer> imageBuffer = std::unique_ptr<ImageBuffer>(new ImageBuffer());
	if (!imageBuffer->initHDRImageBuffer(path))
		return nullptr;
	return imageBuffer;
}

bool ImageBuffer::initHDRImageBuffer(std::string path)
{
	auto &context = VulkanContext::getContext();
	m_device = context.getDevice();
	m_physicalDevice = context.getPhysicalDevice();
	m_commandPool = context.getCommandPool();
	m_graphicsQueue = context.getGraphicsQueue();

	int texWidth, texHeight, texChannels;
	float *pixels = stbi_loadf(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	if (!pixels)
		return false;
	VkDeviceSize imageSize = texWidth * texHeight * 4 * sizeof(float);

	mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
				 stagingBufferMemory);

	void *data;
	vkMapMemory(m_device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(m_device, stagingBufferMemory);

	stbi_image_free(pixels);
	VulkanUtil::createImage(
		texWidth, texHeight, mipLevels, VK_SAMPLE_COUNT_1_BIT, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

	transitionImageLayout(textureImage, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_LAYOUT_UNDEFINED,
						  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
	copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMemory, nullptr);

	generateMipmaps(textureImage, VK_FORMAT_R32G32B32A32_SFLOAT, texWidth, texHeight, mipLevels);
	return true;
}

std::shared_ptr<StorageBuffer> StorageBuffer::createStorageBuffer(VkDeviceSize bufferSize)
{
	std::shared_ptr<StorageBuffer> storageBuffer = std::shared_ptr<StorageBuffer>(new StorageBuffer());
	storageBuffer->initStorageBuffer(bufferSize);
	return storageBuffer;
}

void StorageBuffer::cleanup()
{
	if (m_mappedMemory)
	{
		vkUnmapMemory(m_device, m_bufferMemory);
		m_mappedMemory = nullptr;
	}
	if (m_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(m_device, m_buffer, nullptr);
		m_buffer = VK_NULL_HANDLE;
	}
	if (m_bufferMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(m_device, m_bufferMemory, nullptr);
		m_bufferMemory = VK_NULL_HANDLE;
	}
	m_currentSize = 0;
}

void StorageBuffer::updateStorageBuffer(void *data, VkDeviceSize totalSize)
{
	if (m_mappedMemory == nullptr)
	{
		std::cerr << "StorageBuffer: Mapped memory is nullptr!" << std::endl;
		return;
	}
	if (totalSize > m_currentSize)
	{
		std::cerr << "StorageBuffer: Total size is greater than current size!" << std::endl;
		return;
	}
	memcpy(m_mappedMemory, data, totalSize);
}

void StorageBuffer::updateStorageBufferAt(uint32_t index, void *data, VkDeviceSize structSize)
{
	if (m_mappedMemory == nullptr)
	{
		std::cerr << "StorageBuffer: Mapped memory is nullptr!" << std::endl;
		return;
	}
	if ((index + 1) * structSize > m_currentSize)
	{
		std::cerr << "StorageBuffer: Index out of bounds update attempted!" << std::endl;
		return;
	}
	memcpy(static_cast<uint8_t *>(m_mappedMemory) + index * structSize, data, structSize);
}

void StorageBuffer::resizeStorageBuffer(VkDeviceSize size)
{
	if (size > m_currentSize)
	{
		cleanup();
		initStorageBuffer(size);
	}
}

void StorageBuffer::initStorageBuffer(VkDeviceSize size)
{
	auto &context = VulkanContext::getContext();
	m_device = context.getDevice();
	m_physicalDevice = context.getPhysicalDevice();
	m_commandPool = context.getCommandPool();
	m_graphicsQueue = context.getGraphicsQueue();

	createBuffer(size, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_buffer, m_bufferMemory);
	vkMapMemory(m_device, m_bufferMemory, 0, size, 0, &m_mappedMemory);
	m_currentSize = size;
}

} // namespace ale