#pragma once

#include "Core/Layer.h"
#include "Events/AppEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

#include "ImGui/ImGuiVulkanGlfw.h"
#include "ImGui/ImGuiVulkanRenderer.h"

#include "Renderer/Common.h"
#include "Renderer/VulkanContext.h"

namespace ale
{
/**
 * @class ImGuiLayer
 * @brief ImGui를 위한 렌더링 및 이벤트 처리를 담당하는 레이어 클래스.
 */
class ImGuiLayer : public Layer
{
  public:
	/// @brief ImGuiLayer 생성자. 
	ImGuiLayer();

	/// @brief ImGuiLayer 소멸자. 
	~ImGuiLayer() = default;

	/// @brief 레이어가 추가될 때 실행되는 초기화 함수.
	void onAttach() override;

	/// @brief 레이어가 제거될 때 실행되는 정리 함수.
	void onDetach() override;

	/**
	 * @brief 이벤트를 처리하는 함수.
	 * @param event 발생한 이벤트 참조.
	 */
	void onEvent(Event &event) override;

	/// @brief 새로운 프레임을 시작하는 함수.
	void beginFrame();

	/**
	 * @brief ImGui의 DrawData를 렌더링하는 함수.
	 * @param commandBuffer Vulkan 명령 버퍼.
	 */
	static void renderDrawData(VkCommandBuffer commandBuffer);

	/**
	 * @brief 이벤트 처리를 블록할지 여부를 설정합니다.
	 * @param block true이면 이벤트 처리를 막음, false이면 이벤트 처리 허용.
	 */
	void blockEvents(bool block)
	{
		m_BlockEvents = block;
	}

  private:
	/// @brief 다크 테마 색상을 설정하는 내부 함수.
	void setDarkThemeColors();

  private:
	ImGui_ImplVulkan_InitInfo init_info = {};
	bool m_BlockEvents = false; // true default
};
} // namespace ale
