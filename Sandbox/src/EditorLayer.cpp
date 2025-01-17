#include "EditorLayer.h"
#include "Scene/SceneSerializer.h"
#include "Utils/PlatformUtils.h"

namespace ale
{
EditorLayer::EditorLayer() : Layer("EditorLayer")
{
}

void EditorLayer::onAttach()
{
	// Editor에 필요한 texture들 create
	m_PlayIcon = Texture::createTexture("Sandbox/Resources/Icons/PlayButton.png");
	m_PauseIcon = Texture::createTexture("Sandbox/Resources/Icons/PauseButton.png");
	m_StepIcon = Texture::createTexture("Sandbox/Resources/Icons/StepButton.png");

	auto &context = VulkanContext::getContext();
	descriptorPool = context.getDescriptorPool();
	device = context.getDevice();

	playTextureID =
		VulkanUtil::createIconTexture(device, descriptorPool, m_PlayIcon->getImageView(), m_PlayIcon->getSampler());
	pauseTextureID =
		VulkanUtil::createIconTexture(device, descriptorPool, m_PauseIcon->getImageView(), m_PauseIcon->getSampler());
	stepTextureID =
		VulkanUtil::createIconTexture(device, descriptorPool, m_StepIcon->getImageView(), m_StepIcon->getSampler());

	// scene 생성
	{
		// 아마 이런 흐름으로 작성
		{
			// Open Project File - 인자로 받아온 파일 경로
			// Script Engine Init
			// Open Scene File -> Parse Scene (Entity - Component)
		}
		// Temp - 지금 renderer 형식에 맞게
		m_EditorScene = Scene::createScene();
		m_ActiveScene = m_EditorScene;
		m_SceneHierarchyPanel.setContext(m_ActiveScene);

		// Entity 생성 - 적절한 Component를 Add 해야 함.
		auto box = m_ActiveScene->createEntity("Plane");
		box.addComponent<ModelComponent>();
		box.getComponent<ModelComponent>().m_Model = Model::createPlaneModel();

		box.addComponent<TextureComponent>();
		box.getComponent<TextureComponent>().m_Texture = Texture::createTexture("textures/karina.jpg");
		box.getComponent<TransformComponent>().m_Position = glm::vec3(0.0f, 0.0f, -3.0f);
		box.getComponent<TransformComponent>().m_Rotation = glm::vec3(0.0f, 0.0f, glm::radians(180.0f));
		box.getComponent<TransformComponent>().m_Scale = glm::vec3(5.0f * 0.74f, 5.0f, 1.0f);

		auto light = m_ActiveScene->createEntity("Light");
		light.addComponent<ModelComponent>();
		light.getComponent<ModelComponent>().m_Model = Model::createSphereModel();

		light.addComponent<TextureComponent>();
		light.getComponent<TextureComponent>().m_Texture = Texture::createTexture("textures/texture.png");
		light.getComponent<TransformComponent>().m_Position = m_ActiveScene->getLightPos();
		light.getComponent<TransformComponent>().m_Scale = glm::vec3(0.1f, 0.1f, 0.1f);

		auto camera = m_ActiveScene->createEntity("Camera");
		camera.addComponent<CameraComponent>();
		camera.getComponent<CameraComponent>().m_Camera.setViewportSize(WINDOW_WIDTH, WINDOW_HEIGHT);

		loadSceneToRenderer(m_ActiveScene);

		m_ContentBrowserPanel = std::unique_ptr<ContentBrowserPanel>(new ContentBrowserPanel());
	}

	// camera setting
}

void EditorLayer::onDetach()
{
	// texture clean up
}

void EditorLayer::onUpdate(Timestep ts)
{
	// Resize
	// m_Scene->onViewportResize();
	// editorcamera viewport resize

	// Scene의 State에 따라 update 구분
	switch (m_SceneState)
	{
	case ESceneState::EDIT:

		m_EditorCamera.onUpdate(ts);
		m_ActiveScene->onUpdateEditor(m_EditorCamera);
		break;

	case ESceneState::PLAY:

		m_ActiveScene->onUpdateRuntime(ts);
		break;
	}

	// Render Collider 테두리
}

void EditorLayer::onImGuiRender()
{
	// Docking space
	// setDockingSpace();

	// Menu
	setMenuBar();

	// Stats - hovered entity, rendered entities

	// viewport - texture descriptor set을 가져올 수 있는 방법 있으면 좋을듯

	// Drag & Drop

	// Gizmos

	// UI Toolbar

	m_SceneHierarchyPanel.onImGuiRender();
	m_ContentBrowserPanel->onImGuiRender();

	uiToolBar();

	SceneSerializer serializer(m_EditorScene);
	serializer.serialize("Sandbox/Project/Assets/Scenes/3DExample.ale");

	// ImGui::End(); // DockSpace -> ImGui::Begin, End 쌍 맞추기
}

void EditorLayer::onEvent(Event &e)
{
	m_EditorCamera.onEvent(e);

	EventDispatcher dispatcher(e);
	dispatcher.dispatch<WindowResizeEvent>(AL_BIND_EVENT_FN(EditorLayer::onResized));
	dispatcher.dispatch<MouseButtonPressedEvent>(AL_BIND_EVENT_FN(EditorLayer::onMouseButtonPressed));
	dispatcher.dispatch<KeyPressedEvent>(AL_BIND_EVENT_FN(EditorLayer::onKeyPressed));
}

bool EditorLayer::onMouseButtonPressed(MouseButtonPressedEvent &e)
{
	return false;
}

bool EditorLayer::onKeyPressed(KeyPressedEvent &e)
{
	if (e.isRepeat())
	{
		return false;
	}

	bool control = Input::isKeyPressed(Key::LeftControl) || Input::isKeyPressed(Key::RightControl);
	bool shift = Input::isKeyPressed(Key::LeftShift) || Input::isKeyPressed(Key::RightShift);

	switch (e.getKeyCode())
	{
	case Key::N:
		if (control)
			newScene();
		break;
	case Key::O:
		if (control)
			; // open project
		break;
	case Key::S:
		if (control)
		{
			if (shift)
				saveSceneAs();
			else
				saveScene();
		}
		break;
	default:
		break;
	}

	return false;
}

bool EditorLayer::onResized(WindowResizeEvent &e)
{
	// set viewport editor camera
	m_EditorCamera.setViewportSize(e.getWidth(), e.getHeight());
	return false;
}

void EditorLayer::setDockingSpace()
{
	static bool opt_fullscreen = true;
	static bool opt_padding = false;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
	bool p_open = true;

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		const ImGuiViewport *viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
						ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	if (!opt_padding)
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("DockSpace", &p_open, window_flags); // begin dockspace

	if (!opt_padding)
		ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	ImGuiIO &io = ImGui::GetIO();
	ImGuiStyle &style = ImGui::GetStyle();
	float minWinSizeX = style.WindowMinSize.x;
	style.WindowMinSize.x = 370.0f;
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}
	style.WindowMinSize.x = minWinSizeX;
}

void EditorLayer::setMenuBar()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open Project...", "Ctrl+O"))
				;

			ImGui::Separator();

			if (ImGui::MenuItem("New Scene", "Ctrl+N"))
				newScene();

			if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
				saveScene();

			if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
				saveSceneAs();

			ImGui::Separator();

			if (ImGui::MenuItem("Exit"))
				App::get().close();

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Script"))
		{
			if (ImGui::MenuItem("Reload assembly", "Ctrl+R"))
				;

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}
}

void EditorLayer::uiToolBar()
{
	// ImGui::Begin("##toolbar", nullptr);
	// ImGui::End();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
	auto &colors = ImGui::GetStyle().Colors;
	const auto &buttonHovered = colors[ImGuiCol_ButtonHovered];
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHovered.x, buttonHovered.y, buttonHovered.z, 0.5f));
	const auto &buttonActive = colors[ImGuiCol_ButtonActive];
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActive.x, buttonActive.y, buttonActive.z, 0.5f));

	ImGui::Begin("##toolbar", nullptr,
				 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	bool toolbarEnabled = (bool)m_ActiveScene;

	ImVec4 tintColor = ImVec4(1, 1, 1, 1);
	ImVec4 disabledTint = ImVec4(0.5f, 0.5f, 0.5f, 1);	 // 비활성화된 색상
	ImVec4 activeColor = ImVec4(0.0f, 0.5f, 1.0f, 1.0f); // 파란색 (눌림 상태)

	if (!toolbarEnabled)
		tintColor.w = 0.5f;

	float size = ImGui::GetWindowHeight() - 10.0f;
	ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
	ImGui::SetCursorPosY(size * 0.5f - 5.0f);

	bool isEditMode = (m_SceneState == ESceneState::EDIT);
	bool isPlayMode = (m_SceneState == ESceneState::PLAY);

	static bool isPlayPressed = false;
	static bool isPausePressed = false;

	if (ImGui::ImageButton("a", playTextureID, ImVec2(size + 4, size), ImVec2(0, 0), ImVec2(1, 1),
						   ImVec4(0.0f, 0.0f, 0.0f, 0.45f), isPlayPressed ? activeColor : tintColor) &&
		toolbarEnabled)
	{
		isPlayPressed = !isPlayPressed;
		if (isEditMode)
			onScenePlay();
		else
			onSceneStop();
	}

	if (!isPlayMode)
		ImGui::BeginDisabled();
	ImGui::SameLine();
	{
		if (ImGui::ImageButton("b", pauseTextureID, ImVec2(size + 4, size), ImVec2(0, 0), ImVec2(1, 1),
							   ImVec4(0.0f, 0.0f, 0.0f, 0.45f), isPausePressed ? activeColor : tintColor) &&
			toolbarEnabled)
		{
			isPausePressed = !isPausePressed;
			onScenePause();
		}
	}
	if (!isPlayMode)
		ImGui::EndDisabled();

	if (!isPlayMode)
		ImGui::BeginDisabled();
	ImGui::SameLine();
	{
		if (ImGui::ImageButton("c", stepTextureID, ImVec2(size + 4, size), ImVec2(0, 0), ImVec2(1, 1),
							   ImVec4(0.0f, 0.0f, 0.0f, 0.45f), tintColor) &&
			toolbarEnabled)
		{
			isPausePressed = true;
			// m_ActiveScene->step(1); // step one frame
		}
	}
	if (!isPlayMode)
		ImGui::EndDisabled();

	ImGui::PopStyleVar(3);
	ImGui::PopStyleColor(3);
	ImGui::End();
}

void EditorLayer::newProject()
{
}

void EditorLayer::openProject(const std::filesystem::path &path)
{
}

bool EditorLayer::openProject()
{
	std::string filepath = FileDialogs::openFile("AfterLife Project (*.hproj)\0*.hproj\0");
	if (filepath.empty())
		return false;

	openProject(filepath);
	return true;
}

void EditorLayer::saveProject()
{
}

void EditorLayer::newScene()
{
	m_ActiveScene = Scene::createScene();
	m_SceneHierarchyPanel.setContext(m_ActiveScene);
	m_EditorScenePath = std::filesystem::path();
}

void EditorLayer::openScene()
{
	std::string filepath = FileDialogs::openFile("AfterLife Scene (*.ale)\0*.ale\0");
	if (!filepath.empty())
		openScene(filepath);
}

void EditorLayer::openScene(const std::filesystem::path &path)
{
	// 추후 수정 필요

	if (m_SceneState != ESceneState::EDIT)
		onSceneStop();

	if (path.extension().string() != ".hazel")
	{
		AL_WARN("Could not load {0} - not a scene file", path.filename().string());
		return;
	}

	std::shared_ptr<Scene> newScene = Scene::createScene();
	SceneSerializer serializer(newScene);
	if (serializer.deserialize(path.string()))
	{
		m_EditorScene = newScene;
		m_SceneHierarchyPanel.setContext(m_EditorScene);

		m_ActiveScene = m_EditorScene;
		m_EditorScenePath = path;
	}
}

void EditorLayer::saveScene()
{
	if (!m_EditorScenePath.empty())
	{
		serializeScene(m_ActiveScene, m_EditorScenePath);
	}
	else
		saveSceneAs();
}

void EditorLayer::saveSceneAs()
{
	std::string filepath = FileDialogs::saveFile("AfterLife Scene (*.ale)\0*.ale\0");
	if (!filepath.empty())
	{
		serializeScene(m_ActiveScene, filepath);
		m_EditorScenePath = filepath;
	}
}

void EditorLayer::serializeScene(std::shared_ptr<Scene> &scene, const std::filesystem::path &path)
{
	SceneSerializer serializer(scene);
	serializer.serialize(path.string());
}

void EditorLayer::onScenePlay()
{
	m_SceneState = ESceneState::PLAY;

	m_ActiveScene = Scene::copyScene(m_EditorScene);

	loadSceneToRenderer(m_ActiveScene);

	// active scene runtime start

	m_SceneHierarchyPanel.setContext(m_ActiveScene);
}

void EditorLayer::onScenePause()
{
	// set active scene paused
	m_ActiveScene->setPaused(true);
}

void EditorLayer::onSceneStop()
{
	// active scene runtime stop

	m_SceneState = ESceneState::EDIT;

	m_ActiveScene = m_EditorScene;
	loadSceneToRenderer(m_ActiveScene);

	m_SceneHierarchyPanel.setContext(m_ActiveScene);
}

void EditorLayer::loadSceneToRenderer(std::shared_ptr<Scene> &scene)
{
	Renderer &renderer = App::get().getRenderer();
	renderer.loadScene(scene.get());
}

} // namespace ale