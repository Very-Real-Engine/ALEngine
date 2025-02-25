#pragma once

/**
 * @file Buffer.h
 * @brief 버퍼 클래스
 *
 */

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/VulkanContext.h"
#include "Renderer/VulkanUtil.h"

#include "assimp/texture.h"

namespace ale
{
/**
 * @brief 버퍼 클래스
 * @details 버퍼 데이터를 관리하는 조상 클래스
 */
class Buffer
{
  public:
	/**
	 * @brief 버퍼 클래스 소멸자
	 */
	virtual ~Buffer() = default;
	/**
	 * @brief 버퍼 클래스 정리
	 */
	virtual void cleanup() = 0;

  protected:
	VkBuffer m_buffer;
	VkDeviceMemory m_bufferMemory;

	VkDevice m_device;
	VkPhysicalDevice m_physicalDevice;
	VkCommandPool m_commandPool;
	VkQueue m_graphicsQueue;

	/**
	 * @brief 버퍼 생성
	 * @param size 버퍼 크기
	 * @param usage 버퍼 사용 방법
	 * @param properties 버퍼 속성
	 * @param buffer 버퍼
	 * @param bufferMemory 버퍼 메모리
	 */
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer,
					  VkDeviceMemory &bufferMemory);
	/**
	 * @brief 버퍼 복사
	 * @param srcBuffer 소스 버퍼
	 * @param dstBuffer 대상 버퍼
	 * @param size 버퍼 크기
	 */
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	/**
	 * @brief 단일 시간 명령 시작
	 * @return 명령 버퍼
	 */
	VkCommandBuffer beginSingleTimeCommands();
	/**
	 * @brief 단일 시간 명령 종료
	 * @param commandBuffer 명령 버퍼
	 */
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
};

/**
 * @brief 정점 버퍼 클래스
 * @details 정점 데이터를 관리하는 클래스
 */
class VertexBuffer : public Buffer
{
  public:
	/**
	 * @brief 정점 버퍼 생성
	 * @param vertices 정점 데이터
	 * @return 정점 버퍼
	 */
	static std::unique_ptr<VertexBuffer> createVertexBuffer(std::vector<Vertex> &vertices);
	/**
	 * @brief 정점 버퍼 소멸자
	 */
	~VertexBuffer() = default;
	/**
	 * @brief 정점 버퍼 정리
	 */
	void cleanup();
	/**
	 * @brief 정점 버퍼 바인드
	 * @param commandBuffer 명령 버퍼
	 */
	void bind(VkCommandBuffer commandBuffer);

  private:
	/**
	 * @brief 정점 버퍼 초기화
	 * @param vertices 정점 데이터
	 */
	void initVertexBuffer(std::vector<Vertex> &vertices);
};

/**
 * @brief 인덱스 버퍼 클래스
 * @details 인덱스 데이터를 관리하는 클래스
 */
class IndexBuffer : public Buffer
{
  public:
	/**
	 * @brief 인덱스 버퍼 생성
	 * @param indices 인덱스 데이터
	 * @return 인덱스 버퍼
	 */
	static std::unique_ptr<IndexBuffer> createIndexBuffer(std::vector<uint32_t> &indices);
	/**
	 * @brief 인덱스 버퍼 소멸자
	 */
	~IndexBuffer() = default;
	/**
	 * @brief 인덱스 버퍼 정리
	 */
	void cleanup();
	/**
	 * @brief 인덱스 버퍼 바인드
	 * @param commandBuffer 명령 버퍼
	 */
	void bind(VkCommandBuffer commandBuffer);
	/**
	 * @brief 인덱스 버퍼 카운트 반환
	 * @return 인덱스 버퍼 카운트
	 */
	uint32_t getIndexCount()
	{
		return m_indexCount;
	}

  private:
	uint32_t m_indexCount;
	/**
	 * @brief 인덱스 버퍼 초기화
	 * @param indices 인덱스 데이터
	 */
	void initIndexBuffer(std::vector<uint32_t> &indices);
};

/**
 * @brief 이미지 버퍼 클래스
 * @details 이미지 데이터를 관리하는 클래스
 */
class ImageBuffer : public Buffer
{
  public:
	/**
	 * @brief 이미지 버퍼 생성
	 * @param path 이미지 경로
	 * @param flipVertically 세로 반전 여부
	 * @return 이미지 버퍼
	 */
	static std::unique_ptr<ImageBuffer> createImageBuffer(std::string path, bool flipVertically = false);
	/**
	 * @brief 재질 이미지 버퍼 생성
	 * @param path 이미지 경로
	 * @param flipVertically 세로 반전 여부
	 * @return 재질 이미지 버퍼
	 */
	static std::unique_ptr<ImageBuffer> createMaterialImageBuffer(std::string path, bool flipVertically = false);
	/**
	 * @brief 이미지 버퍼 메모리 생성
	 * @param texture 이미지 데이터
	 * @return 이미지 버퍼 메모리
	 */
	static std::unique_ptr<ImageBuffer> createImageBufferFromMemory(const aiTexture *texture);
	/**
	 * @brief 기본 이미지 버퍼 생성
	 * @param color 색상
	 * @return 기본 이미지 버퍼
	 */
	static std::unique_ptr<ImageBuffer> createDefaultImageBuffer(alglm::vec4 color);
	/**
	 * @brief 기본 단일 채널 이미지 버퍼 생성
	 * @param value 값
	 * @return 기본 단일 채널 이미지 버퍼
	 */
	static std::unique_ptr<ImageBuffer> createDefaultSingleChannelImageBuffer(float value);
	/**
	 * @brief HDR 이미지 버퍼 생성
	 * @param path 이미지 경로
	 * @return HDR 이미지 버퍼
	 */
	static std::unique_ptr<ImageBuffer> createHDRImageBuffer(std::string path);
	/**
	 * @brief 이미지 버퍼 소멸자
	 */
	~ImageBuffer() = default;
	/**
	 * @brief 이미지 버퍼 정리
	 */
	void cleanup();
	/**
	 * @brief 이미지 버퍼 미프 레벨 반환
	 * @return 이미지 버퍼 미프 레벨
	 */
	uint32_t getMipLevels()
	{
		return mipLevels;
	}
	/**
	 * @brief 이미지 버퍼 이미지 반환
	 * @return 이미지 버퍼 이미지
	 */
	VkImage getImage()
	{
		return textureImage;
	}
	/**
	 * @brief 이미지 버퍼 이미지 메모리 반환
	 * @return 이미지 버퍼 이미지 메모리
	 */
	VkDeviceMemory getImageMemory()
	{
		return textureImageMemory;
	}

  private:
	uint32_t mipLevels;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;

	/**
	 * @brief 이미지 버퍼 초기화
	 * @param path 이미지 경로
	 * @param flipVertically 세로 반전 여부
	 * @return 이미지 버퍼 초기화 여부
	 */
	bool initImageBuffer(std::string path, bool flipVertically);
	/**
	 * @brief 재질 이미지 버퍼 초기화
	 * @param path 이미지 경로
	 * @param flipVertically 세로 반전 여부
	 * @return 재질 이미지 버퍼 초기화 여부
	 */
	bool initMaterialImageBuffer(std::string path, bool flipVertically);
	/**
	 * @brief 이미지 버퍼 메모리 초기화
	 * @param texture 이미지 데이터
	 */
	void initImageBufferFromMemory(const aiTexture *texture);
	/**
	 * @brief 기본 이미지 버퍼 초기화
	 * @param color 색상
	 */
	void initDefaultImageBuffer(alglm::vec4 color);
	/**
	 * @brief 기본 단일 채널 이미지 버퍼 초기화
	 * @param value 값
	 */
	void initDefaultSingleChannelImageBuffer(float value);
	/**
	 * @brief HDR 이미지 버퍼 초기화
	 * @param path 이미지 경로
	 * @return HDR 이미지 버퍼 초기화 여부
	 */
	bool initHDRImageBuffer(std::string path);
	/**
	 * @brief 이미지 버퍼 레이아웃 전환
	 * @param image 이미지
	 * @param format 이미지 포맷
	 * @param oldLayout 이전 레이아웃
	 * @param newLayout 새로운 레이아웃
	 * @param mipLevels 미프 레벨
	 */
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
							   uint32_t mipLevels);
	/**
	 * @brief 버퍼에서 이미지로 복사
	 * @param buffer 버퍼
	 * @param image 이미지
	 * @param width 너비
	 * @param height 높이
	 */
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
	/**
	 * @brief 미프맵 생성
	 * @param image 이미지
	 * @param imageFormat 이미지 포맷
	 * @param texWidth 텍스처 너비
	 * @param texHeight 텍스처 높이
	 * @param mipLevels 미프 레벨
	 */
	void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
};

/**
 * @brief 유니폼 버퍼 클래스
 * @details 유니폼 데이터를 관리하는 클래스
 */
class UniformBuffer : public Buffer
{
  public:
	/**
	 * @brief 유니폼 버퍼 생성
	 * @param buffersize 버퍼 크기
	 * @return 유니폼 버퍼
	 */
	static std::shared_ptr<UniformBuffer> createUniformBuffer(VkDeviceSize buffersize);
	/**
	 * @brief 유니폼 버퍼 소멸자
	 */
	~UniformBuffer() = default;
	/**
	 * @brief 유니폼 버퍼 정리
	 */
	void cleanup();
	/**
	 * @brief 유니폼 버퍼 업데이트
	 * @param data 데이터
	 * @param size 데이터 크기
	 */
	void updateUniformBuffer(void *data, VkDeviceSize size);

	/**
	 * @brief 버퍼 반환
	 * @return 버퍼
	 */
	VkBuffer getBuffer()
	{
		return m_buffer;
	}
	/**
	 * @brief 버퍼 메모리 반환
	 * @return 버퍼 메모리
	 */
	VkDeviceMemory getBufferMemory()
	{
		return m_bufferMemory;
	}

  private:
	void *m_mappedMemory = nullptr;

	/**
	 * @brief 유니폼 버퍼 초기화
	 * @param buffersize 버퍼 크기
	 */
	void initUniformBuffer(VkDeviceSize buffersize);
};
} // namespace ale
