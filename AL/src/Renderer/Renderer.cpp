#include "Renderer/Renderer.h"
#include "ALpch.h"
#include "ImGui/ImGuiLayer.h"

namespace ale
{
std::unique_ptr<Renderer> Renderer::createRenderer(GLFWwindow* window) {
    std::unique_ptr<Renderer> renderer = std::unique_ptr<Renderer>(new Renderer());
    renderer->init(window);
    return renderer;
}


void Renderer::init(GLFWwindow* window) {
    this->window = window;

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

    m_swapChain = SwapChain::createSwapChain(window);
    swapChain = m_swapChain->getSwapChain();
    swapChainImages = m_swapChain->getSwapChainImages();
    swapChainImageFormat = m_swapChain->getSwapChainImageFormat();
    swapChainExtent = m_swapChain->getSwapChainExtent();
    swapChainImageViews = m_swapChain->getSwapChainImageViews();


    m_syncObjects = SyncObjects::createSyncObjects();
    imageAvailableSemaphores = m_syncObjects->getImageAvailableSemaphores();
    renderFinishedSemaphores = m_syncObjects->getRenderFinishedSemaphores();
    inFlightFences = m_syncObjects->getInFlightFences();

    for (size_t i = 0; i < 4; i++) {
        m_shadowMapRenderPass.push_back(RenderPass::createShadowMapRenderPass());
        shadowMapRenderPass.push_back(m_shadowMapRenderPass[i]->getRenderPass());
    }

    for (size_t i = 0; i < 4; i++) {
        m_shadowCubeMapRenderPass.push_back(RenderPass::createShadowMapRenderPass());
        shadowCubeMapRenderPass.push_back(m_shadowCubeMapRenderPass[i]->getRenderPass());
    }


    m_shadowMapDescriptorSetLayout = DescriptorSetLayout::createShadowMapDescriptorSetLayout();
    shadowMapDescriptorSetLayout = m_shadowMapDescriptorSetLayout->getDescriptorSetLayout();
    context.setShadowMapDescriptorSetLayout(shadowMapDescriptorSetLayout);

    m_shadowCubeMapDescriptorSetLayout = DescriptorSetLayout::createShadowCubeMapDescriptorSetLayout();
    shadowCubeMapDescriptorSetLayout = m_shadowCubeMapDescriptorSetLayout->getDescriptorSetLayout();
    context.setShadowCubeMapDescriptorSetLayout(shadowCubeMapDescriptorSetLayout);

    for (size_t i = 0; i < 4; i++) {
        m_shadowMapFrameBuffers.push_back(FrameBuffers::createShadowMapFrameBuffers(shadowMapRenderPass[i]));
        shadowMapFramebuffers.push_back(m_shadowMapFrameBuffers[i]->getFramebuffers());
        shadowMapImageViews.push_back(m_shadowMapFrameBuffers[i]->getDepthImageView());
    }

    for (size_t i = 0; i < 4; i++) {
        m_shadowCubeMapFrameBuffers.push_back(FrameBuffers::createShadowCubeMapFrameBuffers(shadowCubeMapRenderPass[i]));
        shadowCubeMapFramebuffers.push_back(m_shadowCubeMapFrameBuffers[i]->getFramebuffers());
        shadowCubeMapImageViews.push_back(m_shadowCubeMapFrameBuffers[i]->getDepthImageView());
    }

    shadowMapSampler = Texture::createShadowMapSampler();
    shadowCubeMapSampler = Texture::createShadowCubeMapSampler();

    for (size_t i = 0; i < 4; i++) {
        m_shadowMapPipeline.push_back(Pipeline::createShadowMapPipeline(shadowMapRenderPass[i], shadowMapDescriptorSetLayout));
        shadowMapPipelineLayout.push_back(m_shadowMapPipeline[i]->getPipelineLayout());
        shadowMapGraphicsPipeline.push_back(m_shadowMapPipeline[i]->getPipeline());
    }

    for (size_t i = 0; i < 4; i++) {
        m_shadowCubeMapPipeline.push_back(Pipeline::createShadowCubeMapPipeline(shadowCubeMapRenderPass[i], shadowCubeMapDescriptorSetLayout));
        shadowCubeMapPipelineLayout.push_back(m_shadowCubeMapPipeline[i]->getPipelineLayout());
        shadowCubeMapGraphicsPipeline.push_back(m_shadowCubeMapPipeline[i]->getPipeline());
    }

    m_deferredRenderPass = RenderPass::createDeferredRenderPass(swapChainImageFormat);
    deferredRenderPass = m_deferredRenderPass->getRenderPass();

    m_swapChainFrameBuffers = FrameBuffers::createSwapChainFrameBuffers(m_swapChain.get(), deferredRenderPass);
    swapChainFramebuffers = m_swapChainFrameBuffers->getFramebuffers();

    m_geometryPassDescriptorSetLayout = DescriptorSetLayout::createGeometryPassDescriptorSetLayout();
    geometryPassDescriptorSetLayout = m_geometryPassDescriptorSetLayout->getDescriptorSetLayout();
    context.setGeometryPassDescriptorSetLayout(geometryPassDescriptorSetLayout);

    m_lightingPassDescriptorSetLayout = DescriptorSetLayout::createLightingPassDescriptorSetLayout();
    lightingPassDescriptorSetLayout = m_lightingPassDescriptorSetLayout->getDescriptorSetLayout();
    
    m_lightingPassShaderResourceManager = ShaderResourceManager::createLightingPassShaderResourceManager(lightingPassDescriptorSetLayout, 
    m_swapChainFrameBuffers->getPositionImageView(), m_swapChainFrameBuffers->getNormalImageView(), m_swapChainFrameBuffers->getAlbedoImageView(), 
    m_swapChainFrameBuffers->getPbrImageView(), shadowMapImageViews, shadowMapSampler, shadowCubeMapImageViews, shadowCubeMapSampler);
    lightingPassDescriptorSets = m_lightingPassShaderResourceManager->getDescriptorSets();
    lightingPassFragmentUniformBuffers = m_lightingPassShaderResourceManager->getFragmentUniformBuffers();
    m_geometryPassPipeline = Pipeline::createGeometryPassPipeline(deferredRenderPass, geometryPassDescriptorSetLayout);
    geometryPassPipelineLayout = m_geometryPassPipeline->getPipelineLayout();
    geometryPassGraphicsPipeline = m_geometryPassPipeline->getPipeline();
    m_lightingPassPipeline = Pipeline::createLightingPassPipeline(deferredRenderPass, lightingPassDescriptorSetLayout);
    lightingPassPipelineLayout = m_lightingPassPipeline->getPipelineLayout();
    lightingPassGraphicsPipeline = m_lightingPassPipeline->getPipeline();
    m_commandBuffers = CommandBuffers::createCommandBuffers();
    commandBuffers = m_commandBuffers->getCommandBuffers();

}


void Renderer::cleanup() {
    m_swapChainFrameBuffers->cleanup();
    m_swapChain->cleanup();

    m_geometryPassPipeline->cleanup();
    m_lightingPassPipeline->cleanup();

    m_deferredRenderPass->cleanup();

    m_lightingPassShaderResourceManager->cleanup();

    m_geometryPassDescriptorSetLayout->cleanup();
    m_lightingPassDescriptorSetLayout->cleanup();

    m_syncObjects->cleanup();
    VulkanContext::getContext().cleanup();
}


void Renderer::loadScene(Scene* scene) {
    // this->scene = scene;
    // m_geometryPassShaderResourceManager = ShaderResourceManager::createGeometryPassShaderResourceManager(scene, geometryPassDescriptorSetLayout);
    // geometryPassDescriptorSets = m_geometryPassShaderResourceManager->getDescriptorSets();
    // geometryPassVertexUniformBuffers = m_geometryPassShaderResourceManager->getVertexUniformBuffers();
    // geometryPassFragmentUniformBuffers = m_geometryPassShaderResourceManager->getFragmentUniformBuffers();
}


void Renderer::drawFrame(Scene* scene) {
    // [이전 GPU 작업 대기]
    // 동시에 작업 가능한 최대 Frame 개수만큼 작업 중인 경우 대기 (가장 먼저 시작한 Frame 작업이 끝나서 Fence에 signal을 보내기를 기다림)
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    // [작업할 image 준비]
    // 이번 Frame 에서 사용할 이미지 준비 및 해당 이미지 index 받아오기 (준비가 끝나면 signal 보낼 세마포어 등록)
    // vkAcquireNextImageKHR 함수는 CPU에서 swapChain과 surface의 호환성을 확인하고 GPU에 이미지 준비 명령을 내리는 함수
    // 만약 image가 프레젠테이션 큐에 작업이 진행 중이거나 대기 중이면 해당 image는 사용하지 않고 대기한다.
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    // image 준비 실패로 인한 오류 처리
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // 스왑 체인이 surface 크기와 호환되지 않는 경우로(창 크기 변경), 스왑 체인 재생성 후 다시 draw
        recreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        // 진짜 오류 gg
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    // [Fence 초기화]
    // Fence signal 상태 not signaled 로 초기화
    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    // [Command Buffer에 명령 기록]
    // 커맨드 버퍼 초기화 및 명령 기록
    vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0); // 두 번째 매개변수인 Flag 를 0으로 초기화하면 기본 초기화 진행
    // recordCommandBuffer(scene, commandBuffers[currentFrame], imageIndex); // 현재 작업할 image의 index와 commandBuffer를 전송
    
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
    

    uint32_t shadowMapIndex = 0;
    for (size_t i = 0; i < scene->getLights().size() && shadowMapIndex < 4; i++) {
        if (scene->getLights()[i].onShadowMap == 1) {
            recordShadowMapCommandBuffer(scene, commandBuffers[currentFrame], i, shadowMapIndex);
            recordShadowCubeMapCommandBuffer(scene, commandBuffers[currentFrame], i, shadowMapIndex);
            shadowMapIndex++;
        }
    }
    recordDeferredRenderPassCommandBuffer(scene, commandBuffers[currentFrame], imageIndex, shadowMapIndex); // 현재 작업할 image의 index와 commandBuffer를 전송


    if (vkEndCommandBuffer(commandBuffers[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record deferred renderpass command buffer!");
    }
    // [렌더링 Command Buffer 제출]
    // 렌더링 커맨드 버퍼 제출 정보 객체 생성
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // 작업 실행 신호를 받을 대기 세마포어 설정 (해당 세마포어가 signal 상태가 되기 전엔 대기)
    VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};				
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT}; 	
    submitInfo.waitSemaphoreCount = 1;														// 대기 세마포어 개수
    submitInfo.pWaitSemaphores = waitSemaphores;											// 대기 세마포어 등록
    submitInfo.pWaitDstStageMask = waitStages;												// 대기할 시점 등록 (그 전까지는 세마포어 상관없이 그냥 진행)	

    // 커맨드 버퍼 등록
    submitInfo.commandBufferCount = 1;														// 커맨드 버퍼 개수 등록
    submitInfo.pCommandBuffers = &commandBuffers[currentFrame];								// 커매드 버퍼 등록

    // 작업이 완료된 후 신호를 보낼 세마포어 설정 (작업이 끝나면 해당 세마포어 signal 상태로 변경)
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
    submitInfo.signalSemaphoreCount = 1;													// 작업 끝나고 신호를 보낼 세마포어 개수
    submitInfo.pSignalSemaphores = signalSemaphores;										// 작업 끝나고 신호를 보낼 세마포어 등록

    // 커맨드 버퍼 제출 
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    // [프레젠테이션 Command Buffer 제출]
    // 프레젠테이션 커맨드 버퍼 제출 정보 객체 생성
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    // 작업 실행 신호를 받을 대기 세마포어 설정
    presentInfo.waitSemaphoreCount = 1;														// 대기 세마포어 개수
    presentInfo.pWaitSemaphores = signalSemaphores;											// 대기 세마포어 등록

    // 제출할 스왑 체인 설정
    VkSwapchainKHR swapChains[] = {swapChain};
    presentInfo.swapchainCount = 1;															// 스왑체인 개수
    presentInfo.pSwapchains = swapChains;													// 스왑체인 등록
    presentInfo.pImageIndices = &imageIndex;												// 스왑체인에서 표시할 이미지 핸들 등록
    // 프레젠테이션 큐에 이미지 제출
    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    // 프레젠테이션 실패 오류 발생 시
    // if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) { <- framebufferResized는 명시적으로 해줄뿐 사실상 필요하지가 않음 나중에 수정할꺼면 하자
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        // 스왑 체인 크기와 surface의 크기가 호환되지 않는 경우
        recreateSwapChain(); 	// 변경된 surface에 맞는 SwapChain, ImageView, FrameBuffer 생성 
    } else if (result != VK_SUCCESS) {
        // 진짜 오류 gg
        throw std::runtime_error("failed to present swap chain image!");
    }
    // [프레임 인덱스 증가]
    // 다음 작업할 프레임 변경
    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}


/*
    변경된 window 크기에 맞게 SwapChain, ImageView, FrameBuffer 재생성
*/
void Renderer::recreateSwapChain() {
    // 현재 프레임버퍼 사이즈 체크
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    
    // 현재 프레임 버퍼 사이즈가 0이면 다음 이벤트 호출까지 대기
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents(); // 다음 이벤트 발생 전까지 대기하여 CPU 사용률을 줄이는 함수 
    }

    // 모든 GPU 작업 종료될 때까지 대기 (사용중인 리소스를 건들지 않기 위해)
    vkDeviceWaitIdle(device);

    // 스왑 체인 관련 리소스 정리
    m_lightingPassShaderResourceManager->cleanup();
    m_geometryPassPipeline->cleanup();
    m_lightingPassPipeline->cleanup();
    m_swapChainFrameBuffers->cleanup();
    m_deferredRenderPass->cleanup();
    m_swapChain->recreateSwapChain();

    swapChain = m_swapChain->getSwapChain();
    swapChainImages = m_swapChain->getSwapChainImages();
    swapChainImageFormat = m_swapChain->getSwapChainImageFormat();
    swapChainExtent = m_swapChain->getSwapChainExtent();
    swapChainImageViews = m_swapChain->getSwapChainImageViews();

    m_deferredRenderPass = RenderPass::createDeferredRenderPass(swapChainImageFormat);
    deferredRenderPass = m_deferredRenderPass->getRenderPass();

    m_swapChainFrameBuffers->initSwapChainFrameBuffers(m_swapChain.get(), deferredRenderPass);
    swapChainFramebuffers = m_swapChainFrameBuffers->getFramebuffers();

    m_geometryPassPipeline = Pipeline::createGeometryPassPipeline(deferredRenderPass, geometryPassDescriptorSetLayout);
    geometryPassPipelineLayout = m_geometryPassPipeline->getPipelineLayout();
    geometryPassGraphicsPipeline = m_geometryPassPipeline->getPipeline();

    m_lightingPassPipeline = Pipeline::createLightingPassPipeline(deferredRenderPass, lightingPassDescriptorSetLayout);
    lightingPassPipelineLayout = m_lightingPassPipeline->getPipelineLayout();
    lightingPassGraphicsPipeline = m_lightingPassPipeline->getPipeline();

    m_lightingPassShaderResourceManager = ShaderResourceManager::createLightingPassShaderResourceManager(lightingPassDescriptorSetLayout,
    m_swapChainFrameBuffers->getPositionImageView(), m_swapChainFrameBuffers->getNormalImageView(), m_swapChainFrameBuffers->getAlbedoImageView(), 
    m_swapChainFrameBuffers->getPbrImageView(), shadowMapImageViews, shadowMapSampler, shadowCubeMapImageViews, shadowCubeMapSampler);
    lightingPassDescriptorSets = m_lightingPassShaderResourceManager->getDescriptorSets();
    lightingPassFragmentUniformBuffers = m_lightingPassShaderResourceManager->getFragmentUniformBuffers();
}


void Renderer::recordDeferredRenderPassCommandBuffer(Scene* scene, VkCommandBuffer commandBuffer, uint32_t imageIndex, uint32_t shadowMapIndex) {
    // 렌더 패스 시작
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = deferredRenderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChainExtent;

    // ClearValues 수정
    std::array<VkClearValue, 6> clearValues{};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
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
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    const std::vector<std::shared_ptr<Object>>& objects = scene->getObjects();
    size_t objectCount = scene->getObjectCount();

    DrawInfo drawInfo;
    drawInfo.currentFrame = currentFrame;
    drawInfo.pipelineLayout = geometryPassPipelineLayout;
    drawInfo.commandBuffer = commandBuffer;
    drawInfo.view = scene->getViewMatrix();
    drawInfo.projection = scene->getProjMatrix(swapChainExtent);
    drawInfo.projection[1][1] *= -1;
    for (size_t i = 0; i < objectCount; i++) {
        objects[i]->draw(drawInfo);
    }

    const std::vector<std::shared_ptr<Object>>& lightObjects = scene->getLightObjects();
    size_t lightObjectCount = lightObjects.size();

    for (size_t i = 0; i < lightObjectCount; i++) {
        lightObjects[i]->draw(drawInfo);
    }

    vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, lightingPassGraphicsPipeline);

    vkCmdBindDescriptorSets(
        commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        lightingPassPipelineLayout,
        0,
        1,
        &lightingPassDescriptorSets[currentFrame],
        0,
        nullptr
    );

    LightingPassUniformBufferObject lightingPassUbo{};

    std::memset(&lightingPassUbo, 0, sizeof(lightingPassUbo));
    auto& lights = scene->getLights();
    uint32_t index = 0;
    for (size_t i = 0; i < lights.size(); i++) {
        glm::vec3 lightPos = lights[i].position;
        glm::vec3 lightDir = glm::normalize(lights[i].direction);
        float outerCutoff = lights[i].outerCutoff;
        lightingPassUbo.lights[i] = lights[i];
        glm::vec3 up = (glm::abs(lightDir.y) > 0.99f) ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);

        if (lights[i].onShadowMap == 1 && index < shadowMapIndex) {
            if (lights[i].type == 0) {
                lightingPassUbo.view[index][0] = glm::lookAt(lightPos, lightPos + glm::vec3(1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
                lightingPassUbo.view[index][1] = glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0, -1.0, 0.0));
                lightingPassUbo.view[index][2] = glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0));
                lightingPassUbo.view[index][3] = glm::lookAt(lightPos, lightPos + glm::vec3(0.0, -1.0, 0.0), glm::vec3(0.0, 0.0, -1.0));
                lightingPassUbo.view[index][4] = glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, 1.0), glm::vec3(0.0, -1.0, 0.0));
                lightingPassUbo.view[index][5] = glm::lookAt(lightPos, lightPos + glm::vec3(0.0, 0.0, -1.0), glm::vec3(0.0, -1.0, 0.0));
                lightingPassUbo.proj[index] = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);
                // lightingPassUbo.proj[index][1][1] *= -1;
            }
            else if (lights[i].type == 1) {
                lightingPassUbo.view[index][0] = glm::lookAt(
                    lightPos,
                    lightPos + lightDir,
                    up
                );
                lightingPassUbo.proj[index] = glm::perspective(
                    glm::acos(outerCutoff) * 2.0f,
                    1.0f,
                    0.1f,
                    100.0f
                );
                lightingPassUbo.proj[index][1][1] *= -1;
            }
            else if (lights[i].type == 2) {
                lightPos = glm::vec3(0.0f) - lightDir * 10.0f;
                lightingPassUbo.view[index][0] = glm::lookAt(
                    lightPos,
                    glm::vec3(0.0f),
                    up
                );
                float orthoSize = 10.0f;
                lightingPassUbo.proj[index] = glm::ortho(
                    -orthoSize, orthoSize,
                    -orthoSize, orthoSize,
                    -10.0f, 20.0f
                );
                lightingPassUbo.proj[index][1][1] *= -1;
            }
            index++;
        }
    }
    lightingPassUbo.numLights = static_cast<uint32_t>(lights.size());
    lightingPassUbo.cameraPos = scene->getCamPos();
    lightingPassUbo.ambientStrength = scene->getAmbientStrength();
    lightingPassFragmentUniformBuffers[currentFrame]->updateUniformBuffer(&lightingPassUbo, sizeof(lightingPassUbo));
    vkCmdDraw(commandBuffer, 6, 1, 0, 0);

	ImGuiLayer::renderDrawData(scene, commandBuffer);

    vkCmdEndRenderPass(commandBuffer);

    for (size_t i = 0; i < shadowMapIndex; i++) {
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

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrierToDepthWrite
        );
    }


    VkImageMemoryBarrier barrierToDepthWrite{};
    barrierToDepthWrite.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrierToDepthWrite.oldLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrierToDepthWrite.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    barrierToDepthWrite.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    barrierToDepthWrite.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    barrierToDepthWrite.image = m_shadowCubeMapFrameBuffers[0]->getDepthImage();
    barrierToDepthWrite.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    barrierToDepthWrite.subresourceRange.baseMipLevel = 0;
    barrierToDepthWrite.subresourceRange.levelCount = 1;
    barrierToDepthWrite.subresourceRange.baseArrayLayer = 0;
    barrierToDepthWrite.subresourceRange.layerCount = 1;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrierToDepthWrite
    );
}

void Renderer::recordShadowMapCommandBuffer(Scene* scene, VkCommandBuffer commandBuffer, uint32_t lightIndex, uint32_t shadowMapIndex) {
   // Clear 값 설정
    VkClearValue clearValue{};
    clearValue.depthStencil = {1.0f, 0};

    // Render Pass 시작 정보 설정
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = shadowMapRenderPass[shadowMapIndex]; // Shadow Map 전용 RenderPass
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

    auto& objects = scene->getObjects();
    size_t objectCount = objects.size();
    auto& Lights = scene->getLights();
    auto& lightInfo = Lights[lightIndex];

    glm::vec3 lightPos = lightInfo.position;
    glm::vec3 lightDir = glm::normalize(lightInfo.direction);
    float outerCutoff = lightInfo.outerCutoff;
    glm::vec3 up = (glm::abs(lightDir.y) > 0.99f) ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 lightView = glm::mat4(1.0f);
    glm::mat4 lightProj = glm::mat4(1.0f);
    if (lightInfo.type == 1) { // spotlight
        lightView = glm::lookAt(
            lightPos,
            lightPos + lightDir,
            up
        );
        lightProj = glm::perspective(
            glm::acos(outerCutoff) * 2.0f,
            1.0f,
            0.1f,
            100.0f
        );
        lightProj[1][1] *= -1;
    }
    else if (lightInfo.type == 2) { // directional light
        lightPos = glm::vec3(0.0f) - lightDir * 10.0f; // 광원을 기준으로 카메라처럼 뒤쪽으로 멀어짐
        // View 행렬 계산
        lightView = glm::lookAt(
            lightPos,             // 광원이 가리키는 가상의 위치
            glm::vec3(0.0f),      // 광원이 비추는 중심 (월드 좌표계 원점)
            up                    // 카메라의 상단 방향
        );
        // Projection 행렬 계산 (Orthographic)
        float orthoSize = 10.0f; // 광원의 영향을 받는 영역의 크기
        lightProj = glm::ortho(
            -orthoSize, orthoSize, // 좌/우 클립 경계
            -orthoSize, orthoSize, // 아래/위 클립 경계
            -10.0f, 20.0f          // 근/원 클립 경계
        );
        // Vulkan 좌표계 보정
        lightProj[1][1] *= -1;
    }
    
    for (size_t i = 0; i < objectCount; i++) {
        ShadowMapDrawInfo drawInfo;
        drawInfo.view = lightView;
        drawInfo.projection = lightProj;
        drawInfo.commandBuffer = commandBuffer;
        drawInfo.pipelineLayout = shadowMapPipelineLayout[shadowMapIndex];
        drawInfo.currentFrame = currentFrame;
        objects[i]->drawShadow(drawInfo, shadowMapIndex);
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

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrierToShaderRead
    );

}

void Renderer::recordShadowCubeMapCommandBuffer(Scene* scene, VkCommandBuffer commandBuffer, uint32_t lightIndex, uint32_t shadowMapIndex) {

    VkClearValue clearValue{};
    clearValue.depthStencil = {1.0f, 0};

    // Render Pass 시작 정보 설정
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = shadowCubeMapRenderPass[shadowMapIndex]; // Shadow Map 전용 RenderPass
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

    auto& objects = scene->getObjects();
    size_t objectCount = objects.size();
    auto& Lights = scene->getLights();
    auto& lightInfo = Lights[lightIndex];

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
    for (size_t i = 0; i < objectCount; i++) {
        objects[i]->drawShadowCubeMap(drawInfo, shadowMapIndex);
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

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrierToShaderRead
    );

}

} // namespace ale