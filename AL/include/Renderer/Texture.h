#pragma once
/**
 * @file Texture.h
 * @brief 텍스쳐 클래스
 */

#include "Core/Base.h"
#include "Renderer/Buffer.h"
#include "Renderer/Common.h"
#include "Renderer/VulkanContext.h"

namespace ale
{
class Texture : public Buffer
{
  public:
	/**
	 * @brief 텍스쳐 생성
	 * @param path 텍스쳐 경로
	 * @param flipVertically 세로 뒤집기 여부
	 * @return std::shared_ptr<Texture> 텍스쳐
	 */
	static std::shared_ptr<Texture> createTexture(std::string path, bool flipVertically = false);
	/**
	 * @brief 재질 텍스쳐 생성
	 * @param path 텍스쳐 경로
	 * @param flipVertically 세로 뒤집기 여부
	 * @return std::shared_ptr<Texture> 재질 텍스쳐
	 */
	static std::shared_ptr<Texture> createMaterialTexture(std::string path, bool flipVertically = false);
	/**
	 * @brief 기본 텍스쳐 생성
	 * @param color 색상
	 * @return std::shared_ptr<Texture> 기본 텍스쳐
	 */
	static std::shared_ptr<Texture> createDefaultTexture(alglm::vec4 color);
	/**
	 * @brief 기본 단일 채널 텍스쳐 생성
	 * @param value 값
	 * @return std::shared_ptr<Texture> 기본 단일 채널 텍스쳐
	 */
	static std::shared_ptr<Texture> createDefaultSingleChannelTexture(float value);
	/**
	 * @brief 메모리에서 텍스쳐 생성
	 * @param aiTexture aiTexture
	 * @return std::shared_ptr<Texture> 메모리에서 텍스쳐
	 */
	static std::shared_ptr<Texture> createTextureFromMemory(const aiTexture *aiTexture);
	/**
	 * @brief HDR 텍스쳐 생성
	 * @param path HDR 텍스쳐 경로
	 * @return std::shared_ptr<Texture> HDR 텍스쳐
	 */
	static std::shared_ptr<Texture> createHDRTexture(std::string path);
	/**
	 * @brief 그림자 맵 샘플러 생성
	 * @return VkSampler 그림자 맵 샘플러
	 */
	static VkSampler createShadowMapSampler();
	/**
	 * @brief 그림자 큐브 맵 샘플러 생성
	 * @return VkSampler 그림자 큐브 맵 샘플러
	 */
	static VkSampler createShadowCubeMapSampler();
	/**
	 * @brief 구면 맵 샘플러 생성
	 * @return VkSampler 구면 맵 샘플러
	 */
	static VkSampler createSphericalMapSampler();
	/**
	 * @brief 배경 샘플러 생성
	 * @return VkSampler 배경 샘플러
	 */
	static VkSampler createBackgroundSampler();
	/**
	 * @brief 콜라이더 샘플러 생성
	 * @return VkSampler 콜라이더 샘플러
	 */
	static VkSampler createColliderSampler();

	/**
	 * @brief 텍스쳐 초기화
	 * @param path 텍스쳐 경로
	 * @param flipVertically 세로 뒤집기 여부
	 */
	void initTexture(std::string path, bool flipVertically = false);
	/**
	 * @brief HDR 텍스쳐 초기화
	 * @param path HDR 텍스쳐 경로
	 */
	void initHDRTexture(std::string path);

	~Texture() = default;

	/**
	 * @brief 텍스쳐 정리
	 */
	void cleanup();

	/**
	 * @brief 텍스쳐 이미지 뷰 반환
	 * @return VkImageView 텍스쳐 이미지 뷰
	 */
	VkImageView getImageView()
	{
		return textureImageView;
	}
	/**
	 * @brief 텍스쳐 샘플러 반환
	 * @return VkSampler 텍스쳐 샘플러
	 */
	VkSampler getSampler()
	{
		return textureSampler;
	}

  private:
	Texture() = default;

	uint32_t mipLevels;
	std::unique_ptr<ImageBuffer> m_imageBuffer;
	VkImageView textureImageView;
	VkSampler textureSampler;

	/**
	 * @brief 텍스쳐 로드
	 * @param path 텍스쳐 경로
	 * @param flipVertically 세로 뒤집기 여부
	 */
	void loadTexture(std::string path, bool flipVertically = false);
	/**
	 * @brief 텍스쳐 이미지 뷰 생성
	 */
	void createTextureImageView();
	/**
	 * @brief 텍스쳐 샘플러 생성
	 */
	void createTextureSampler();

	/**
	 * @brief 재질 텍스쳐 초기화
	 * @param path 텍스쳐 경로
	 * @param flipVertically 세로 뒤집기 여부
	 */
	void initMaterialTexture(std::string path, bool flipVertically = false);
	/**
	 * @brief 재질 텍스쳐 로드
	 * @param path 텍스쳐 경로
	 * @param flipVertically 세로 뒤집기 여부
	 */
	void loadMaterialTexture(std::string path, bool flipVertically = false);
	/**
	 * @brief 재질 텍스쳐 이미지 뷰 생성
	 */
	void createMaterialTextureImageView();

	/**
	 * @brief 기본 텍스쳐 초기화
	 * @param color 색상
	 */
	void initDefaultTexture(alglm::vec4 color);
	/**
	 * @brief 기본 텍스쳐 이미지 뷰 생성
	 */
	void createDefaultTextureImageView();
	/**
	 * @brief 기본 텍스쳐 샘플러 생성
	 */
	void createDefaultTextureSampler();

	/**
	 * @brief 기본 단일 채널 텍스쳐 초기화
	 * @param value 값
	 */
	void initDefaultSingleChannelTexture(float value);
	/**
	 * @brief 기본 단일 채널 텍스쳐 이미지 뷰 생성
	 */
	void createDefaultSingleChannelTextureImageView();
	/**
	 * @brief 기본 단일 채널 텍스쳐 샘플러 생성
	 */
	void createDefaultSingleChannelTextureSampler();

	/**
	 * @brief 메모리에서 텍스쳐 초기화
	 * @param texture 텍스쳐
	 */
	void initTextureFromMemory(const aiTexture *texture);
	/**
	 * @brief HDR 텍스쳐 이미지 뷰 생성
	 */
	void createHDRTextureImageView();
	/**
	 * @brief HDR 텍스쳐 샘플러 생성
	 */
	void createHDRTextureSampler();
};

/**
 * @brief 기본 텍스쳐 구조체
 */
struct DefaultTextures
{
	std::shared_ptr<Texture> albedo;
	std::shared_ptr<Texture> normal;
	std::shared_ptr<Texture> roughness;
	std::shared_ptr<Texture> metallic;
	std::shared_ptr<Texture> ao;
	std::shared_ptr<Texture> height;
};

} // namespace ale
