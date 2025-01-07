#ifndef TEXTURE_H
#define TEXTURE_H

#include "Core/Base.h"
#include "Renderer/Buffer.h"
#include "Renderer/Common.h"
#include "Renderer/VulkanContext.h"

namespace ale
{
class AL_API Texture : public Buffer
{
  public:
	static std::shared_ptr<Texture> createTexture(std::string path, bool flipVertically = false);
	static std::shared_ptr<Texture> createDefaultTexture(glm::vec4 color);
	static std::shared_ptr<Texture> createDefaultSingleChannelTexture(float value);

	~Texture() = default;

	void cleanup();

	VkImageView getImageView()
	{
		return textureImageView;
	}
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

	void initTexture(std::string path, bool flipVertically = false);
	void loadTexture(std::string path, bool flipVertically = false);
	void createTextureImageView();
	void createTextureSampler();

	void initDefaultTexture(glm::vec4 color);
	void createDefaultTextureImageView();
	void createDefaultTextureSampler();

	void initDefaultSingleChannelTexture(float value);
	void createDefaultSingleChannelTextureImageView();
	void createDefaultSingleChannelTextureSampler();
};

struct DefaultTextures {
	std::shared_ptr<Texture> albedo;
	std::shared_ptr<Texture> normal;
	std::shared_ptr<Texture> roughness;
	std::shared_ptr<Texture> metallic;
	std::shared_ptr<Texture> ao;
	std::shared_ptr<Texture> height;
};


} // namespace ale

#endif