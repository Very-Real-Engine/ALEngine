#pragma once

#include "AL.h"
#include "Panel/ContentBrowserPanel.h"
#include "Panel/SceneHierarchyPanel.h"
#include "Renderer/CameraController.h"
#include "Renderer/EditorCamera.h"
#include "Renderer/Texture.h"

namespace ale
{
/**
 * @class EditorLayer
 * @brief 에디터 기능을 제공하는 레이어 클래스.
 *        씬 관리, UI 렌더링, 입력 처리 등을 담당합니다.
 */
class EditorLayer : public Layer
{
  public:
	/// @brief EditorLayer 생성자.
	EditorLayer();

	/// @brief EditorLayer 가상 소멸자.
	virtual ~EditorLayer() = default;

	/// @brief 레이어가 추가될 때 호출됩니다.
	void onAttach() override;

	/// @brief 레이어가 제거될 때 호출됩니다.
	void onDetach() override;

	/**
	 * @brief 에디터 업데이트 함수.
	 * @param ts 시간 간격(Timestep).
	 */
	void onUpdate(Timestep ts) override;

	/**
	 * @brief ImGui를 사용하여 UI를 렌더링합니다.
	 */
	void onImGuiRender() override;

	/**
	 * @brief 이벤트를 처리합니다.
	 * @param e 처리할 이벤트.
	 */
	void onEvent(Event &e) override;

	/**
	 * @brief 현재 에디터 씬을 반환합니다.
	 * @return std::shared_ptr<Scene> 에디터 씬 객체.
	 */
	std::shared_ptr<Scene> getEditorScene() const
	{
		return m_EditorScene;
	}

	/**
	 * @brief 현재 활성화된 씬을 반환합니다.
	 * @return std::shared_ptr<Scene> 활성 씬 객체.
	 */
	std::shared_ptr<Scene> getActiveScene() const
	{
		return m_ActiveScene;
	}

  private:
	// EVENTS
	bool onMouseButtonPressed(MouseButtonPressedEvent &e);
	bool onKeyPressed(KeyPressedEvent &e);
	bool onResized(WindowResizeEvent &e);

	// GUI
	void setDockingSpace();
	void setMenuBar();
	void uiToolBar();

	// PROJECT
	void newProject();
	bool openProject(const std::filesystem::path &path);
	bool openProject();
	void saveProject();

	// SCENE
	void newScene();
	void openScene();
	void openScene(const std::filesystem::path &path);
	void saveScene();
	void saveSceneAs();

	/**
	 * @brief 특정 씬을 직렬화하여 파일로 저장합니다.
	 * @param scene 저장할 씬 객체.
	 * @param path 저장할 파일 경로.
	 */
	void serializeScene(std::shared_ptr<Scene> &scene, const std::filesystem::path &path);

	void onScenePlay();
	void onScenePause();
	void onSceneStop();

	/**
	 * @brief 현재 선택된 엔티티를 복제합니다.
	 */
	void duplicateEntity();

  private:
	CameraController m_CameraController;
	EditorCamera m_EditorCamera;

	std::shared_ptr<Scene> m_EditorScene;
	std::shared_ptr<Scene> m_ActiveScene;

	std::filesystem::path m_EditorScenePath;

	SceneHierarchyPanel m_SceneHierarchyPanel;
	std::unique_ptr<ContentBrowserPanel> m_ContentBrowserPanel;

	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	VkDevice device;

	alglm::vec2 m_ViewportSize = {0.0f, 0.0f};

	enum class ESceneState
	{
		EDIT = 0,
		PLAY
	};

	ESceneState m_SceneState = ESceneState::EDIT;
	std::shared_ptr<Texture> m_PlayIcon, m_PauseIcon, m_StepIcon;
	ImTextureID playTextureID, pauseTextureID, stepTextureID;
	VkDescriptorSetLayout iconDescriptorSetLayout;
	VkDescriptorSet playDescriptorSet, pauseDescriptorSet, stepDescriptorSet;
};

} // namespace ale
