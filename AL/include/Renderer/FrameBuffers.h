#pragma once

/**
 * @file FrameBuffers.h
 * @brief Frame Buffers 클래스
 */

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/SwapChain.h"
#include "Renderer/VulkanContext.h"

namespace ale
{
class FrameBuffers
{
  public:
	/**
	 * @brief Viewport Frame Buffers 생성
	 * @param viewPortSize 뷰포트 크기
	 * @param renderPass 렌더 패스
	 * @return Viewport Frame Buffers
	 */
	static std::unique_ptr<FrameBuffers> createViewPortFrameBuffers(alglm::vec2 viewPortSize, VkRenderPass renderPass);
	/**
	 * @brief ImGui Frame Buffers 생성
	 * @param swapChain 스왑 체인
	 * @param renderPass 렌더 패스
	 * @return ImGui Frame Buffers
	 */
	static std::unique_ptr<FrameBuffers> createImGuiFrameBuffers(SwapChain *swapChain, VkRenderPass renderPass);
	/**
	 * @brief Shadow Map Frame Buffers 생성
	 * @param renderPass 렌더 패스
	 * @return Shadow Map Frame Buffers
	 */
	static std::unique_ptr<FrameBuffers> createShadowMapFrameBuffers(VkRenderPass renderPass);
	/**
	 * @brief Shadow Cube Map Frame Buffers 생성
	 * @param renderPass 렌더 패스
	 * @return Shadow Cube Map Frame Buffers
	 */
	static std::unique_ptr<FrameBuffers> createShadowCubeMapFrameBuffers(VkRenderPass renderPass);
	/**
	 * @brief Spherical Map Frame Buffers 생성
	 * @param renderPass 렌더 패스
	 * @return Spherical Map Frame Buffers
	 */
	static std::unique_ptr<FrameBuffers> createSphericalMapFrameBuffers(VkRenderPass renderPass);

	/**
	 * @brief Background Frame Buffers 생성
	 * @param viewPortSize 뷰포트 크기
	 * @param renderPass 렌더 패스
	 * @return Background Frame Buffers
	 */
	static std::unique_ptr<FrameBuffers> createBackgroundFrameBuffers(alglm::vec2 viewPortSize,
																	  VkRenderPass renderPass);

	/**
	 * @brief Collider Frame Buffers 생성
	 * @param viewPortSize 뷰포트 크기
	 * @param renderPass 렌더 패스
	 * @param viewPortImageView 뷰포트 이미지 뷰
	 * @return Collider Frame Buffers
	 */
	static std::unique_ptr<FrameBuffers> createColliderFrameBuffers(alglm::vec2 viewPortSize, VkRenderPass renderPass,
																	VkImageView viewPortImageView);

	~FrameBuffers() = default;

	/**
	 * @brief 프레임 버퍼 정리
	 */
	void cleanup();

	/**
	 * @brief Viewport 프레임 버퍼 초기화
	 * @param viewPortSize 뷰포트 크기
	 * @param renderPass 렌더 패스
	 */
	void initViewPortFrameBuffers(alglm::vec2 viewPortSize, VkRenderPass renderPass);
	/**
	 * @brief ImGui 프레임 버퍼 초기화
	 * @param swapChain 스왑 체인
	 * @param renderPass 렌더 패스
	 */
	void initImGuiFrameBuffers(SwapChain *swapChain, VkRenderPass renderPass);
	/**
	 * @brief Shadow Map 프레임 버퍼 초기화
	 * @param renderPass 렌더 패스
	 */
	void initShadowMapFrameBuffers(VkRenderPass renderPass);
	/**
	 * @brief Shadow Cube Map 프레임 버퍼 초기화
	 * @param renderPass 렌더 패스
	 */
	void initShadowCubeMapFrameBuffers(VkRenderPass renderPass);
	/**
	 * @brief Spherical Map 프레임 버퍼 초기화
	 * @param renderPass 렌더 패스
	 */
	void initSphericalMapFrameBuffers(VkRenderPass renderPass);
	/**
	 * @brief Background 프레임 버퍼 초기화
	 * @param viewPortSize 뷰포트 크기
	 * @param renderPass 렌더 패스
	 */
	void initBackgroundFrameBuffers(alglm::vec2 viewPortSize, VkRenderPass renderPass);
	/**
	 * @brief Collider 프레임 버퍼 초기화
	 * @param viewPortSize 뷰포트 크기
	 * @param renderPass 렌더 패스
	 * @param viewPortImageView 뷰포트 이미지 뷰
	 */
	void initColliderFrameBuffers(alglm::vec2 viewPortSize, VkRenderPass renderPass, VkImageView viewPortImageView);

	/**
	 * @brief Spherical Map 프레임 버퍼 반환
	 * @return Spherical Map 프레임 버퍼
	 */
	VkFramebuffer &getSphericalMapFramebuffer()
	{
		return framebuffers[0];
	}

	/**
	 * @brief Spherical Map 이미지 뷰 반환
	 * @return Spherical Map 이미지 뷰
	 */
	VkImageView &getSphericalMapImageView()
	{
		return sphericalMapImageView;
	}

	/**
	 * @brief 프레임 버퍼 반환
	 * @return 프레임 버퍼
	 */
	std::vector<VkFramebuffer> &getFramebuffers()
	{
		return framebuffers;
	}

	/**
	 * @brief 디퍼드 이미지 반환
	 * @return 디퍼드 이미지
	 */
	VkImage &getDepthImage()
	{
		return depthImage;
	}

	/**
	 * @brief 디퍼드 이미지 뷰 반환
	 * @return 디퍼드 이미지 뷰
	 */
	VkImageView &getDepthImageView()
	{
		return depthImageView;
	}

	/**
	 * @brief 포지션 이미지 뷰 반환
	 * @return 포지션 이미지 뷰
	 */
	VkImageView &getPositionImageView()
	{
		return positionImageView;
	}

	/**
	 * @brief 노말 이미지 뷰 반환
	 * @return 노말 이미지 뷰
	 */
	VkImageView &getNormalImageView()
	{
		return normalImageView;
	}

	/**
	 * @brief 알베도 이미지 뷰 반환
	 * @return 알베도 이미지 뷰
	 */
	VkImageView &getAlbedoImageView()
	{
		return albedoImageView;
	}

	/**
	 * @brief PBR 이미지 뷰 반환
	 * @return PBR 이미지 뷰
	 */
	VkImageView &getPbrImageView()
	{
		return pbrImageView;
	}

	/**
	 * @brief 뷰포트 이미지 뷰 반환
	 * @return 뷰포트 이미지 뷰
	 */
	VkImageView &getViewPortImageView()
	{
		return viewPortImageView;
	}

	/**
	 * @brief 뷰포트 이미지 반환
	 * @return 뷰포트 이미지
	 */
	VkImage &getViewPortImage()
	{
		return viewPortImage;
	}

	/**
	 * @brief 배경 이미지 뷰 반환
	 * @return 배경 이미지 뷰
	 */
	VkImageView &getBackgroundImageView()
	{
		return backgroundImageView;
	}

	/**
	 * @brief 배경 이미지 반환
	 * @return 배경 이미지
	 */
	VkImage &getBackgroundImage()
	{
		return backgroundImage;
	}

  private:
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkImage positionImage;
	VkDeviceMemory positionImageMemory;
	VkImageView positionImageView;

	VkImage normalImage;
	VkDeviceMemory normalImageMemory;
	VkImageView normalImageView;

	VkImage albedoImage;
	VkDeviceMemory albedoImageMemory;
	VkImageView albedoImageView;

	VkImage pbrImage;
	VkDeviceMemory pbrImageMemory;
	VkImageView pbrImageView;

	VkImage viewPortImage;
	VkDeviceMemory viewPortImageMemory;
	VkImageView viewPortImageView;

	VkImage sphericalMapImage;
	VkDeviceMemory sphericalMapImageMemory;
	VkImageView sphericalMapImageView;

	VkImage backgroundImage;
	VkDeviceMemory backgroundImageMemory;
	VkImageView backgroundImageView;

	std::vector<VkFramebuffer> framebuffers;
};
} // namespace ale
