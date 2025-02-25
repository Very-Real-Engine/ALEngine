#ifndef DESCRIPTORSETLAYOUT_H
#define DESCRIPTORSETLAYOUT_H

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/VulkanContext.h"

namespace ale
{
class DescriptorSetLayout
{
  public:
	static std::unique_ptr<DescriptorSetLayout> createDescriptorSetLayout();
	static std::unique_ptr<DescriptorSetLayout> createGeometryPassDescriptorSetLayout();
	static std::unique_ptr<DescriptorSetLayout> createLightingPassDescriptorSetLayout();
	static std::unique_ptr<DescriptorSetLayout> createShadowMapDescriptorSetLayout();
	static std::unique_ptr<DescriptorSetLayout> createShadowCubeMapDescriptorSetLayout();
	static std::unique_ptr<DescriptorSetLayout> createViewPortDescriptorSetLayout();
	static std::unique_ptr<DescriptorSetLayout> createSphericalMapDescriptorSetLayout();
	static std::unique_ptr<DescriptorSetLayout> createBackgroundDescriptorSetLayout();
	static std::unique_ptr<DescriptorSetLayout> createColliderDescriptorSetLayout();

	~DescriptorSetLayout() = default;

	void cleanup();

	VkDescriptorSetLayout getDescriptorSetLayout()
	{
		return descriptorSetLayout;
	}

  private:
	VkDescriptorSetLayout descriptorSetLayout;

	void initDescriptorSetLayout();
	void initGeometryPassDescriptorSetLayout();
	void initLightingPassDescriptorSetLayout();
	void initShadowMapDescriptorSetLayout();
	void initShadowCubeMapDescriptorSetLayout();
	void initViewPortDescriptorSetLayout();
	void initSphericalMapDescriptorSetLayout();
	void initBackgroundDescriptorSetLayout();
	void initColliderDescriptorSetLayout();
};
} // namespace ale

#endif
