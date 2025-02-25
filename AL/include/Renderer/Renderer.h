#ifndef RENDERER_H
#define RENDERER_H

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
class Renderer
{
  public:
	static std::unique_ptr<Renderer> createRenderer(GLFWwindow *window);
	~Renderer() = default;
	void cleanup();

	void loadScene(Scene *scene);
	void beginScene(Scene *scene, EditorCamera &camera);
	void beginScene(Scene *scene, Camera &camera);
	void biginNoCamScene();
	void drawFrame(Scene *scene);
	void drawNoCamFrame();
	void recreateSwapChain();
	void recreateViewPort();

	void updateSkybox(std::string path);

	VkDevice getDevice()
	{
		return device;
	}

	VkRenderPass getRenderPass()
	{
		return imGuiRenderPass;
	}

	VkDescriptorSetLayout getDescriptorSetLayout()
	{
		return geometryPassDescriptorSetLayout;
	}

	VkDescriptorSet getViewPortDescriptorSet()
	{
		return viewPortDescriptorSets[0];
	}
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

	glm::vec2 viewPortSize;

	glm::mat4 projMatrix;
	glm::mat4 viewMatirx;

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

	void init(GLFWwindow *window);

	void recordDeferredRenderPassCommandBuffer(Scene *scene, VkCommandBuffer commandBuffer, uint32_t imageIndex,
											   uint32_t shadowMapIndex);
	void recordImGuiCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	void recordShadowMapCommandBuffer(Scene *scene, VkCommandBuffer commandBuffer, Light &lightInfo,
									  uint32_t shadowMapIndex);
	void recordShadowCubeMapCommandBuffer(Scene *scene, VkCommandBuffer commandBuffer, Light &lightInfo,
										  uint32_t shadowMapIndex);
	void recordSphericalMapCommandBuffer();
	void recordBackgroundCommandBuffer(VkCommandBuffer commandBuffer);
	void recordColliderCommandBuffer(Scene *scene, VkCommandBuffer commandBuffer);
};
} // namespace ale

#endif