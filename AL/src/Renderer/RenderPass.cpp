#include "Renderer/RenderPass.h"

namespace ale
{
std::unique_ptr<RenderPass> RenderPass::createRenderPass(VkFormat swapChainImageFormat) {
    std::unique_ptr<RenderPass> renderPass = std::unique_ptr<RenderPass>(new RenderPass());
    renderPass->initRenderPass(swapChainImageFormat);
    return renderPass;
}


void RenderPass::cleanup(){
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();

    vkDestroyRenderPass(device, renderPass, nullptr);
}


void RenderPass::initRenderPass(VkFormat swapChainImageFormat) {
    // [attachment 설정]
    // FrameBuffer의 attachment에 어떤 정보를 어떻게 기록할지 정하는 객체
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();
    VkSampleCountFlagBits msaaSamples = context.getMsaaSamples();

    // 멀티 샘플링 color attachment 설정
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;		 					// 이미지 포맷 (스왑 체인과 일치 시킴)
    colorAttachment.samples = msaaSamples;			 						// 샘플 개수 (멀티 샘플링을 위한 값 사용)
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;					// 렌더링 전 버퍼 클리어 (렌더링 시작 시 기존 attachment의 데이터 처리 방법)
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; 				// 렌더링 결과 저장 (렌더링 후 attachment를 메모리에 저장하는 방법 결정)
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; 		// 이전 데이터 무시 (스텐실 버퍼의 loadOp)
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; 		// 저장 x (스텐실 버퍼의 storeOp)
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; 				// 초기 레이아웃 설정을 UNDEFINED로 설정 (초기 데이터 가공을 하지 않기 때문에 가장 빠름)
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // color attachment 레이아웃 설정 

    // depth attachment 설정
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = VulkanUtil::findDepthFormat();
    depthAttachment.samples = msaaSamples;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;						// 초기 레이아웃 설정을 UNDEFINED로 설정
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // 최종 레이아웃 depth-stencil buffer로 사용

    // resolve attachment 설정
    // 멀티 샘플링 attachment를 단일 샘플링 attachment로 전환
    VkAttachmentDescription colorAttachmentResolve{};
    colorAttachmentResolve.format = swapChainImageFormat;
    colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;


    VkAttachmentDescription swapChainAttachment{};
    swapChainAttachment.format = swapChainImageFormat;
    swapChainAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    swapChainAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    swapChainAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    swapChainAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    swapChainAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    swapChainAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    swapChainAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;



    // subpass가 attachment 설정 어떤 것을 어떻게 참조할지 정의
    // color attachment
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0; 										// 특정 attachment 설정의 index
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// attachment 설정을 subpass 내에서
                                                                            // 어떤 layout으로 쓸지 결정 (현재는 color attachment로 사용하는 설정)
    // depth attachment
    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // resolve attachment
    VkAttachmentReference colorAttachmentResolveRef{};
    colorAttachmentResolveRef.attachment = 2;
    colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // swap chain attachment
    VkAttachmentReference swapChainAttachmentRef{};
    swapChainAttachmentRef.attachment = 3;
    swapChainAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // [subpass 정의]
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1; 										// attachment 설정 개수 등록
    subpass.pColorAttachments = &colorAttachmentRef;						// color attachment 등록
    subpass.pDepthStencilAttachment = &depthAttachmentRef;					// depth attachment 등록
    subpass.pResolveAttachments = &swapChainAttachmentRef;				// resolve attachment 등록


    // [subpass 종속성 설정]
    // 렌더패스 외부 작업(srcSubpass)과 0번 서브패스(dstSubpass) 간의 동기화 설정.
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;	// 렌더패스 외부 작업(이전 프레임 처리 또는 렌더패스 외부의 GPU 작업)
    dependency.dstSubpass = 0;					 	// 첫 번째 서브패스(0번 서브패스)에 종속
    // srcStageMask: 동기화를 기다릴 렌더패스 외부 작업의 파이프라인 단계
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;	// 색상 첨부물 출력 단계 | 프래그먼트 테스트의 최종 단계
    // srcAccessMask: 렌더패스 외부 작업에서 보장해야 할 메모리 접근 권한
    dependency.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;												// 깊이/스텐실 첨부물 쓰기 권한
    // dstStageMask: 0번 서브패스에서 동기화를 기다릴 파이프라인 단계
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;	// 색상 첨부물 출력 단계 | 프래그먼트 테스트의 초기 단계
    // dstAccessMask: 0번 서브패스에서 필요한 메모리 접근 권한
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;			// 색상 첨부물 쓰기 권한 | 깊이/스텐실 첨부물 쓰기 권한

    // [렌더 패스 정의]
    std::array<VkAttachmentDescription, 4> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve, swapChainAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size()); // attachment 설정 개수 등록
    renderPassInfo.pAttachments = attachments.data();							// attachment 설정 등록
    renderPassInfo.subpassCount = 1;											// subpass 개수 등록
    renderPassInfo.pSubpasses = &subpass;										// subpass 등록
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;
    
    // [렌더 패스 생성]
    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}


std::unique_ptr<RenderPass> RenderPass::createDeferredRenderPass(VkFormat swapChainImageFormat) {
    std::unique_ptr<RenderPass> renderPass = std::unique_ptr<RenderPass>(new RenderPass());
    renderPass->initDeferredRenderPass(swapChainImageFormat);
    return renderPass;
}


void RenderPass::initDeferredRenderPass(VkFormat swapChainImageFormat) {
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();    

    // Position Attachment
    VkAttachmentDescription positionAttachment{};
    positionAttachment.format = VK_FORMAT_R16G16B16A16_SFLOAT;
    positionAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    positionAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    positionAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    positionAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    positionAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // Normal Attachment
    VkAttachmentDescription normalAttachment = positionAttachment;

    // Albedo Attachment
    VkAttachmentDescription albedoAttachment{};
    albedoAttachment.format = VK_FORMAT_R8G8B8A8_UNORM;
    albedoAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    albedoAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    albedoAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    albedoAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    albedoAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    // PBR Attachment
    VkAttachmentDescription pbrAttachment = albedoAttachment;

    // Depth Attachment
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = VulkanUtil::findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // SwapChain Attachment
    VkAttachmentDescription swapChainAttachment{};
    swapChainAttachment.format = swapChainImageFormat;
    swapChainAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    swapChainAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    swapChainAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    swapChainAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    swapChainAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Subpass 1 - Geometry Pass
    VkAttachmentReference positionRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference normalRef{1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference albedoRef{2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference pbrRef{3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkAttachmentReference depthRef{4, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

    VkAttachmentReference geometryAttachments[] = {positionRef, normalRef, albedoRef, pbrRef};

    VkSubpassDescription subpass1{};
    subpass1.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass1.colorAttachmentCount = 4;
    subpass1.pColorAttachments = geometryAttachments;
    subpass1.pDepthStencilAttachment = &depthRef;

    // Subpass 2 - Lighting Pass
    VkAttachmentReference positionInputRef{0, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkAttachmentReference normalInputRef{1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkAttachmentReference albedoInputRef{2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkAttachmentReference pbrInputRef{3, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkAttachmentReference swapChainRef{5, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkAttachmentReference inputAttachments[] = {positionInputRef, normalInputRef, albedoInputRef, pbrInputRef};

    VkSubpassDescription subpass2{};
    subpass2.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass2.inputAttachmentCount = 4;
    subpass2.pInputAttachments = inputAttachments;
    subpass2.colorAttachmentCount = 1;
    subpass2.pColorAttachments = &swapChainRef;

    // Subpass Dependencies
    std::array<VkSubpassDependency, 2> dependencies{};
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = 0;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = 1;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    // Render Pass 생성
    std::array<VkAttachmentDescription, 6> attachments = {
        positionAttachment, normalAttachment, albedoAttachment, pbrAttachment,
        depthAttachment, swapChainAttachment
    };

    VkSubpassDescription subpasses[] = {subpass1, subpass2};

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 2;
    renderPassInfo.pSubpasses = subpasses;
    renderPassInfo.dependencyCount = 2;
    renderPassInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create deferred render pass!");
    }
}


std::unique_ptr<RenderPass> RenderPass::createShadowMapRenderPass() {
    std::unique_ptr<RenderPass> renderPass = std::unique_ptr<RenderPass>(new RenderPass());
    renderPass->initShadowMapRenderPass();
    return renderPass;
}


void RenderPass::initShadowMapRenderPass() {
    auto& context = VulkanContext::getContext();
    VkDevice device = context.getDevice();

    // Depth Attachment 설정
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = VulkanUtil::findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    // Subpass 설정
    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 0; // Depth Attachment는 첫 번째 (0번째)
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 0; // Color Attachment 없음
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    // Subpass 종속성 설정
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    // RenderPass 생성
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &depthAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shadow map render pass!");
    }
}

} // namespace ale