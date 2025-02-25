#pragma once

#include "Renderer/Common.h"
#include "Renderer/Texture.h"

#include <filesystem>

namespace ale
{
/**
 * @class ContentBrowserPanel
 * @brief 콘텐츠 브라우저 패널을 관리하는 클래스.
 *        파일 및 디렉터리 탐색 기능을 제공하며, ImGui를 통해 UI를 렌더링합니다.
 */
class ContentBrowserPanel
{
  public:
	/// @brief ContentBrowserPanel 생성자.
	ContentBrowserPanel();

	/// @brief ContentBrowserPanel 소멸자.
	~ContentBrowserPanel();

	/**
	 * @brief ImGui를 사용하여 콘텐츠 브라우저 UI를 렌더링합니다.
	 */
	void onImGuiRender();

  private:
	/**
	 * @brief 아이콘 텍스처를 생성합니다.
	 * @param imageView Vulkan 이미지 뷰.
	 * @param sampler Vulkan 샘플러.
	 * @return ImTextureID 생성된 ImGui 텍스처 ID.
	 */
	ImTextureID createIconTexture(VkImageView imageView, VkSampler sampler);

  private:
	std::filesystem::path m_BaseDirectory;
	std::filesystem::path m_CurrentDirectory;

	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	VkDevice device;

	VkDescriptorSetLayout iconDescriptorSetLayout;
	VkDescriptorSet directoryDescriptorSet;
	VkDescriptorSet fileDescriptorSet;
	std::shared_ptr<Texture> m_DirectoryIcon;
	std::shared_ptr<Texture> m_FileIcon;
	ImTextureID directoryTextureID;
	ImTextureID fileTextureID;
};

} // namespace ale