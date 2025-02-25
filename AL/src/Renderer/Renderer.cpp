#include "Renderer/Renderer.h"
#include "ALpch.h"
#include "ImGui/ImGuiLayer.h"
#include "Renderer/CameraController.h"

#include "Renderer/RenderingComponent.h"
#include "Scene/Component.h"

namespace ale
{
std::unique_ptr<Renderer> Renderer::createRenderer(GLFWwindow *window)
{
	std::unique_ptr<Renderer> renderer = std::unique_ptr<Renderer>(new Renderer());
	renderer->init(window);
	return renderer;
}

void Renderer::init(GLFWwindow *window)
{
	this->window = window;
	viewPortSize = {1024, 1024};

#pragma region Vulkan Context
	auto &context = VulkanContext::getContext();
	context.initContext(window);
	surface = context.getSurface();
	physicalDevice = context.getPhysicalDevice();
	device = context.getDevice();
	graphicsQueue = context.getGraphicsQueue();
	presentQueue = context.getPresentQueue();
	commandPool = context.getCommandPool();
	msaaSamples = context.getMsaaSamples();
	descriptorPool = context.getDescriptorPool();

#pragma endregion

#pragma region SwapChain
	m_swapChain = SwapChain::createSwapChain(window);
	swapChain = m_swapChain->getSwapChain();
	swapChainImages = m_swapChain->getSwapChainImages();
	swapChainImageFormat = m_swapChain->getSwapChainImageFormat();
	swapChainExtent = m_swapChain->getSwapChainExtent();
	swapChainImageViews = m_swapChain->getSwapChainImageViews();
#pragma endregion

#pragma region sync
	m_syncObjects = SyncObjects::createSyncObjects();
	imageAvailableSemaphores = m_syncObjects->getImageAvailableSemaphores();
	renderFinishedSemaphores = m_syncObjects->getRenderFinishedSemaphores();
	inFlightFences = m_syncObjects->getInFlightFences();
#pragma endregion

	m_sphericalMapTexture = Texture::createHDRTexture("./Sandbox/assets/defaultSkybox.hdr");

	m_sphericalMapRenderPass = RenderPass::createSphericalMapRenderPass();
	sphericalMapRenderPass = m_sphericalMapRenderPass->getRenderPass();

	m_sphericalMapFrameBuffers = FrameBuffers::createSphericalMapFrameBuffers(sphericalMapRenderPass);
	sphericalMapFramebuffers = m_sphericalMapFrameBuffers->getFramebuffers();
	sphericalMapImageView = m_sphericalMapFrameBuffers->getSphericalMapImageView();
	sphericalMapSampler = Texture::createSphericalMapSampler();

	m_sphericalMapDescriptorSetLayout = DescriptorSetLayout::createSphericalMapDescriptorSetLayout();
	sphericalMapDescriptorSetLayout = m_sphericalMapDescriptorSetLayout->getDescriptorSetLayout();

	m_sphericalMapPipeline =
		Pipeline::createSphericalMapPipeline(sphericalMapRenderPass, sphericalMapDescriptorSetLayout);
	sphericalMapPipelineLayout = m_sphericalMapPipeline->getPipelineLayout();
	sphericalMapGraphicsPipeline = m_sphericalMapPipeline->getPipeline();

	m_sphericalMapShaderResourceManager = ShaderResourceManager::createSphericalMapShaderResourceManager(
		sphericalMapDescriptorSetLayout, m_sphericalMapTexture->getImageView(), m_sphericalMapTexture->getSampler());
	sphericalMapDescriptorSets = m_sphericalMapShaderResourceManager->getDescriptorSets();
	sphericalMapUniformBuffers = m_sphericalMapShaderResourceManager->getUniformBuffers();

	recordSphericalMapCommandBuffer();

	m_backgroundRenderPass = RenderPass::createBackgroundRenderPass();
	backgroundRenderPass = m_backgroundRenderPass->getRenderPass();

	m_backgroundDescriptorSetLayout = DescriptorSetLayout::createBackgroundDescriptorSetLayout();
	backgroundDescriptorSetLayout = m_backgroundDescriptorSetLayout->getDescriptorSetLayout();

	m_backgroundPipeline = Pipeline::createBackgroundPipeline(backgroundRenderPass, backgroundDescriptorSetLayout);
	backgroundPipelineLayout = m_backgroundPipeline->getPipelineLayout();
	backgroundGraphicsPipeline = m_backgroundPipeline->getPipeline();

	m_backgroundFrameBuffers = FrameBuffers::createBackgroundFrameBuffers(viewPortSize, backgroundRenderPass);
	backgroundFramebuffers = m_backgroundFrameBuffers->getFramebuffers();
	backgroundImageView = m_backgroundFrameBuffers->getBackgroundImageView();
	backgroundSampler = Texture::createBackgroundSampler();

	m_backgroundShaderResourceManager = ShaderResourceManager::createBackgroundShaderResourceManager(
		backgroundDescriptorSetLayout, sphericalMapImageView, sphericalMapSampler);
	backgroundDescriptorSets = m_backgroundShaderResourceManager->getDescriptorSets();
	backgroundUniformBuffers = m_backgroundShaderResourceManager->getUniformBuffers();

#pragma region RenderPass
	m_deferredRenderPass = RenderPass::createDeferredRenderPass();

	deferredRenderPass = m_deferredRenderPass->getRenderPass();

	m_ImGuiRenderPass = RenderPass::createImGuiRenderPass(swapChainImageFormat);
	imGuiRenderPass = m_ImGuiRenderPass->getRenderPass();

	for (size_t i = 0; i < 4; i++)
	{
		m_shadowMapRenderPass.push_back(RenderPass::createShadowMapRenderPass());
		shadowMapRenderPass.push_back(m_shadowMapRenderPass[i]->getRenderPass());
	}

	for (size_t i = 0; i < 4; i++)
	{
		m_shadowCubeMapRenderPass.push_back(RenderPass::createShadowMapRenderPass());
		shadowCubeMapRenderPass.push_back(m_shadowCubeMapRenderPass[i]->getRenderPass());
	}
#pragma endregion

#pragma region Framebuffer
	m_viewPortFrameBuffers = FrameBuffers::createViewPortFrameBuffers(viewPortSize, deferredRenderPass);
	viewPortFramebuffers = m_viewPortFrameBuffers->getFramebuffers();
	viewPortImageView = m_viewPortFrameBuffers->getViewPortImageView();
	viewPortSampler = VulkanUtil::createSampler();

	m_ImGuiSwapChainFrameBuffers = FrameBuffers::createImGuiFrameBuffers(m_swapChain.get(), imGuiRenderPass);
	imGuiSwapChainFrameBuffers = m_ImGuiSwapChainFrameBuffers->getFramebuffers();

	for (size_t i = 0; i < 4; i++)
	{
		m_shadowMapFrameBuffers.push_back(FrameBuffers::createShadowMapFrameBuffers(shadowMapRenderPass[i]));
		shadowMapFramebuffers.push_back(m_shadowMapFrameBuffers[i]->getFramebuffers());
		shadowMapImageViews.push_back(m_shadowMapFrameBuffers[i]->getDepthImageView());
	}

	for (size_t i = 0; i < 4; i++)
	{
		m_shadowCubeMapFrameBuffers.push_back(
			FrameBuffers::createShadowCubeMapFrameBuffers(shadowCubeMapRenderPass[i]));
		shadowCubeMapFramebuffers.push_back(m_shadowCubeMapFrameBuffers[i]->getFramebuffers());
		shadowCubeMapImageViews.push_back(m_shadowCubeMapFrameBuffers[i]->getDepthImageView());
	}
#pragma endregion

#pragma region DescriptorSetLaytout
	m_geometryPassDescriptorSetLayout = DescriptorSetLayout::createGeometryPassDescriptorSetLayout();
	geometryPassDescriptorSetLayout = m_geometryPassDescriptorSetLayout->getDescriptorSetLayout();
	context.setGeometryPassDescriptorSetLayout(geometryPassDescriptorSetLayout);

	m_lightingPassDescriptorSetLayout = DescriptorSetLayout::createLightingPassDescriptorSetLayout();
	lightingPassDescriptorSetLayout = m_lightingPassDescriptorSetLayout->getDescriptorSetLayout();

	m_shadowMapDescriptorSetLayout = DescriptorSetLayout::createShadowMapDescriptorSetLayout();
	shadowMapDescriptorSetLayout = m_shadowMapDescriptorSetLayout->getDescriptorSetLayout();
	context.setShadowMapDescriptorSetLayout(shadowMapDescriptorSetLayout);

	m_shadowCubeMapDescriptorSetLayout = DescriptorSetLayout::createShadowCubeMapDescriptorSetLayout();
	shadowCubeMapDescriptorSetLayout = m_shadowCubeMapDescriptorSetLayout->getDescriptorSetLayout();
	context.setShadowCubeMapDescriptorSetLayout(shadowCubeMapDescriptorSetLayout);

#pragma endregion

#pragma region Pipeline

	m_geometryPassPipeline = Pipeline::createGeometryPassPipeline(deferredRenderPass, geometryPassDescriptorSetLayout);
	geometryPassPipelineLayout = m_geometryPassPipeline->getPipelineLayout();
	geometryPassGraphicsPipeline = m_geometryPassPipeline->getPipeline();

	m_lightingPassPipeline = Pipeline::createLightingPassPipeline(deferredRenderPass, lightingPassDescriptorSetLayout);
	lightingPassPipelineLayout = m_lightingPassPipeline->getPipelineLayout();
	lightingPassGraphicsPipeline = m_lightingPassPipeline->getPipeline();

	for (size_t i = 0; i < 4; i++)
	{
		m_shadowMapPipeline.push_back(
			Pipeline::createShadowMapPipeline(shadowMapRenderPass[i], shadowMapDescriptorSetLayout));
		shadowMapPipelineLayout.push_back(m_shadowMapPipeline[i]->getPipelineLayout());
		shadowMapGraphicsPipeline.push_back(m_shadowMapPipeline[i]->getPipeline());
	}

	for (size_t i = 0; i < 4; i++)
	{
		m_shadowCubeMapPipeline.push_back(
			Pipeline::createShadowCubeMapPipeline(shadowCubeMapRenderPass[i], shadowCubeMapDescriptorSetLayout));
		shadowCubeMapPipelineLayout.push_back(m_shadowCubeMapPipeline[i]->getPipelineLayout());
		shadowCubeMapGraphicsPipeline.push_back(m_shadowCubeMapPipeline[i]->getPipeline());
	}

#pragma endregion

#pragma region etc(Sampler, ShaderResourceManager, Commandbuffer)
	shadowMapSampler = Texture::createShadowMapSampler();
	shadowCubeMapSampler = Texture::createShadowCubeMapSampler();

	m_lightingPassShaderResourceManager = ShaderResourceManager::createLightingPassShaderResourceManager(
		lightingPassDescriptorSetLayout, m_viewPortFrameBuffers->getPositionImageView(),
		m_viewPortFrameBuffers->getNormalImageView(), m_viewPortFrameBuffers->getAlbedoImageView(),
		m_viewPortFrameBuffers->getPbrImageView(), shadowMapImageViews, shadowMapSampler, shadowCubeMapImageViews,
		shadowCubeMapSampler, backgroundImageView, backgroundSampler);

	lightingPassDescriptorSets = m_lightingPassShaderResourceManager->getDescriptorSets();
	lightingPassFragmentUniformBuffers = m_lightingPassShaderResourceManager->getFragmentUniformBuffers();

	m_viewPortDescriptorSetLayout = DescriptorSetLayout::createViewPortDescriptorSetLayout();
	viewPortDescriptorSetLayout = m_viewPortDescriptorSetLayout->getDescriptorSetLayout();
	m_viewPortShaderResourceManager = ShaderResourceManager::createViewPortShaderResourceManager(
		viewPortDescriptorSetLayout, viewPortImageView, viewPortSampler);
	viewPortDescriptorSets = m_viewPortShaderResourceManager->getDescriptorSets();

	m_noCamTexture = Texture::createTexture("./Sandbox/assets/noCam.png");
	m_noCamShaderResourceManager = ShaderResourceManager::createViewPortShaderResourceManager(
		viewPortDescriptorSetLayout, m_noCamTexture->getImageView(), m_noCamTexture->getSampler());
	noCamDescriptorSets = m_noCamShaderResourceManager->getDescriptorSets();

	m_commandBuffers = CommandBuffers::createCommandBuffers();
	commandBuffers = m_commandBuffers->getCommandBuffers();

#pragma endregion
}

void Renderer::cleanup()
{
	// texture
	m_sphericalMapTexture->cleanup();
	m_noCamTexture->cleanup();

	// framebuffer
	m_viewPortFrameBuffers->cleanup();
	m_ImGuiSwapChainFrameBuffers->cleanup();
	for (size_t i = 0; i < 4; i++)
	{
		m_shadowMapFrameBuffers[i]->cleanup();
		m_shadowCubeMapFrameBuffers[i]->cleanup();
	}
	m_sphericalMapFrameBuffers->cleanup();
	m_backgroundFrameBuffers->cleanup();

	// swapchain
	m_swapChain->cleanup();
	// model
	for (auto &model : m_modelsMap)
	{
		model.second->cleanup();
	}

	// pipeline
	m_geometryPassPipeline->cleanup();
	m_lightingPassPipeline->cleanup();
	for (size_t i = 0; i < 4; i++)
	{
		m_shadowMapPipeline[i]->cleanup();
		m_shadowCubeMapPipeline[i]->cleanup();
	}
	m_sphericalMapPipeline->cleanup();
	m_backgroundPipeline->cleanup();

	// renderpass
	m_deferredRenderPass->cleanup();
	m_sphericalMapRenderPass->cleanup();
	m_backgroundRenderPass->cleanup();
	for (size_t i = 0; i < 4; i++)
	{
		m_shadowMapRenderPass[i]->cleanup();
		m_shadowCubeMapRenderPass[i]->cleanup();
	}
	m_ImGuiRenderPass->cleanup();

	// sampler
	vkDestroySampler(device, sphericalMapSampler, nullptr);
	vkDestroySampler(device, backgroundSampler, nullptr);
	vkDestroySampler(device, shadowMapSampler, nullptr);
	vkDestroySampler(device, shadowCubeMapSampler, nullptr);
	vkDestroySampler(device, viewPortSampler, nullptr);

	// shaderResourceManager
	m_sphericalMapShaderResourceManager->cleanup();
	m_backgroundShaderResourceManager->cleanup();
	m_viewPortShaderResourceManager->cleanup();
	m_noCamShaderResourceManager->cleanup();
	m_lightingPassShaderResourceManager->cleanup();

	// descriptorSetLayout
	m_geometryPassDescriptorSetLayout->cleanup();
	m_lightingPassDescriptorSetLayout->cleanup();
	m_viewPortDescriptorSetLayout->cleanup();
	m_shadowMapDescriptorSetLayout->cleanup();
	m_shadowCubeMapDescriptorSetLayout->cleanup();
	m_sphericalMapDescriptorSetLayout->cleanup();
	m_backgroundDescriptorSetLayout->cleanup();

	m_syncObjects->cleanup();
	VulkanContext::getContext().cleanup();
}

/*
	변경된 window 크기에 맞게 SwapChain, ImageView, FrameBuffer 재생성
*/
void Renderer::recreateSwapChain()
{
	// 현재 프레임버퍼 사이즈 체크
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);

	// 현재 프레임 버퍼 사이즈가 0이면 다음 이벤트 호출까지 대기
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents(); // 다음 이벤트 발생 전까지 대기하여 CPU 사용률을 줄이는 함수
	}

	// 모든 GPU 작업 종료될 때까지 대기 (사용중인 리소스를 건들지 않기 위해)
	vkDeviceWaitIdle(device);

	// 스왑 체인 관련 리소스 정리
	m_ImGuiSwapChainFrameBuffers->cleanup();
	m_swapChain->recreateSwapChain();

	swapChain = m_swapChain->getSwapChain();
	swapChainImages = m_swapChain->getSwapChainImages();
	swapChainImageFormat = m_swapChain->getSwapChainImageFormat();
	swapChainExtent = m_swapChain->getSwapChainExtent();
	swapChainImageViews = m_swapChain->getSwapChainImageViews();

	m_ImGuiSwapChainFrameBuffers->initImGuiFrameBuffers(m_swapChain.get(), imGuiRenderPass);
	imGuiSwapChainFrameBuffers = m_ImGuiSwapChainFrameBuffers->getFramebuffers();
}

void Renderer::recreateViewPort()
{
	while (viewPortSize.x == 0 || viewPortSize.y == 0)
	{
		viewPortSize.x = ImGui::GetContentRegionAvail().x;
		viewPortSize.y = ImGui::GetContentRegionAvail().y;
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(device);
	// 스왑 체인 관련 리소스 정리
	m_lightingPassShaderResourceManager->cleanup();
	m_geometryPassPipeline->cleanup();
	m_lightingPassPipeline->cleanup();
	m_deferredRenderPass->cleanup();

	m_viewPortFrameBuffers->cleanup();
	m_viewPortShaderResourceManager->cleanup();
	m_noCamShaderResourceManager->cleanup();

	m_backgroundShaderResourceManager->cleanup();
	m_backgroundFrameBuffers->cleanup();
	m_backgroundPipeline->cleanup();
	m_backgroundRenderPass->cleanup();

	m_backgroundRenderPass->initBackgroundRenderPass();
	backgroundRenderPass = m_backgroundRenderPass->getRenderPass();

	m_backgroundPipeline->initBackgroundPipeline(backgroundRenderPass, backgroundDescriptorSetLayout);
	backgroundPipelineLayout = m_backgroundPipeline->getPipelineLayout();
	backgroundGraphicsPipeline = m_backgroundPipeline->getPipeline();

	m_backgroundFrameBuffers->initBackgroundFrameBuffers(viewPortSize, backgroundRenderPass);
	backgroundFramebuffers = m_backgroundFrameBuffers->getFramebuffers();
	backgroundImageView = m_backgroundFrameBuffers->getBackgroundImageView();

	m_backgroundShaderResourceManager->initBackgroundShaderResourceManager(backgroundDescriptorSetLayout,
																		   sphericalMapImageView, sphericalMapSampler);
	backgroundDescriptorSets = m_backgroundShaderResourceManager->getDescriptorSets();
	backgroundUniformBuffers = m_backgroundShaderResourceManager->getUniformBuffers();

	m_deferredRenderPass->initDeferredRenderPass();
	deferredRenderPass = m_deferredRenderPass->getRenderPass();

	m_viewPortFrameBuffers->initViewPortFrameBuffers(viewPortSize, deferredRenderPass);
	viewPortFramebuffers = m_viewPortFrameBuffers->getFramebuffers();
	viewPortImageView = m_viewPortFrameBuffers->getViewPortImageView();

	m_geometryPassPipeline->initGeometryPassPipeline(deferredRenderPass, geometryPassDescriptorSetLayout);
	geometryPassPipelineLayout = m_geometryPassPipeline->getPipelineLayout();
	geometryPassGraphicsPipeline = m_geometryPassPipeline->getPipeline();

	m_lightingPassPipeline->initLightingPassPipeline(deferredRenderPass, lightingPassDescriptorSetLayout);
	lightingPassPipelineLayout = m_lightingPassPipeline->getPipelineLayout();
	lightingPassGraphicsPipeline = m_lightingPassPipeline->getPipeline();

	m_lightingPassShaderResourceManager->initLightingPassShaderResourceManager(
		lightingPassDescriptorSetLayout, m_viewPortFrameBuffers->getPositionImageView(),
		m_viewPortFrameBuffers->getNormalImageView(), m_viewPortFrameBuffers->getAlbedoImageView(),
		m_viewPortFrameBuffers->getPbrImageView(), shadowMapImageViews, shadowMapSampler, shadowCubeMapImageViews,
		shadowCubeMapSampler, backgroundImageView, backgroundSampler);

	lightingPassDescriptorSets = m_lightingPassShaderResourceManager->getDescriptorSets();
	lightingPassFragmentUniformBuffers = m_lightingPassShaderResourceManager->getFragmentUniformBuffers();

	m_viewPortShaderResourceManager->initViewPortShaderResourceManager(viewPortDescriptorSetLayout, viewPortImageView,
																	   viewPortSampler);
	viewPortDescriptorSets = m_viewPortShaderResourceManager->getDescriptorSets();
	m_noCamShaderResourceManager->initViewPortShaderResourceManager(
		viewPortDescriptorSetLayout, m_noCamTexture->getImageView(), m_noCamTexture->getSampler());
	noCamDescriptorSets = m_noCamShaderResourceManager->getDescriptorSets();
}

void Renderer::updateSkybox(std::string path)
{
	vkDeviceWaitIdle(device);

	// clean up은 초기화의 역순

	m_lightingPassShaderResourceManager->cleanup();
	m_backgroundShaderResourceManager->cleanup();
	m_backgroundFrameBuffers->cleanup();
	m_backgroundPipeline->cleanup();
	m_backgroundRenderPass->cleanup();
	m_sphericalMapShaderResourceManager->cleanup();
	m_sphericalMapPipeline->cleanup();
	m_sphericalMapFrameBuffers->cleanup();
	m_sphericalMapRenderPass->cleanup();
	m_sphericalMapTexture->cleanup();

	// 새로운 텍스쳐 로드
	m_sphericalMapTexture->initHDRTexture(path);
	m_sphericalMapRenderPass->initSphericalMapRenderPass();
	sphericalMapRenderPass = m_sphericalMapRenderPass->getRenderPass();

	m_sphericalMapFrameBuffers->initSphericalMapFrameBuffers(sphericalMapRenderPass);
	sphericalMapFramebuffers = m_sphericalMapFrameBuffers->getFramebuffers();
	sphericalMapImageView = m_sphericalMapFrameBuffers->getSphericalMapImageView();
	// sphericalMapSampler = Texture::createSphericalMapSampler();
	m_sphericalMapPipeline->initSphericalMapPipeline(sphericalMapRenderPass, sphericalMapDescriptorSetLayout);
	sphericalMapPipelineLayout = m_sphericalMapPipeline->getPipelineLayout();
	sphericalMapGraphicsPipeline = m_sphericalMapPipeline->getPipeline();

	m_sphericalMapShaderResourceManager->initSphericalMapShaderResourceManager(
		sphericalMapDescriptorSetLayout, m_sphericalMapTexture->getImageView(), m_sphericalMapTexture->getSampler());
	sphericalMapDescriptorSets = m_sphericalMapShaderResourceManager->getDescriptorSets();
	sphericalMapUniformBuffers = m_sphericalMapShaderResourceManager->getUniformBuffers();

	recordSphericalMapCommandBuffer();
	m_backgroundRenderPass->initBackgroundRenderPass();
	backgroundRenderPass = m_backgroundRenderPass->getRenderPass();
	m_backgroundPipeline->initBackgroundPipeline(backgroundRenderPass, backgroundDescriptorSetLayout);
	backgroundPipelineLayout = m_backgroundPipeline->getPipelineLayout();
	backgroundGraphicsPipeline = m_backgroundPipeline->getPipeline();

	m_backgroundFrameBuffers->initBackgroundFrameBuffers(viewPortSize, backgroundRenderPass);
	backgroundFramebuffers = m_backgroundFrameBuffers->getFramebuffers();
	backgroundImageView = m_backgroundFrameBuffers->getBackgroundImageView();

	m_backgroundShaderResourceManager->initBackgroundShaderResourceManager(backgroundDescriptorSetLayout,
																		   sphericalMapImageView, sphericalMapSampler);
	backgroundDescriptorSets = m_backgroundShaderResourceManager->getDescriptorSets();
	backgroundUniformBuffers = m_backgroundShaderResourceManager->getUniformBuffers();

	m_lightingPassShaderResourceManager->initLightingPassShaderResourceManager(
		lightingPassDescriptorSetLayout, m_viewPortFrameBuffers->getPositionImageView(),
		m_viewPortFrameBuffers->getNormalImageView(), m_viewPortFrameBuffers->getAlbedoImageView(),
		m_viewPortFrameBuffers->getPbrImageView(), shadowMapImageViews, shadowMapSampler, shadowCubeMapImageViews,
		shadowCubeMapSampler, backgroundImageView, backgroundSampler);

	lightingPassDescriptorSets = m_lightingPassShaderResourceManager->getDescriptorSets();
	lightingPassFragmentUniformBuffers = m_lightingPassShaderResourceManager->getFragmentUniformBuffers();
}

void Renderer::loadScene(Scene *scene)
{
}

void Renderer::beginScene(Scene *scene, EditorCamera &camera)
{
	// frustum culling
	// AL_CORE_INFO("frustum culling start");
	scene->frustumCulling(camera.getFrustum());
	// AL_CORE_INFO("frustum culling finish");

	camera.setAspectRatio(viewPortSize.x / viewPortSize.y);
	projMatrix = camera.getProjection();
	viewMatirx = camera.getView();

	drawFrame(scene);

	// AL_CORE_INFO("before init Frustum");
	// scene->printCullTree();
	scene->initFrustumDrawFlag();
	// AL_CORE_INFO("after init Frustum");
	// scene->printCullTree();
}

void Renderer::beginScene(Scene *scene, Camera &camera)
{
	// projMatrix = camera.getProjection();
	// projMatrix = glm::perspective(glm::radians(45.0f), viewPortSize.x / viewPortSize.y, 0.01f, 100.0f);
	camera.setAspectRatio(viewPortSize.x / viewPortSize.y);
	projMatrix = camera.getProjection();
	viewMatirx = camera.getView();

	scene->frustumCulling(camera.getFrustum());
	drawFrame(scene);

	scene->initFrustumDrawFlag();
}

void Renderer::biginNoCamScene()
{
	drawNoCamFrame();
}

void Renderer::drawFrame(Scene *scene)
{
	// [이전 GPU 작업 대기]
	// 동시에 작업 가능한 최대 Frame 개수만큼 작업 중인 경우 대기 (가장 먼저 시작한 Frame 작업이 끝나서 Fence에 signal을
	// 보내기를 기다림)
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	// [작업할 image 준비]
	// 이번 Frame 에서 사용할 이미지 준비 및 해당 이미지 index 받아오기 (준비가 끝나면 signal 보낼 세마포어 등록)
	// vkAcquireNextImageKHR 함수는 CPU에서 swapChain과 surface의 호환성을 확인하고 GPU에 이미지 준비 명령을 내리는 함수
	// 만약 image가 프레젠테이션 큐에 작업이 진행 중이거나 대기 중이면 해당 image는 사용하지 않고 대기한다.
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame],
											VK_NULL_HANDLE, &imageIndex);

	// image 준비 실패로 인한 오류 처리
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		// 스왑 체인이 surface 크기와 호환되지 않는 경우로(창 크기 변경), 스왑 체인 재생성 후 다시 draw
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		// 진짜 오류 gg
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	if (firstFrame)
	{
		ImGui::Begin("ViewPort");
		ImGui::Image(reinterpret_cast<ImTextureID>(viewPortDescriptorSets[0]), ImVec2{viewPortSize.x, viewPortSize.y});
		ImGui::End();
		firstFrame = false;
	}
	else
	{
		ImGui::Begin("ViewPort");
		ImVec2 guiViewPortSize = ImGui::GetContentRegionAvail();
		if (guiViewPortSize.x != viewPortSize.x || guiViewPortSize.y != viewPortSize.y)
		{
			viewPortSize = glm::vec2(guiViewPortSize.x, guiViewPortSize.y);
			recreateViewPort();
		}
		ImGui::Image(reinterpret_cast<ImTextureID>(viewPortDescriptorSets[0]), ImVec2{viewPortSize.x, viewPortSize.y});
		ImGui::End();
	}

	// [Fence 초기화]
	// Fence signal 상태 not signaled 로 초기화
	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	// [Command Buffer에 명령 기록]
	// 커맨드 버퍼 초기화 및 명령 기록
	vkResetCommandBuffer(
		commandBuffers[currentFrame],
		/*VkCommandBufferResetFlagBits*/ 0); // 두 번째 매개변수인 Flag 를 0으로 초기화하면 기본 초기화 진행

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	auto view = scene->getAllEntitiesWith<LightComponent, TagComponent>();

	uint32_t shadowMapIndex = 0;
	for (auto entity : view)
	{
		if (!view.get<TagComponent>(entity).m_isActive)
		{
			continue;
		}
		std::shared_ptr<Light> light = view.get<LightComponent>(entity).m_Light;
		if (light->onShadowMap == 1 && shadowMapIndex < 4)
		{
			recordShadowMapCommandBuffer(scene, commandBuffers[currentFrame], *light.get(), shadowMapIndex);
			recordShadowCubeMapCommandBuffer(scene, commandBuffers[currentFrame], *light.get(), shadowMapIndex);
			shadowMapIndex++;
		}
	}

	recordBackgroundCommandBuffer(commandBuffers[currentFrame]);
	recordDeferredRenderPassCommandBuffer(scene, commandBuffers[currentFrame], imageIndex,
										  shadowMapIndex); // 현재 작업할 image의 index와 commandBuffer를 전송

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.image = m_viewPortFrameBuffers->getViewPortImage();
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;

	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(commandBuffers[currentFrame], VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
						 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	recordImGuiCommandBuffer(commandBuffers[currentFrame], imageIndex);

	if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record deferred renderpass command buffer!");
	}

	// [렌더링 Command Buffer 제출]
	// 렌더링 커맨드 버퍼 제출 정보 객체 생성
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	// 작업 실행 신호를 받을 대기 세마포어 설정 (해당 세마포어가 signal 상태가 되기 전엔 대기)
	VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;			 // 대기 세마포어 개수
	submitInfo.pWaitSemaphores = waitSemaphores; // 대기 세마포어 등록
	submitInfo.pWaitDstStageMask = waitStages;	 // 대기할 시점 등록 (그 전까지는 세마포어 상관없이 그냥 진행)

	// 커맨드 버퍼 등록
	submitInfo.commandBufferCount = 1;							// 커맨드 버퍼 개수 등록
	submitInfo.pCommandBuffers = &commandBuffers[currentFrame]; // 커매드 버퍼 등록

	// 작업이 완료된 후 신호를 보낼 세마포어 설정 (작업이 끝나면 해당 세마포어 signal 상태로 변경)
	VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
	submitInfo.signalSemaphoreCount = 1;			 // 작업 끝나고 신호를 보낼 세마포어 개수
	submitInfo.pSignalSemaphores = signalSemaphores; // 작업 끝나고 신호를 보낼 세마포어 등록

	// 커맨드 버퍼 제출
	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	// [프레젠테이션 Command Buffer 제출]
	// 프레젠테이션 커맨드 버퍼 제출 정보 객체 생성
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	// 작업 실행 신호를 받을 대기 세마포어 설정
	presentInfo.waitSemaphoreCount = 1;				// 대기 세마포어 개수
	presentInfo.pWaitSemaphores = signalSemaphores; // 대기 세마포어 등록

	// 제출할 스왑 체인 설정
	VkSwapchainKHR swapChains[] = {swapChain};
	presentInfo.swapchainCount = 1;			 // 스왑체인 개수
	presentInfo.pSwapchains = swapChains;	 // 스왑체인 등록
	presentInfo.pImageIndices = &imageIndex; // 스왑체인에서 표시할 이미지 핸들 등록

	// 프레젠테이션 큐에 이미지 제출
	result = vkQueuePresentKHR(presentQueue, &presentInfo);

	// 프레젠테이션 실패 오류 발생 시
	// if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) { <-
	// framebufferResized는 명시적으로 해줄뿐 사실상 필요하지가 않음 나중에 수정할꺼면 하자
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		// 스왑 체인 크기와 surface의 크기가 호환되지 않는 경우
		recreateSwapChain(); // 변경된 surface에 맞는 SwapChain, ImageView, FrameBuffer 생성
	}
	else if (result != VK_SUCCESS)
	{
		// 진짜 오류 gg
		throw std::runtime_error("failed to present swap chain image!");
	}
	// [프레임 인덱스 증가]
	// 다음 작업할 프레임 변경
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::drawNoCamFrame()
{
	// [이전 GPU 작업 대기]
	// 동시에 작업 가능한 최대 Frame 개수만큼 작업 중인 경우 대기 (가장 먼저 시작한 Frame 작업이 끝나서 Fence에 signal을
	// 보내기를 기다림)
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	// [작업할 image 준비]
	// 이번 Frame 에서 사용할 이미지 준비 및 해당 이미지 index 받아오기 (준비가 끝나면 signal 보낼 세마포어 등록)
	// vkAcquireNextImageKHR 함수는 CPU에서 swapChain과 surface의 호환성을 확인하고 GPU에 이미지 준비 명령을 내리는 함수
	// 만약 image가 프레젠테이션 큐에 작업이 진행 중이거나 대기 중이면 해당 image는 사용하지 않고 대기한다.
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame],
											VK_NULL_HANDLE, &imageIndex);

	// image 준비 실패로 인한 오류 처리
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		// 스왑 체인이 surface 크기와 호환되지 않는 경우로(창 크기 변경), 스왑 체인 재생성 후 다시 draw
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		// 진짜 오류 gg
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	if (firstFrame)
	{
		ImGui::Begin("ViewPort");
		ImGui::Image(reinterpret_cast<ImTextureID>(noCamDescriptorSets[0]), ImVec2{viewPortSize.x, viewPortSize.y});
		ImGui::End();
		firstFrame = false;
	}
	else
	{
		ImGui::Begin("ViewPort");
		ImVec2 guiViewPortSize = ImGui::GetContentRegionAvail();
		if (guiViewPortSize.x != viewPortSize.x || guiViewPortSize.y != viewPortSize.y)
		{
			viewPortSize = glm::vec2(guiViewPortSize.x, guiViewPortSize.y);
			recreateViewPort();
		}
		ImGui::Image(reinterpret_cast<ImTextureID>(noCamDescriptorSets[0]), ImVec2{viewPortSize.x, viewPortSize.y});
		ImGui::End();
	}

	// [Fence 초기화]
	// Fence signal 상태 not signaled 로 초기화
	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	// [Command Buffer에 명령 기록]
	// 커맨드 버퍼 초기화 및 명령 기록
	vkResetCommandBuffer(
		commandBuffers[currentFrame],
		/*VkCommandBufferResetFlagBits*/ 0); // 두 번째 매개변수인 Flag 를 0으로 초기화하면 기본 초기화 진행

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	recordImGuiCommandBuffer(commandBuffers[currentFrame], imageIndex);

	if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to record deferred renderpass command buffer!");
	}

	// [렌더링 Command Buffer 제출]
	// 렌더링 커맨드 버퍼 제출 정보 객체 생성
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	// 작업 실행 신호를 받을 대기 세마포어 설정 (해당 세마포어가 signal 상태가 되기 전엔 대기)
	VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;			 // 대기 세마포어 개수
	submitInfo.pWaitSemaphores = waitSemaphores; // 대기 세마포어 등록
	submitInfo.pWaitDstStageMask = waitStages;	 // 대기할 시점 등록 (그 전까지는 세마포어 상관없이 그냥 진행)

	// 커맨드 버퍼 등록
	submitInfo.commandBufferCount = 1;							// 커맨드 버퍼 개수 등록
	submitInfo.pCommandBuffers = &commandBuffers[currentFrame]; // 커매드 버퍼 등록

	// 작업이 완료된 후 신호를 보낼 세마포어 설정 (작업이 끝나면 해당 세마포어 signal 상태로 변경)
	VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
	submitInfo.signalSemaphoreCount = 1;			 // 작업 끝나고 신호를 보낼 세마포어 개수
	submitInfo.pSignalSemaphores = signalSemaphores; // 작업 끝나고 신호를 보낼 세마포어 등록

	// 커맨드 버퍼 제출
	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	// [프레젠테이션 Command Buffer 제출]
	// 프레젠테이션 커맨드 버퍼 제출 정보 객체 생성
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	// 작업 실행 신호를 받을 대기 세마포어 설정
	presentInfo.waitSemaphoreCount = 1;				// 대기 세마포어 개수
	presentInfo.pWaitSemaphores = signalSemaphores; // 대기 세마포어 등록

	// 제출할 스왑 체인 설정
	VkSwapchainKHR swapChains[] = {swapChain};
	presentInfo.swapchainCount = 1;			 // 스왑체인 개수
	presentInfo.pSwapchains = swapChains;	 // 스왑체인 등록
	presentInfo.pImageIndices = &imageIndex; // 스왑체인에서 표시할 이미지 핸들 등록

	// 프레젠테이션 큐에 이미지 제출
	result = vkQueuePresentKHR(presentQueue, &presentInfo);

	// 프레젠테이션 실패 오류 발생 시
	// if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) { <-
	// framebufferResized는 명시적으로 해줄뿐 사실상 필요하지가 않음 나중에 수정할꺼면 하자
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
	{
		// 스왑 체인 크기와 surface의 크기가 호환되지 않는 경우
		recreateSwapChain(); // 변경된 surface에 맞는 SwapChain, ImageView, FrameBuffer 생성
	}
	else if (result != VK_SUCCESS)
	{
		// 진짜 오류 gg
		throw std::runtime_error("failed to present swap chain image!");
	}
	// [프레임 인덱스 증가]
	// 다음 작업할 프레임 변경
	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::recordImGuiCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	// 렌더 패스 시작
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = imGuiRenderPass;
	renderPassInfo.framebuffer = imGuiSwapChainFrameBuffers[imageIndex];
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = swapChainExtent;

	std::array<VkClearValue, 1> clearValues{};
	clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	ImGuiLayer::renderDrawData(commandBuffer);
	vkCmdEndRenderPass(commandBuffer);
}

/*
	[커맨드 버퍼에 작업 기록]
	1. 커맨드 버퍼 기록 시작
	2. 렌더패스 시작하는 명령 기록
	3. 파이프라인 설정 명령 기록
	4. 렌더링 명령 기록
	5. 렌더 패스 종료 명령 기록
	6. 커맨드 버퍼 기록 종료
*/
void Renderer::recordDeferredRenderPassCommandBuffer(Scene *scene, VkCommandBuffer commandBuffer, uint32_t imageIndex,
													 uint32_t shadowMapIndex)
{
	// 렌더 패스 시작
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = deferredRenderPass;
	renderPassInfo.framebuffer = viewPortFramebuffers[currentFrame];

	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = {static_cast<uint32_t>(viewPortSize.x), static_cast<uint32_t>(viewPortSize.y)};

	// ClearValues 수정
	std::array<VkClearValue, 6> clearValues{};
	// clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
	clearValues[0].color = {INFINITY, INFINITY, INFINITY, 1.0f};
	clearValues[1].color = {0.0f, 0.0f, 0.0f, 1.0f};
	clearValues[2].color = {0.0f, 0.0f, 0.0f, 1.0f};
	clearValues[3].color = {0.0f, 0.0f, 0.0f, 1.0f};
	clearValues[4].depthStencil = {1.0f, 0};
	clearValues[5].color = {0.0f, 0.0f, 0.0f, 1.0f};

	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, geometryPassGraphicsPipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = viewPortSize.x;
	viewport.height = viewPortSize.y;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = {static_cast<uint32_t>(viewPortSize.x), static_cast<uint32_t>(viewPortSize.y)};
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	DrawInfo drawInfo;
	drawInfo.currentFrame = currentFrame;
	drawInfo.pipelineLayout = geometryPassPipelineLayout;
	drawInfo.commandBuffer = commandBuffer;
	drawInfo.view = viewMatirx;
	drawInfo.projection = projMatrix;
	// drawInfo.projection = glm::perspective(glm::radians(45.0f), viewPortSize.x / viewPortSize.y, 0.01f, 100.0f);
	drawInfo.projection[1][1] *= -1;
	for (size_t i = 0; i < MAX_BONES; ++i) // 항등행렬 초기화
		drawInfo.finalBonesMatrices[i] = glm::mat4(1.0f);

	// int32_t drawNum = 0;
	auto view = scene->getAllEntitiesWith<TransformComponent, TagComponent, MeshRendererComponent>();
	for (auto entity : view)
	{
		if (!view.get<TagComponent>(entity).m_isActive || view.get<MeshRendererComponent>(entity).type == 0)
		{
			continue;
		}
		drawInfo.model = view.get<TransformComponent>(entity).m_WorldTransform;
		if (auto *sa = scene->tryGet<SkeletalAnimatorComponent>(entity)) // SA 컴포넌트 있으면 데이터 전달
		{
			auto *sac = (SAComponent *)sa->sac.get();
			std::vector<glm::mat4> matrices = sac->getCurrentPose();

			for (size_t i = 0; i < matrices.size(); ++i)
				drawInfo.finalBonesMatrices[i] = matrices[i];
		}
		MeshRendererComponent &meshRendererComponent = view.get<MeshRendererComponent>(entity);
		if (meshRendererComponent.cullState == ECullState::RENDER)
		{
			// drawNum++;
			meshRendererComponent.m_RenderingComponent->draw(drawInfo);
		}
	}

	// AL_CORE_INFO("draw Num = {}", drawNum);

	vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lightingPassGraphicsPipeline);

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lightingPassPipelineLayout, 0, 1,
							&lightingPassDescriptorSets[currentFrame], 0, nullptr);

	LightingPassUniformBufferObject lightingPassUbo{};

	std::memset(&lightingPassUbo, 0, sizeof(lightingPassUbo));
	// auto &lights = scene->getLights();

	// get light component
	auto lightView = scene->getAllEntitiesWith<TransformComponent, LightComponent>();
	size_t lightSize = 0;
	for (auto entity : lightView)
	{
		lightSize++;
	}

	uint32_t index = 0;
	size_t i = 0;
	for (auto entity : lightView)
	{
		std::shared_ptr<Light> light = lightView.get<LightComponent>(entity).m_Light;
		glm::vec3 lightPos = light->position;
		glm::vec3 lightDir = glm::normalize(light->direction);
		float outerCutoff = light->outerCutoff;
		lightingPassUbo.lights[i] = *light.get();
		glm::vec3 up = (glm::abs(lightDir.y) > 0.99f) ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);

		if (light->onShadowMap == 1 && index < shadowMapIndex)
		{
			if (light->type == 0)
			{
				lightingPassUbo.view[index][0] =
					glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
				lightingPassUbo.view[index][1] =
					glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
				lightingPassUbo.view[index][2] =
					glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
				lightingPassUbo.view[index][3] =
					glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
				lightingPassUbo.view[index][4] =
					glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
				lightingPassUbo.view[index][5] =
					glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));
				lightingPassUbo.proj[index] = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
				// lightingPassUbo.proj[index][1][1] *= -1;
			}
			else if (light->type == 1)
			{
				lightingPassUbo.view[index][0] = glm::lookAt(lightPos, lightPos + lightDir, up);
				lightingPassUbo.proj[index] = glm::perspective(glm::acos(outerCutoff) * 2.0f, 1.0f, 0.1f, 100.0f);
				lightingPassUbo.proj[index][1][1] *= -1;
			}
			else if (light->type == 2)
			{
				lightPos = glm::vec3(0.0f) - lightDir * 10.0f;
				lightingPassUbo.view[index][0] = glm::lookAt(lightPos, glm::vec3(0.0f), up);
				float orthoSize = 10.0f;
				lightingPassUbo.proj[index] = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, -10.0f, 20.0f);
				lightingPassUbo.proj[index][1][1] *= -1;
			}
			index++;
		}
		i++;
	}

	lightingPassUbo.numLights = static_cast<uint32_t>(lightSize);
	lightingPassUbo.cameraPos = scene->getCamPos();
	lightingPassUbo.ambientStrength = scene->getAmbientStrength();
	lightingPassFragmentUniformBuffers[currentFrame]->updateUniformBuffer(&lightingPassUbo, sizeof(lightingPassUbo));
	vkCmdDraw(commandBuffer, 6, 1, 0, 0);

	vkCmdEndRenderPass(commandBuffer);

	for (size_t i = 0; i < shadowMapIndex; i++)
	{
		VkImageMemoryBarrier barrierToDepthWrite{};
		barrierToDepthWrite.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrierToDepthWrite.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrierToDepthWrite.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		barrierToDepthWrite.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrierToDepthWrite.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		barrierToDepthWrite.image = m_shadowMapFrameBuffers[i]->getDepthImage();
		barrierToDepthWrite.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		barrierToDepthWrite.subresourceRange.baseMipLevel = 0;
		barrierToDepthWrite.subresourceRange.levelCount = 1;
		barrierToDepthWrite.subresourceRange.baseArrayLayer = 0;
		barrierToDepthWrite.subresourceRange.layerCount = 1;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
							 VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr, 1,
							 &barrierToDepthWrite);

		barrierToDepthWrite.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrierToDepthWrite.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrierToDepthWrite.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		barrierToDepthWrite.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrierToDepthWrite.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		barrierToDepthWrite.image = m_shadowCubeMapFrameBuffers[i]->getDepthImage();
		barrierToDepthWrite.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		barrierToDepthWrite.subresourceRange.baseMipLevel = 0;
		barrierToDepthWrite.subresourceRange.levelCount = 1;
		barrierToDepthWrite.subresourceRange.baseArrayLayer = 0;
		barrierToDepthWrite.subresourceRange.layerCount = 6;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
							 VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, 0, 0, nullptr, 0, nullptr, 1,
							 &barrierToDepthWrite);
	}
}

void Renderer::recordShadowMapCommandBuffer(Scene *scene, VkCommandBuffer commandBuffer, Light &lightInfo,
											uint32_t shadowMapIndex)
{
	// Clear 값 설정
	VkClearValue clearValue{};
	clearValue.depthStencil = {1.0f, 0};

	// Render Pass 시작 정보 설정
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = shadowMapRenderPass[shadowMapIndex];				  // Shadow Map 전용 RenderPass
	renderPassInfo.framebuffer = shadowMapFramebuffers[shadowMapIndex][currentFrame]; // 첫 번째 Framebuffer
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = {2048, 2048}; // 고정된 Shadow Map 크기
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearValue;

	// Render Pass 시작
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Shadow Map 파이프라인 바인딩
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowMapGraphicsPipeline[shadowMapIndex]);

	// Viewport 및 Scissor 설정
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = 2048.0f;
	viewport.height = 2048.0f;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = {2048, 2048};
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	// Depth Bias 설정
	vkCmdSetDepthBias(commandBuffer, 1.25f, 0.0f, 1.75f);

	glm::vec3 lightPos = lightInfo.position;
	glm::vec3 lightDir = glm::normalize(lightInfo.direction);
	float outerCutoff = lightInfo.outerCutoff;
	glm::vec3 up = (glm::abs(lightDir.y) > 0.99f) ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
	glm::mat4 lightView = glm::mat4(1.0f);
	glm::mat4 lightProj = glm::mat4(1.0f);
	if (lightInfo.type == 1)
	{ // spotlight
		lightView = glm::lookAt(lightPos, lightPos + lightDir, up);
		lightProj = glm::perspective(glm::acos(outerCutoff) * 2.0f, 1.0f, 0.1f, 100.0f);
		lightProj[1][1] *= -1;
	}
	else if (lightInfo.type == 2)
	{												   // directional light
		lightPos = glm::vec3(0.0f) - lightDir * 10.0f; // 광원을 기준으로 카메라처럼 뒤쪽으로 멀어짐
		// View 행렬 계산
		lightView = glm::lookAt(lightPos,		 // 광원이 가리키는 가상의 위치
								glm::vec3(0.0f), // 광원이 비추는 중심 (월드 좌표계 원점)
								up				 // 카메라의 상단 방향
		);
		// Projection 행렬 계산 (Orthographic)
		float orthoSize = 10.0f;					  // 광원의 영향을 받는 영역의 크기
		lightProj = glm::ortho(-orthoSize, orthoSize, // 좌/우 클립 경계
							   -orthoSize, orthoSize, // 아래/위 클립 경계
							   -10.0f, 20.0f		  // 근/원 클립 경계
		);
		// Vulkan 좌표계 보정
		lightProj[1][1] *= -1;
	}

	auto view = scene->getAllEntitiesWith<TransformComponent, TagComponent, MeshRendererComponent>();
	for (auto entity : view)
	{
		if (!view.get<TagComponent>(entity).m_isActive || view.get<MeshRendererComponent>(entity).type == 0)
		{
			continue;
		}

		ShadowMapDrawInfo drawInfo;
		drawInfo.view = lightView;
		drawInfo.projection = lightProj;
		drawInfo.commandBuffer = commandBuffer;
		drawInfo.pipelineLayout = shadowMapPipelineLayout[shadowMapIndex];
		drawInfo.currentFrame = currentFrame;
		drawInfo.model = view.get<TransformComponent>(entity).m_WorldTransform;

		MeshRendererComponent &meshRendererComponent = view.get<MeshRendererComponent>(entity);
		if (meshRendererComponent.cullState == ECullState::RENDER)
		{
			meshRendererComponent.m_RenderingComponent->drawShadow(drawInfo, shadowMapIndex);
		}
	}

	// Render Pass 종료
	vkCmdEndRenderPass(commandBuffer);

	VkImageMemoryBarrier barrierToShaderRead{};
	barrierToShaderRead.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrierToShaderRead.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	barrierToShaderRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrierToShaderRead.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	barrierToShaderRead.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	barrierToShaderRead.image = m_shadowMapFrameBuffers[shadowMapIndex]->getDepthImage();
	barrierToShaderRead.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	barrierToShaderRead.subresourceRange.baseMipLevel = 0;
	barrierToShaderRead.subresourceRange.levelCount = 1;
	barrierToShaderRead.subresourceRange.baseArrayLayer = 0;
	barrierToShaderRead.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
						 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrierToShaderRead);
}

void Renderer::recordShadowCubeMapCommandBuffer(Scene *scene, VkCommandBuffer commandBuffer, Light &lightInfo,
												uint32_t shadowMapIndex)
{

	VkClearValue clearValue{};
	clearValue.depthStencil = {1.0f, 0};

	// Render Pass 시작 정보 설정
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = shadowCubeMapRenderPass[shadowMapIndex];				  // Shadow Map 전용 RenderPass
	renderPassInfo.framebuffer = shadowCubeMapFramebuffers[shadowMapIndex][currentFrame]; // 첫 번째 Framebuffer
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = {2048, 2048}; // 고정된 Shadow Map 크기
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearValue;

	// Render Pass 시작
	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	// Shadow Map 파이프라인 바인딩
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowCubeMapGraphicsPipeline[shadowMapIndex]);

	// Viewport 및 Scissor 설정
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = 2048.0f;
	viewport.height = 2048.0f;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = {2048, 2048};
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	// Depth Bias 설정
	vkCmdSetDepthBias(commandBuffer, 1.25f, 0.0f, 1.75f);

	glm::vec3 lightPos = lightInfo.position;
	glm::mat4 lightProj = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
	// lightProj[1][1] *= -1;

	ShadowCubeMapDrawInfo drawInfo;
	drawInfo.view[0] = glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
	drawInfo.view[1] = glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
	drawInfo.view[2] = glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
	drawInfo.view[3] = glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
	drawInfo.view[4] = glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
	drawInfo.view[5] = glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));
	drawInfo.projection = lightProj;
	drawInfo.commandBuffer = commandBuffer;
	drawInfo.pipelineLayout = shadowCubeMapPipelineLayout[shadowMapIndex];
	drawInfo.currentFrame = currentFrame;

	auto view = scene->getAllEntitiesWith<TransformComponent, TagComponent, MeshRendererComponent>();
	for (auto entity : view)
	{
		if (!view.get<TagComponent>(entity).m_isActive || view.get<MeshRendererComponent>(entity).type == 0)
		{
			continue;
		}
		drawInfo.model = view.get<TransformComponent>(entity).m_WorldTransform;
		MeshRendererComponent &meshRendererComponent = view.get<MeshRendererComponent>(entity);
		if (meshRendererComponent.cullState == ECullState::RENDER)
		{
			meshRendererComponent.m_RenderingComponent->drawShadowCubeMap(drawInfo, shadowMapIndex);
		}
	}

	// Render Pass 종료
	vkCmdEndRenderPass(commandBuffer);

	VkImageMemoryBarrier barrierToShaderRead{};
	barrierToShaderRead.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrierToShaderRead.oldLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	barrierToShaderRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrierToShaderRead.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	barrierToShaderRead.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	barrierToShaderRead.image = m_shadowCubeMapFrameBuffers[shadowMapIndex]->getDepthImage();
	barrierToShaderRead.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	barrierToShaderRead.subresourceRange.baseMipLevel = 0;
	barrierToShaderRead.subresourceRange.levelCount = 1;
	barrierToShaderRead.subresourceRange.baseArrayLayer = 0;
	barrierToShaderRead.subresourceRange.layerCount = 6;

	vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
						 VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrierToShaderRead);
}

void Renderer::recordSphericalMapCommandBuffer()
{
	VkCommandBuffer commandBuffer = VulkanUtil::beginSingleTimeCommands(device, commandPool);
	VkClearValue clearValue{};
	clearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = sphericalMapRenderPass;
	renderPassInfo.framebuffer = sphericalMapFramebuffers[0];
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = {2048, 2048};
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearValue;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sphericalMapGraphicsPipeline);

	auto projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	projection[1][1] *= -1;
	std::vector<glm::mat4> views = {
		// **Right (+X) - layerIndex 0**
		glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),

		// **Left (-X) - layerIndex 1**
		glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f)),

		// **Top (+Y) - layerIndex 2**
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),

		// **Bottom (-Y) - layerIndex 3**
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),

		// **Front (+Z) - layerIndex 4**
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f)),

		// **Back (-Z) - layerIndex 5**
		glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f))};

	for (size_t i = 0; i < 6; i++)
	{
		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, sphericalMapPipelineLayout, 0, 1,
								&sphericalMapDescriptorSets[i], 0, nullptr);
		SphericalMapUniformBufferObject ubo;
		ubo.transform = projection * views[i];
		ubo.layerIndex = i;
		sphericalMapUniformBuffers[i]->updateUniformBuffer(&ubo, sizeof(ubo));
		vkCmdDraw(commandBuffer, 36, 1, 0, 0);
	}
	vkCmdEndRenderPass(commandBuffer);
	VulkanUtil::endSingleTimeCommands(device, graphicsQueue, commandPool, commandBuffer);
}

void Renderer::recordBackgroundCommandBuffer(VkCommandBuffer commandBuffer)
{
	VkClearValue clearValue{};
	clearValue.color = {0.0f, 0.0f, 0.0f, 1.0f};

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = backgroundRenderPass;
	renderPassInfo.framebuffer = backgroundFramebuffers[currentFrame];
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = {static_cast<uint32_t>(viewPortSize.x), static_cast<uint32_t>(viewPortSize.y)};
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearValue;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, backgroundGraphicsPipeline);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = viewPortSize.x;
	viewport.height = viewPortSize.y;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = {static_cast<uint32_t>(viewPortSize.x), static_cast<uint32_t>(viewPortSize.y)};
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), viewPortSize.x / viewPortSize.y, 0.01f, 100.0f);
	projection[1][1] *= -1;

	glm::mat4 view = viewMatirx;

	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, backgroundPipelineLayout, 0, 1,
							&backgroundDescriptorSets[currentFrame], 0, nullptr);

	BackgroundUniformBufferObject ubo{};
	ubo.proj = projection;
	ubo.view = view;
	backgroundUniformBuffers[currentFrame]->updateUniformBuffer(&ubo, sizeof(ubo));

	vkCmdDraw(commandBuffer, 36, 1, 0, 0);

	vkCmdEndRenderPass(commandBuffer);
}

} // namespace ale