#ifndef DESCRIPTORSETLAYOUT_H
#define DESCRIPTORSETLAYOUT_H

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/VulkanContext.h"

namespace ale
{
class AL_API DescriptorSetLayout
{
  public:
	static std::unique_ptr<DescriptorSetLayout> createDescriptorSetLayout();
	static std::unique_ptr<DescriptorSetLayout> createGeometryPassDescriptorSetLayout();
	static std::unique_ptr<DescriptorSetLayout> createLightingPassDescriptorSetLayout();
	static std::unique_ptr<DescriptorSetLayout> createShadowMapDescriptorSetLayout();
	static std::unique_ptr<DescriptorSetLayout> createShadowCubeMapDescriptorSetLayout();
	~DescriptorSetLayout()
	{
	}
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
};
} // namespace ale
#endif
