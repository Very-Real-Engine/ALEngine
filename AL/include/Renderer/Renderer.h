#pragma once

/**
 * @file Renderer.h
 * @brief 렌더러 클래스
 */

#include "Core/Base.h"
#include "Renderer/CommandBuffers.h"
#include "Renderer/Common.h"
#include "Renderer/DescriptorSetLayout.h"
#include "Renderer/EditorCamera.h"
#include "Renderer/FrameBuffers.h"
#include "Renderer/Pipeline.h"
#include "Renderer/RenderPass.h"
#include "Renderer/SAComponent.h"
#include "Renderer/ShaderResourceManager.h"
#include "Renderer/SwapChain.h"
#include "Renderer/SyncObjects.h"
#include "Renderer/VulkanContext.h"

#include "Scene/Scene.h"
#include "Scene/SceneCamera.h"

namespace ale
{
/**
 * @class Renderer
 * @brief 렌더러 클래스
 */
class Renderer
{
  public:
	/**
	 * @brief 렌더러 생성
	 * @param window 윈도우
	 * @return std::unique_ptr<Renderer> 렌더러
	 */
	static std::unique_ptr<Renderer> createRenderer(GLFWwindow *window);
	~Renderer() = default;
	/**
	 * @brief 렌더러 정리
	 */
	void cleanup();

	/**
	 * @brief 씬 로드
	 * @param scene 씬
	 */
	void loadScene(Scene *scene);
	/**
	 * @brief 씬 시작
	 * @param scene 씬
	 * @param camera 카메라
	 */
	void beginScene(Scene *scene, EditorCamera &camera);
	/**
	 * @brief 카메라 없는 씬 시작
	 * @param scene 씬
	 * @param camera 카메라
	 */
	void beginScene(Scene *scene, Camera &camera);

	/**
	 * @brief 카메라 없는 씬 시작
	 */
	void biginNoCamScene();
	/**
	 * @brief 프레임 그리기
	 * @param scene 씬
	 */
	void drawFrame(Scene *scene);
	/**
	 * @brief 카메라 없는 프레임 그리기
	 */
	void drawNoCamFrame();
	/**
	 * @brief 스왑 체인 재생성
	 */
	void recreateSwapChain();
	/**
	 * @brief 뷰포트 재생성
	 */
	void recreateViewPort();
	/**
	 * @brief 스카이박스 업데이트
	 * @param path 경로
	 */
	void updateSkybox(std::string path);

	/**
	 * @brief 디바이스 반환
	 * @return VkDevice 디바이스
	 */
	VkDevice getDevice()
	{
		return device;
	}
	/**
	 * @brief 렌더 패스 반환
	 * @return VkRenderPass 렌더 패스
	 */
	VkRenderPass getRenderPass()
	{
		return imGuiRenderPass;
	}
	/**
	 * @brief 디스크립터 세트 레이아웃 반환
	 * @return VkDescriptorSetLayout 디스크립터 세트 레이아웃
	 */
	VkDescriptorSetLayout getDescriptorSetLayout()
	{
		return geometryPassDescriptorSetLayout;
	}
	/**
	 * @brief 뷰포트 디스크립터 세트 반환
	 * @return VkDescriptorSet 뷰포트 디스크립터 세트
	 */
	VkDescriptorSet getViewPortDescriptorSet()
	{
		return viewPortDescriptorSets[0];
	}
	/**
	 * @brief 모델 맵 반환
	 * @return std::unordered_map<std::string, std::shared_ptr<Model>> & 모델 맵
	 */
	std::unordered_map<std::string, std::shared_ptr<Model>> &getModelsMap()
	{
		return m_modelsMap;
	}

  private:
	Renderer() = default;

	// Scene *scene;

	// Vulkan
	GLFWwindow *window;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkDevice device;
	VkCommandPool commandPool;
	VkQueue graphicsQueue;
	VkQueue presentQueue;

	// SwapChain
	std::unique_ptr<SwapChain> m_swapChain;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	std::vector<VkImageView> swapChainImageViews;
	std::unique_ptr<FrameBuffers> m_viewPortFrameBuffers;
	std::vector<VkFramebuffer> viewPortFramebuffers;

	std::unique_ptr<FrameBuffers> m_ImGuiSwapChainFrameBuffers;
	std::vector<VkFramebuffer> imGuiSwapChainFrameBuffers;

	// RenderPass
	std::unique_ptr<RenderPass> m_renderPass;
	VkRenderPass renderPass;

	std::unique_ptr<RenderPass> m_ImGuiRenderPass;
	VkRenderPass imGuiRenderPass;

	std::unique_ptr<RenderPass> m_deferredRenderPass;
	VkRenderPass deferredRenderPass;

	// DescriptorSetLayout
	std::unique_ptr<DescriptorSetLayout> m_geometryPassDescriptorSetLayout;
	VkDescriptorSetLayout geometryPassDescriptorSetLayout;

	std::unique_ptr<DescriptorSetLayout> m_lightingPassDescriptorSetLayout;
	VkDescriptorSetLayout lightingPassDescriptorSetLayout;

	// Pipeline
	std::unique_ptr<Pipeline> m_geometryPassPipeline;
	VkPipelineLayout geometryPassPipelineLayout;
	VkPipeline geometryPassGraphicsPipeline;

	std::unique_ptr<Pipeline> m_lightingPassPipeline;
	VkPipelineLayout lightingPassPipelineLayout;
	VkPipeline lightingPassGraphicsPipeline;

	// Descriptor Pool
	VkDescriptorPool descriptorPool;

	std::unique_ptr<ShaderResourceManager> m_lightingPassShaderResourceManager;
	std::vector<VkDescriptorSet> lightingPassDescriptorSets;
	std::vector<std::shared_ptr<UniformBuffer>> lightingPassUniformBuffers;
	std::vector<std::shared_ptr<UniformBuffer>> lightingPassFragmentUniformBuffers;

	std::unique_ptr<CommandBuffers> m_commandBuffers;

	std::vector<VkCommandBuffer> commandBuffers;
	std::unique_ptr<SyncObjects> m_syncObjects;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	uint32_t currentFrame = 0;

	// ShadowMap Info
	std::vector<std::unique_ptr<RenderPass>> m_shadowMapRenderPass;
	std::vector<VkRenderPass> shadowMapRenderPass;

	std::vector<std::unique_ptr<Pipeline>> m_shadowMapPipeline;
	std::vector<VkPipelineLayout> shadowMapPipelineLayout;
	std::vector<VkPipeline> shadowMapGraphicsPipeline;

	std::vector<std::unique_ptr<FrameBuffers>> m_shadowMapFrameBuffers;
	std::vector<std::vector<VkFramebuffer>> shadowMapFramebuffers;
	std::vector<VkImageView> shadowMapImageViews;

	std::unique_ptr<DescriptorSetLayout> m_shadowMapDescriptorSetLayout;
	VkDescriptorSetLayout shadowMapDescriptorSetLayout;
	VkSampler shadowMapSampler;

	std::vector<std::unique_ptr<RenderPass>> m_shadowCubeMapRenderPass;
	std::vector<VkRenderPass> shadowCubeMapRenderPass;

	std::vector<std::unique_ptr<Pipeline>> m_shadowCubeMapPipeline;
	std::vector<VkPipelineLayout> shadowCubeMapPipelineLayout;
	std::vector<VkPipeline> shadowCubeMapGraphicsPipeline;

	std::vector<std::unique_ptr<FrameBuffers>> m_shadowCubeMapFrameBuffers;
	std::vector<std::vector<VkFramebuffer>> shadowCubeMapFramebuffers;
	std::vector<VkImageView> shadowCubeMapImageViews;

	std::unique_ptr<DescriptorSetLayout> m_shadowCubeMapDescriptorSetLayout;
	VkDescriptorSetLayout shadowCubeMapDescriptorSetLayout;

	VkSampler shadowCubeMapSampler;

	VkImageView viewPortImageView;
	VkSampler viewPortSampler;

	std::unique_ptr<DescriptorSetLayout> m_viewPortDescriptorSetLayout;
	VkDescriptorSetLayout viewPortDescriptorSetLayout;

	std::unique_ptr<ShaderResourceManager> m_viewPortShaderResourceManager;
	std::vector<VkDescriptorSet> viewPortDescriptorSets;

	alglm::vec2 viewPortSize;

	alglm::mat4 projMatrix;
	alglm::mat4 viewMatirx;

	std::unordered_map<std::string, std::shared_ptr<Model>> m_modelsMap;

	// skybox
	std::shared_ptr<Texture> m_sphericalMapTexture;

	std::unique_ptr<RenderPass> m_sphericalMapRenderPass;
	VkRenderPass sphericalMapRenderPass;

	std::unique_ptr<FrameBuffers> m_sphericalMapFrameBuffers;
	std::vector<VkFramebuffer> sphericalMapFramebuffers;
	VkImageView sphericalMapImageView;
	VkSampler sphericalMapSampler;

	std::unique_ptr<Pipeline> m_sphericalMapPipeline;
	VkPipelineLayout sphericalMapPipelineLayout;
	VkPipeline sphericalMapGraphicsPipeline;

	std::unique_ptr<DescriptorSetLayout> m_sphericalMapDescriptorSetLayout;
	VkDescriptorSetLayout sphericalMapDescriptorSetLayout;

	std::unique_ptr<ShaderResourceManager> m_sphericalMapShaderResourceManager;
	std::vector<VkDescriptorSet> sphericalMapDescriptorSets;
	std::vector<std::shared_ptr<UniformBuffer>> sphericalMapUniformBuffers;

	// background
	std::unique_ptr<RenderPass> m_backgroundRenderPass;
	VkRenderPass backgroundRenderPass;

	std::unique_ptr<Pipeline> m_backgroundPipeline;
	VkPipelineLayout backgroundPipelineLayout;
	VkPipeline backgroundGraphicsPipeline;

	std::unique_ptr<DescriptorSetLayout> m_backgroundDescriptorSetLayout;
	VkDescriptorSetLayout backgroundDescriptorSetLayout;

	std::unique_ptr<ShaderResourceManager> m_backgroundShaderResourceManager;
	std::vector<VkDescriptorSet> backgroundDescriptorSets;
	std::vector<std::shared_ptr<UniformBuffer>> backgroundUniformBuffers;

	std::unique_ptr<FrameBuffers> m_backgroundFrameBuffers;
	std::vector<VkFramebuffer> backgroundFramebuffers;
	VkImageView backgroundImageView;
	VkSampler backgroundSampler;

	std::shared_ptr<Texture> m_noCamTexture;
	std::unique_ptr<ShaderResourceManager> m_noCamShaderResourceManager;
	std::vector<VkDescriptorSet> noCamDescriptorSets;

	std::unique_ptr<RenderPass> m_colliderRenderPass;
	VkRenderPass colliderRenderPass;

	std::unique_ptr<FrameBuffers> m_colliderFrameBuffers;
	std::vector<VkFramebuffer> colliderFramebuffers;

	std::unique_ptr<DescriptorSetLayout> m_colliderDescriptorSetLayout;
	VkDescriptorSetLayout colliderDescriptorSetLayout;

	std::unique_ptr<ShaderResourceManager> m_colliderShaderResourceManager;
	std::vector<VkDescriptorSet> colliderDescriptorSets;
	std::vector<std::shared_ptr<UniformBuffer>> colliderUniformBuffers;

	std::unique_ptr<Pipeline> m_colliderPipeline;
	VkPipelineLayout colliderPipelineLayout;
	VkPipeline colliderGraphicsPipeline;

	bool firstFrame = true;

	/**
	 * @brief 초기화
	 * @param window 윈도우
	 */
	void init(GLFWwindow *window);
	/**
	 * @brief 디퍼드 렌더 패스 명령 버퍼 레코드
	 * @param scene 씬
	 * @param commandBuffer 명령 버퍼
	 * @param imageIndex 이미지 인덱스
	 * @param shadowMapIndex 그림자 맵 인덱스
	 */
	void recordDeferredRenderPassCommandBuffer(Scene *scene, VkCommandBuffer commandBuffer, uint32_t imageIndex,
											   uint32_t shadowMapIndex);
	/**
	 * @brief 이미지 인덱스 레코드
	 * @param commandBuffer 명령 버퍼
	 * @param imageIndex 이미지 인덱스
	 */
	void recordImGuiCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	/**
	 * @brief 그림자 맵 명령 버퍼 레코드
	 * @param scene 씬
	 * @param commandBuffer 명령 버퍼
	 * @param lightInfo 조명 정보
	 * @param shadowMapIndex 그림자 맵 인덱스
	 */
	void recordShadowMapCommandBuffer(Scene *scene, VkCommandBuffer commandBuffer, Light &lightInfo,
									  uint32_t shadowMapIndex);

	/**
	 * @brief 그림자 큐브 맵 명령 버퍼 레코드
	 * @param scene 씬
	 * @param commandBuffer 명령 버퍼
	 * @param lightInfo 조명 정보
	 * @param shadowMapIndex 그림자 맵 인덱스
	 */
	void recordShadowCubeMapCommandBuffer(Scene *scene, VkCommandBuffer commandBuffer, Light &lightInfo,
										  uint32_t shadowMapIndex);
	/**
	 * @brief 구면 맵 명령 버퍼 레코드
	 */
	void recordSphericalMapCommandBuffer();
	/**
	 * @brief 배경 명령 버퍼 레코드
	 * @param commandBuffer 명령 버퍼
	 */
	void recordBackgroundCommandBuffer(VkCommandBuffer commandBuffer);
	/**
	 * @brief 콜라이더 명령 버퍼 레코드
	 * @param scene 씬
	 * @param commandBuffer 명령 버퍼
	 */
	void recordColliderCommandBuffer(Scene *scene, VkCommandBuffer commandBuffer);
};
} // namespace ale
