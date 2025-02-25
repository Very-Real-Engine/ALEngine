#pragma once

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/VulkanContext.h"
#include "Renderer/VulkanUtil.h"

namespace ale
{
/**
 * @class RenderPass
 * @brief 렌더 패스 클래스
 */
class RenderPass
{
  public:
	/**
	 * @brief 렌더 패스 생성
	 * @param swapChainImageFormat 스왑 체인 이미지 포맷
	 * @return std::unique_ptr<RenderPass> 렌더 패스
	 */
	static std::unique_ptr<RenderPass> createRenderPass(VkFormat swapChainImageFormat);
	/**
	 * @brief deferred renderpass 생성
	 * @return std::unique_ptr<RenderPass> deferred renderpass
	 */
	static std::unique_ptr<RenderPass> createDeferredRenderPass();
	/**
	 * @brief imgui renderpass 생성
	 * @param swapChainImageFormat 스왑 체인 이미지 포맷
	 * @return std::unique_ptr<RenderPass> imgui renderpass
	 */
	static std::unique_ptr<RenderPass> createImGuiRenderPass(VkFormat swapChainImageFormat);
	/**
	 * @brief shadow map renderpass 생성
	 * @return std::unique_ptr<RenderPass> shadow map renderpass
	 */
	static std::unique_ptr<RenderPass> createShadowMapRenderPass();
	/**
	 * @brief 구면 맵 렌더 패스 생성
	 * @return std::unique_ptr<RenderPass> 구면 맵 렌더 패스
	 */
	static std::unique_ptr<RenderPass> createSphericalMapRenderPass();
	/**
	 * @brief 배경 렌더 패스 생성
	 * @return std::unique_ptr<RenderPass> 배경 렌더 패스
	 */
	static std::unique_ptr<RenderPass> createBackgroundRenderPass();
	/**
	 * @brief 콜라이더 렌더 패스 생성
	 * @return std::unique_ptr<RenderPass> 콜라이더 렌더 패스
	 */
	static std::unique_ptr<RenderPass> createColliderRenderPass();

	/**
	 * @brief 렌더 패스 초기화
	 * @param swapChainImageFormat 스왑 체인 이미지 포맷
	 */
	void initRenderPass(VkFormat swapChainImageFormat);
	/**
	 * @brief deferred renderpass 초기화
	 */
	void initDeferredRenderPass();
	/**
	 * @brief imgui renderpass 초기화
	 * @param swapChainImageFormat 스왑 체인 이미지 포맷
	 */
	void initImGuiRenderPass(VkFormat swapChainImageFormat);
	/**
	 * @brief shadow map renderpass 초기화
	 */
	void initShadowMapRenderPass();
	/**
	 * @brief 구면 맵 렌더 패스 초기화
	 */
	void initSphericalMapRenderPass();
	/**
	 * @brief 배경 렌더 패스 초기화
	 */
	void initBackgroundRenderPass();
	/**
	 * @brief 콜라이더 렌더 패스 초기화
	 */
	void initColliderRenderPass();

	~RenderPass() = default;

	/**
	 * @brief 정리
	 */
	void cleanup();
	/**
	 * @brief 렌더 패스 반환
	 * @return VkRenderPass 렌더 패스
	 */
	VkRenderPass getRenderPass()
	{
		return renderPass;
	}

  private:
	VkRenderPass renderPass;
};
} // namespace ale
