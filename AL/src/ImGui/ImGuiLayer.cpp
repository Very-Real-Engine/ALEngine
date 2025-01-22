#include "ImGui/ImGuiLayer.h"
#include "ALpch.h"
#include "Core/App.h"

#include "Events/AppEvent.h"
#include "imgui/imgui.h"
#include "glm/gtc/type_ptr.hpp"

namespace ale
{
ImGuiLayer::ImGuiLayer() : Layer("ImGuiLayer")
{
	auto &context = VulkanContext::getContext();
	commandPool = context.getCommandPool();
	init_info.Instance = context.getInstance();								  // VulkanContext
	init_info.PhysicalDevice = context.getPhysicalDevice();					  // renderer
	init_info.Device = context.getDevice();									  // renderer
	init_info.QueueFamily = context.getQueueFamily(); // indices.graphicsFamily.value()
	init_info.Queue = context.getGraphicsQueue();							  // renderer
	init_info.PipelineCache = VK_NULL_HANDLE;								  // vk null handle
	init_info.Subpass = 1;

	// gui용 descriptor pool 필요
	init_info.DescriptorPool = context.getDescriptorPool(); // renderer
	// >= 2
	init_info.MinImageCount = 2; // 2
	// >= MinImageCount
	init_info.ImageCount = init_info.MinImageCount + 1; // min_ImageCount + 1
	// >= VK_SAMPLE_COUNT_1_BIT (0 -> default to VK_SAMPLE_COUNT_1_BIT)
	init_info.Allocator = nullptr;				   // nullptr
}

void ImGuiLayer::onAttach()
{
	AL_CORE_INFO("ImGuiLayer::onAttach");

	ImGui::CreateContext();

	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;	// Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
	// io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
	// io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular
	// ones.
	ImGuiStyle &style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	App &app = App::get();
	Renderer &renderer = app.getRenderer();

	// ImGui Glfw 초기화
	ImGui_ImplGlfw_InitForVulkan(app.getWindow().getNativeWindow(), true);

	// ImGui Vulkan 초기화
	ImGui_ImplVulkan_Init(&init_info, renderer.getRenderPass());

	// ImGui CommandBuffer begin
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	// ImGui Font 생성
	ImGui_ImplVulkan_CreateFontsTexture(commandBuffer);

	// ImGui CommandBuffer end
	endSingleTimeCommands(commandBuffer);

	// ImGui Font Object 파괴
	ImGui_ImplVulkan_DestroyFontUploadObjects();

	// framebuffer
}

void ImGuiLayer::onDetach()
{
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void ImGuiLayer::onImGuiRender()
{
	// ImGui::ShowDemoWindow();
	
}

void ImGuiLayer::onEvent(Event &event)
{
}

void ImGuiLayer::begin()
{
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplVulkan_NewFrame();
	ImGui::NewFrame();
}

void ImGuiLayer::renderDrawData(Scene* scene, VkCommandBuffer commandBuffer)
{
	ImGuiIO &io = ImGui::GetIO();
	App &app = App::get();
	// io.DisplaySize = ImVec2((float)app.getWindow().getWidth(), (float)app.getWindow().getHeight());
	io.FontGlobalScale = 1.5f; // 기본 폰트 크기를 1.5배로 증가

	ImGui::Begin("GUI");


	float& ambientStrength = scene->getAmbientStrength();
    ImGui::DragFloat("Global Ambient Strength", &ambientStrength, 0.01f, 0.0f, 1.0f);
    ImGui::Separator();
	 // 다중 광원 처리
    auto& lights = scene->getLights(); // 모든 광원을 가져옵니다.
	auto& lightObjects = scene->getLightObjects();
    for (size_t i = 0; i < lights.size(); i++) {
        std::string lightLabelPrefix = "Light " + std::to_string(i);
		auto& lightObject = lightObjects[i];
        if (ImGui::TreeNode(lightLabelPrefix.c_str())) {
            glm::vec3& lightPosition = lights[i].position;
            if (ImGui::DragFloat3((lightLabelPrefix + " Position").c_str(), glm::value_ptr(lightPosition), 0.01f, -10.0f, 10.0f)) {
				lightObject->setPosition(lightPosition);
			}

            glm::vec3& lightColor = lights[i].color;
            ImGui::ColorEdit3((lightLabelPrefix + " Color").c_str(), glm::value_ptr(lightColor));

            float& intensity = lights[i].intensity;
			ImGui::DragFloat((lightLabelPrefix + " Intensity").c_str(), &intensity, 0.01f, 0.0f, 1.0f);


			// 타입 설정
			uint32_t& type = lights[i].type;
			if (ImGui::DragInt((lightLabelPrefix + " Type").c_str(), reinterpret_cast<int*>(&type), 1, 0, 2)) {
			}

			// 방향 설정 (Spot Light 및 Directional Light만 사용)
			if (type == 1 || type == 2) { // Spot Light 또는 Directional Light
				glm::vec3& direction = lights[i].direction;
				if (ImGui::DragFloat3((lightLabelPrefix + " Direction").c_str(), glm::value_ptr(direction), 0.01f, -1.0f, 1.0f)) {
				}
			}

			// 스포트라이트 추가 속성 (cutoff 설정)
			if (type == 1) { // Spot Light
				float innerCutoffDegree = glm::degrees(glm::acos(lights[i].innerCutoff));
				float outerCutoffDegree = glm::degrees(glm::acos(lights[i].outerCutoff));

				if (ImGui::DragFloat((lightLabelPrefix + " Inner Cutoff (Degree)").c_str(), &innerCutoffDegree, 0.1f, 0.0f, 90.0f)) {
					// 입력받은 각도를 코사인 값으로 변환하여 저장
					lights[i].innerCutoff = glm::cos(glm::radians(innerCutoffDegree));
				}

				if (ImGui::DragFloat((lightLabelPrefix + " Outer Cutoff (Degree)").c_str(), &outerCutoffDegree, 0.1f, 0.0f, 90.0f)) {
					// 입력받은 각도를 코사인 값으로 변환하여 저장
					lights[i].outerCutoff = glm::cos(glm::radians(outerCutoffDegree));
				}
			}

			uint32_t& onShadowMap = lights[i].onShadowMap;
			if (ImGui::Checkbox((lightLabelPrefix + " On Shadow Map").c_str(), reinterpret_cast<bool*>(&onShadowMap))) {
			}
            ImGui::TreePop();
        }
    }

	auto& objects = scene->getObjects();
    for (uint32_t index = 0; index < objects.size(); index++) {
        std::string label = "Object: " + objects[index]->getName();

        if (ImGui::TreeNode(label.c_str())) {
            glm::vec3& position = objects[index]->getPosition();
            if (ImGui::DragFloat3("Position", glm::value_ptr(position), 0.01f, -10.0f, 10.0f)) {
            }

            glm::vec3& rotation = objects[index]->getRotation();
            if (ImGui::DragFloat3("Rotation", glm::value_ptr(rotation), 0.01f, -180.0f, 180.0f)) {
            }

            glm::vec3& scale = objects[index]->getScale();
            if (ImGui::DragFloat3("Scale", glm::value_ptr(scale), 0.01f, 0.1f, 10.0f)) {
            }

            // 🛠️ Material Settings
            if (ImGui::CollapsingHeader("Material Settings")) {

				auto& materials = objects[index]->getMaterials();

				for (uint32_t i = 0; i < materials.size(); i++) {
					auto& albedo = materials[i]->getAlbedo();
					std::string tag = "Material: " + objects[index]->getName() + " " + std::to_string(i);
					if (ImGui::TreeNode(tag.c_str())) {
						if (ImGui::Checkbox("Albedo Flag", &albedo.flag)) {
						}
						if (!albedo.flag) {
							ImGui::ColorEdit3("Albedo Color", glm::value_ptr(albedo.albedo));
						}
						auto& normalMap = materials[i]->getNormalMap();
						if (ImGui::Checkbox("NormalMap Flag", &normalMap.flag)) {
						}
						auto& roughness = materials[i]->getRoughness();
						if (ImGui::Checkbox("Roughness Flag", &roughness.flag)) {
						}
						if (!roughness.flag) {
							ImGui::SliderFloat("Roughness Value", &roughness.roughness, 0.0f, 1.0f);
						}
						auto& metallic = materials[i]->getMetallic();
						if (ImGui::Checkbox("Metallic Flag", &metallic.flag)) {
						}
						if (!metallic.flag) {
							ImGui::SliderFloat("Metallic Value", &metallic.metallic, 0.0f, 1.0f);
						}
						auto& ao = materials[i]->getAOMap();
						if (ImGui::Checkbox("AO Flag", &ao.flag)) {
						}
						if (!ao.flag) {
							ImGui::SliderFloat("AO Value", &ao.ao, 0.0f, 1.0f);
						}
						auto& height = materials[i]->getHeightMap();
						if (ImGui::Checkbox("Height Flag", &height.flag)) {
						}
						if (!height.flag) {
							ImGui::SliderFloat("Height Value", &height.height, 0.0f, 1.0f);
						}
						ImGui::TreePop();
					}
				}
            }

            ImGui::TreePop();
        }
    }
	ImGui::End();

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		AL_CORE_INFO("render draw data");
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

VkCommandBuffer ImGuiLayer::beginSingleTimeCommands()
{
	AL_CORE_TRACE("ImGuiLayer::beginSingleTimeCommands");

	VkCommandBuffer commandBuffer;

	// 커맨드 버퍼 할당을 위한 구조체
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level =
		VK_COMMAND_BUFFER_LEVEL_PRIMARY; // PRIMARY LEVEL 등록 (해당 커맨드 버퍼가 큐에 단독으로 제출될 수 있음)
	allocInfo.commandPool = commandPool; // 커맨드 풀 지정
	allocInfo.commandBufferCount = 1;	 // 커맨드 버퍼 개수 지정

	// 커맨드 버퍼 생성
	vkAllocateCommandBuffers(init_info.Device, &allocInfo, &commandBuffer);

	// 커맨드 버퍼 기록을 위한 정보 객체
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // 커맨드 버퍼를 1번만 제출

	// GPU에 필요한 작업을 모두 커맨드 버퍼에 기록하기 시작
	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	return commandBuffer;
}

void ImGuiLayer::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	AL_CORE_TRACE("ImGuiLayer::endSingleTimeCommands");
	// 커맨드 버퍼 기록 중지
	vkEndCommandBuffer(commandBuffer);

	// 복사 커맨드 버퍼 제출 정보 객체 생성
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;			 // 커맨드 버퍼 개수
	submitInfo.pCommandBuffers = &commandBuffer; // 커맨드 버퍼 등록

	vkQueueSubmit(init_info.Queue, 1, &submitInfo, VK_NULL_HANDLE); // 커맨드 버퍼 큐에 제출
	vkQueueWaitIdle(init_info.Queue);								// 그래픽스 큐 작업 종료 대기

	vkFreeCommandBuffers(init_info.Device, commandPool, 1, &commandBuffer); // 커맨드 버퍼 제거
}

} // namespace ale