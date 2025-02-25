#pragma once

/**
 * @file DescriptorSetLayout.h
 * @brief Descriptor Set Layout 클래스
 */

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/VulkanContext.h"

namespace ale
{
/**
 * @brief Descriptor Set Layout 클래스
 */
class DescriptorSetLayout
{
  public:
	/**
	 * @brief 디스크립터 세트 레이아웃 생성
	 * @return 디스크립터 세트 레이아웃
	 */
	static std::unique_ptr<DescriptorSetLayout> createDescriptorSetLayout();
	/**
	 * @brief Geometry Pass Descriptor Set Layout 생성
	 * @return Geometry Pass Descriptor Set Layout
	 */
	static std::unique_ptr<DescriptorSetLayout> createGeometryPassDescriptorSetLayout();
	/**
	 * @brief Lighting Pass Descriptor Set Layout 생성
	 * @return Lighting Pass Descriptor Set Layout
	 */
	static std::unique_ptr<DescriptorSetLayout> createLightingPassDescriptorSetLayout();
	/**
	 * @brief Shadow Map Descriptor Set Layout 생성
	 * @return Shadow Map Descriptor Set Layout
	 */
	static std::unique_ptr<DescriptorSetLayout> createShadowMapDescriptorSetLayout();
	/**
	 * @brief Shadow Cube Map Descriptor Set Layout 생성
	 * @return Shadow Cube Map Descriptor Set Layout
	 */
	static std::unique_ptr<DescriptorSetLayout> createShadowCubeMapDescriptorSetLayout();
	/**
	 * @brief Viewport Descriptor Set Layout 생성
	 * @return Viewport Descriptor Set Layout
	 */
	static std::unique_ptr<DescriptorSetLayout> createViewPortDescriptorSetLayout();
	/**
	 * @brief Spherical Map Descriptor Set Layout 생성
	 * @return Spherical Map Descriptor Set Layout
	 */
	static std::unique_ptr<DescriptorSetLayout> createSphericalMapDescriptorSetLayout();
	/**
	 * @brief Background Descriptor Set Layout 생성
	 * @return Background Descriptor Set Layout
	 */
	static std::unique_ptr<DescriptorSetLayout> createBackgroundDescriptorSetLayout();
	/**
	 * @brief Collider Descriptor Set Layout 생성
	 * @return Collider Descriptor Set Layout
	 */
	static std::unique_ptr<DescriptorSetLayout> createColliderDescriptorSetLayout();

	/**
	 * @brief Descriptor Set Layout 소멸자
	 */
	~DescriptorSetLayout() = default;

	/**
	 * @brief Descriptor Set Layout 정리
	 */
	void cleanup();

	/**
	 * @brief Descriptor Set Layout 반환
	 * @return Descriptor Set Layout
	 */
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
