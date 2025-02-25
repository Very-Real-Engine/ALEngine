#ifndef BUFFER_H
#define BUFFER_H

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/VulkanContext.h"
#include "Renderer/VulkanUtil.h"

#include "assimp/texture.h"

namespace ale
{
class Buffer
{
  public:
	virtual ~Buffer() = default;
	virtual void cleanup() = 0;

  protected:
	VkBuffer m_buffer;
	VkDeviceMemory m_bufferMemory;

	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;
	VkCommandPool m_commandPool;
	VkQueue m_graphicsQueue;

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer,
					  VkDeviceMemory &bufferMemory);
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
};

class VertexBuffer : public Buffer
{
  public:
	static std::unique_ptr<VertexBuffer> createVertexBuffer(std::vector<Vertex> &vertices);
	~VertexBuffer() = default;

	void cleanup();

	void bind(VkCommandBuffer commandBuffer);

  private:
	void initVertexBuffer(std::vector<Vertex> &vertices);
};

class IndexBuffer : public Buffer
{
  public:
	static std::unique_ptr<IndexBuffer> createIndexBuffer(std::vector<uint32_t> &indices);
	~IndexBuffer() = default;

	void cleanup();

	void bind(VkCommandBuffer commandBuffer);

	uint32_t getIndexCount()
	{
		return m_indexCount;
	}

  private:
	uint32_t m_indexCount;

	void initIndexBuffer(std::vector<uint32_t> &indices);
};

class ImageBuffer : public Buffer
{
  public:
	static std::unique_ptr<ImageBuffer> createImageBuffer(std::string path, bool flipVertically = false);
	static std::unique_ptr<ImageBuffer> createMaterialImageBuffer(std::string path, bool flipVertically = false);
	static std::unique_ptr<ImageBuffer> createImageBufferFromMemory(const aiTexture *texture);
	static std::unique_ptr<ImageBuffer> createDefaultImageBuffer(glm::vec4 color);
	static std::unique_ptr<ImageBuffer> createDefaultSingleChannelImageBuffer(float value);
	static std::unique_ptr<ImageBuffer> createHDRImageBuffer(std::string path);

	~ImageBuffer() = default;

	void cleanup();

	uint32_t getMipLevels()
	{
		return mipLevels;
	}
	VkImage getImage()
	{
		return textureImage;
	}
	VkDeviceMemory getImageMemory()
	{
		return textureImageMemory;
	}

  private:
	uint32_t mipLevels;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;

	bool initImageBuffer(std::string path, bool flipVertically);
	bool initMaterialImageBuffer(std::string path, bool flipVertically);
	void initImageBufferFromMemory(const aiTexture *texture);
	void initDefaultImageBuffer(glm::vec4 color);
	void initDefaultSingleChannelImageBuffer(float value);
	bool initHDRImageBuffer(std::string path);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
							   uint32_t mipLevels);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
};

class UniformBuffer : public Buffer
{
  public:
	static std::shared_ptr<UniformBuffer> createUniformBuffer(VkDeviceSize buffersize);
	~UniformBuffer() = default;

	void cleanup();

	void updateUniformBuffer(void *data, VkDeviceSize size);

	VkBuffer getBuffer()
	{
		return m_buffer;
	}
	VkDeviceMemory getBufferMemory()
	{
		return m_bufferMemory;
	}

  private:
	void *m_mappedMemory = nullptr;

	void initUniformBuffer(VkDeviceSize buffersize);
};
} // namespace ale

#endif