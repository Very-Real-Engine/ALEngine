#include "SceneHierarchyPanel.h"
#include "Scene/Component.h"

#include "Renderer/RenderingComponent.h"
#include "Scripting/ScriptingEngine.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/misc/cpp/imgui_stdlib.h"

#include "UI/UI.h"

#include "Physics/Rigidbody.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <cstring>

/* The Microsoft C++ compiler is non-compliant with the C++ standard and needs
 * the following definition to disable a security warning on std::strncpy().
 */
#ifdef _MSVC_LANG
#define _CRT_SECURE_NO_WARNINGS
#endif

namespace utils
{

} // namespace utils

namespace ale
{
static std::string s_DroppedFilePath;
static bool s_IsHovered = false;

SceneHierarchyPanel::SceneHierarchyPanel(const std::shared_ptr<Scene> &context)
{
	setContext(context);
}

void SceneHierarchyPanel::setContext(const std::shared_ptr<Scene> &context)
{
	m_Context = context;
	m_SelectionContext = {};
}

void SceneHierarchyPanel::onImGuiRender()
{
	ImGui::Begin("Scene Hierarchy");
	if (m_Context)
	{
		auto view = m_Context->m_Registry.view<RelationshipComponent>();

		for (auto e : view)
		{
			Entity entity(e, m_Context.get());

			auto p = entity.getComponent<RelationshipComponent>().parent;

			// Root node만 보이게
			if (p == entt::null)
			{
				drawEntityNode(entity);
			}
		}
		m_Context->destroyEntities();

		// 왼쪽 클릭 && Hovered(마우스를 window에 올려뒀을 때)
		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
			m_SelectionContext = {};

		// 오른쪽 클릭으로 Popup 활성화
		if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Create Empty"))
				m_Context->createEntity("Empty");

			if (ImGui::BeginMenu("3D Object"))
			{
				if (ImGui::MenuItem("Box"))
					;
				if (ImGui::MenuItem("Sphere"))
					;
				if (ImGui::MenuItem("Capsule"))
					;
				if (ImGui::MenuItem("Cylinder"))
					;
				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		}
	}
	ImGui::End();

	ImGui::Begin("Inspector");
	if (m_SelectionContext)
	{
		drawComponents(m_SelectionContext);
	}
	ImGui::End();
}

static bool decomposeMatrix(const glm::mat4 &transform, glm::vec3 &outScale, glm::vec3 &outRotation,
							glm::vec3 &outTranslation)
{
	glm::vec3 skew;
	glm::vec4 perspective;

	glm::quat &orientation = glm::quat(glm::radians(outRotation));

	// glm::decompose(매트릭스, 스케일, 로테이션, 위치, skew, perspective)
	if (!glm::decompose(transform, outScale, orientation, outTranslation, skew, perspective))
		return false;

	outRotation = glm::eulerAngles(orientation);

	return true;
}

void SceneHierarchyPanel::updateRelationship(Entity &newParent, Entity &child)
{
	// 1. 기존 부모에게서 제거
	auto &childRelation = child.getComponent<RelationshipComponent>();
	entt::entity oldParent = childRelation.parent;

	// Memory pool needed
	// if (oldParent != entt::null)
	// {
	// 	auto &oldParentRelation = m_Context->m_Registry.get<RelationshipComponent>(oldParent);
	// 	auto &siblings = oldParentRelation.children;
	// 	siblings.erase(std::remove(siblings.begin(), siblings.end(), &child), siblings.end());
	// }

	// 2. 새 부모로 교체
	childRelation.parent = (entt::entity)newParent;

	// 3. 새 부모의 children 목록에 추가
	auto &parentRelation = newParent.getComponent<RelationshipComponent>();
	parentRelation.children.push_back((entt::entity)child);

	// 4. 자식의 위치를 부모 기준 local좌표로 변환
	auto &tc = child.getComponent<TransformComponent>();
	glm::mat4 parentWorld = newParent.getComponent<TransformComponent>().m_WorldTransform; // 부모의 월드 매트릭스
	glm::mat4 childWorld = child.getComponent<TransformComponent>().m_WorldTransform;	   // 자식의 현재 월드 매트릭스
	// tc.m_LocalTransform = glm::inverse(parentWorld) * childWorld;
	glm::mat4 childLocalMat = glm::inverse(parentWorld) * childWorld;

	decomposeMatrix(childLocalMat, tc.m_Scale, tc.m_Rotation, tc.m_Position);
	// updateTransforms(newParent);
}

void SceneHierarchyPanel::updateTransforms(Entity entity)
{
	auto view = m_Context->m_Registry.view<RelationshipComponent, TransformComponent>();

	auto &relate = entity.getComponent<RelationshipComponent>();
	auto &tc = entity.getComponent<TransformComponent>();

	// tc.m_WorldTransform = tc.getTransform();

	entt::entity top = (entt::entity)entity;
	while (true)
	{
		auto &parentRel = m_Context->m_Registry.get<RelationshipComponent>(top);
		if (parentRel.parent == entt::null)
			break;
		top = parentRel.parent;
	}

	Entity t{top, m_Context.get()};
	updateTransformRecursive(t, glm::mat4(1.0f));
}

void SceneHierarchyPanel::updateTransformRecursive(Entity entity, const glm::mat4 &parentWorldTransform)
{
	auto &transform = entity.getComponent<TransformComponent>();
	auto &relation = entity.getComponent<RelationshipComponent>();

	// 2. 로컬 행렬 계산
	// if (relation.parent != entt::null)
	// 	// 3. 월드 변환 = 부모 월드 변환 x 로컬 변환
	// 	transform.m_WorldTransform = parentWorldTransform * transform.m_LocalTransform;
	// else
	// 	transform.m_WorldTransform = transform.getTransform();

	transform.m_WorldTransform = parentWorldTransform * transform.getTransform();

	// 4. 자식들 업데이트
	for (auto child : relation.children)
	{
		Entity entity{child, m_Context.get()};
		updateTransformRecursive(entity, transform.m_WorldTransform);
	}
}

void SceneHierarchyPanel::setSelectedEntity(Entity entity)
{
	m_SelectionContext = entity;
}

void SceneHierarchyPanel::drawEntityNode(Entity entity)
{
	auto &tag = entity.getComponent<TagComponent>().m_Tag;

	ImGuiTreeNodeFlags flags =
		((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
	flags |= ImGuiTreeNodeFlags_SpanAvailWidth; // 선택 영역이 가장자리까지 넓어지게 설정
	bool opened = ImGui::TreeNodeEx((void *)(uint64_t)(uint32_t)entity, flags, tag.c_str());
	if (ImGui::IsItemClicked())
	{
		m_SelectionContext = entity;
	}

	if (ImGui::BeginDragDropSource())
	{
		Entity e = entity;
		ImGui::SetDragDropPayload("EntityPayload", &e, sizeof(Entity));
		ImGui::Text("%s", tag.c_str());
		ImGui::EndDragDropSource();
	}

	if (ImGui::BeginDragDropTarget())
	{
		if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("EntityPayload"))
		{
			Entity *droppedEntity = (Entity *)payload->Data;
			// droppedEntity가 entity의 자식이 되도록 설정

			if (*droppedEntity != entity)
				updateRelationship(entity, *droppedEntity);
		}
		ImGui::EndDragDropTarget();
	}

	bool entityDeleted = false;
	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem("Delete Entity"))
			entityDeleted = true;

		ImGui::EndPopup();
	}

	// If node opened, draw child nodes
	if (opened)
	{
		auto &relation = entity.getComponent<RelationshipComponent>();
		for (auto child : relation.children)
		{
			Entity entity(child, m_Context.get());
			drawEntityNode(entity);
		}
		ImGui::TreePop();
	}

	if (entityDeleted)
	{
		// m_Context->destroyEntity(entity);
		m_Context->insertDestroyEntity(entity);
		if (m_SelectionContext == entity)
			m_SelectionContext = {};
	}
}

static void drawVec3Control(const std::string &label, glm::vec3 &values, float resetValue = 0.0f,
							float columnWidth = 100.0f)
{
	ImGuiIO &io = ImGui::GetIO();
	auto boldFont = io.Fonts->Fonts[0];

	ImGui::PushID(label.c_str());

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::Text(label.c_str());
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.9f, 0.2f, 0.2f, 1.0f});
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.8f, 0.1f, 0.15f, 1.0f});
	ImGui::PushFont(boldFont);
	if (ImGui::Button("X", buttonSize))
		values.x = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.3f, 0.8f, 0.3f, 1.0f});
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.2f, 0.7f, 0.2f, 1.0f});
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Y", buttonSize))
		values.y = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Z", buttonSize))
		values.z = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();

	ImGui::Columns(1);

	ImGui::PopID();
}

static void drawFloatControl(const std::string &label, float &value, float resetValue = 0.0f,
							 float columnWidth = 100.0f)
{
	ImGuiIO &io = ImGui::GetIO();
	auto boldFont = io.Fonts->Fonts[0];

	ImGui::PushID(label.c_str());

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::Text(label.c_str());
	ImGui::NextColumn();

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{0, 0});

	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = {lineHeight + 3.0f, lineHeight};

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.2f, 0.35f, 0.9f, 1.0f});
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{0.1f, 0.25f, 0.8f, 1.0f});
	ImGui::PushFont(boldFont);

	if (ImGui::Button("R", buttonSize))
		value = resetValue;

	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();

	// DragFloat
	ImGui::DragFloat("##SingleValue", &value, 0.1f, 0.0f, 0.0f, "%.2f");

	ImGui::PopStyleVar();

	// 컬럼 닫기
	ImGui::Columns(1);

	ImGui::PopID();
}

static void drawColorControl(const std::string &label, glm::vec3 &color, float columnWidth = 100.0f)
{
	ImGuiIO &io = ImGui::GetIO();
	auto boldFont = io.Fonts->Fonts[0];

	ImGui::PushID(label.c_str()); // 고유 ID를 추가하여 UI 충돌 방지

	// 컬럼 설정
	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::Text(label.c_str()); // 레이블 표시
	ImGui::NextColumn();

	// ColorEdit3을 사용하여 색상 조정
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{2, 2});						 // 패딩 조정
	ImGui::ColorEdit3("##Color", glm::value_ptr(color), ImGuiColorEditFlags_DisplayRGB); // 컬러 조정
	ImGui::PopStyleVar();

	// 컬럼 마무리
	ImGui::Columns(1);
	ImGui::PopID();
}

void drawDragDropUI(const std::string &label, float columnWidth = 100.0f)
{
	ImGuiIO &io = ImGui::GetIO();
	auto boldFont = io.Fonts->Fonts[0];

	// 고유 ID 푸시 (label 중복 방지)
	ImGui::PushID(label.c_str());

	// 2개의 컬럼 구성
	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);

	// 왼쪽 컬럼: 레이블
	ImGui::Text(label.c_str());
	ImGui::NextColumn();

	// 오른쪽 컬럼: 드래그 앤 드롭 Child 영역
	// -- Unity 스타일을 위해 스타일 설정 --
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f)); // 배경색
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));	 // 테두리 색
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);					 // 둥근 모서리
	ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);				 // 테두리 두께

	// 적절한 크기의 드래그 영역 (가로 자동, 높이 60px 예시)
	ImVec2 dragDropSize = ImVec2(ImGui::GetContentRegionAvail().x, 20.0f);

	ImGui::BeginChild("DragDropArea", dragDropSize, true, ImGuiWindowFlags_NoScrollbar);
	{
		if (!s_DroppedFilePath.empty())
		{
			// 이미 드롭된 파일이 있는 경우 표시
			ImGui::TextWrapped("Dropped File: %s", s_DroppedFilePath.c_str());
			ImGui::Separator();
			if (ImGui::Button("Clear"))
			{
				s_DroppedFilePath.clear();
			}
		}
		else
		{
			ImVec2 availableSpace = ImGui::GetContentRegionAvail();				  // 사용 가능한 영역 크기
			ImVec2 textSize = ImGui::CalcTextSize("Drag & Drop Model file here"); // 텍스트 크기 계산

			// 수평 및 수직으로 텍스트를 중앙 정렬
			ImGui::SetCursorPosX((availableSpace.x - textSize.x) * 0.5f);
			ImGui::SetCursorPosY((availableSpace.y - textSize.y) * 0.5f + 6.5f);

			// 아직 드롭된 파일이 없는 경우: 안내 문구
			if (s_IsHovered)
				ImGui::TextColored(ImVec4(0.9f, 0.9f, 1.0f, 1.0f), "Drop Here!");
			else
				ImGui::Text("Drag & Drop Model file here");
		}

		// 드래그 앤 드롭 처리
		if (ImGui::BeginDragDropTarget())
		{
			s_IsHovered = true; // 드래그 중이면 하이라이트 표시
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t *path = (const wchar_t *)payload->Data;
				std::filesystem::path gltfPath(path);
				std::cout << "gltf: " << gltfPath.string() << '\n';
			}
			ImGui::EndDragDropTarget();
		}
		else
		{
			s_IsHovered = false; // 드래그가 끝나면 해제
		}
	}
	ImGui::EndChild();

	// 스타일 되돌리기
	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(2);

	// 컬럼 끝
	ImGui::Columns(1);

	ImGui::PopID();
}

static void drawCheckBox(const std::string &label, bool &values)
{
	// bool result = values ? true : false;
	// ImGui::Checkbox(label.c_str(), &result);
	// values = result;
	ImGui::Checkbox(label.c_str(), &values);
}

template <typename T, typename UIFunction>
static void drawComponent(const std::string &name, Entity entity, UIFunction uiFunction)
{
	const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
											 ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap |
											 ImGuiTreeNodeFlags_FramePadding;
	if (entity.hasComponent<T>())
	{
		auto &component = entity.getComponent<T>();
		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{4, 4});
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImGui::Separator();
		bool open = ImGui::TreeNodeEx((void *)typeid(T).hash_code(), treeNodeFlags, name.c_str());
		ImGui::PopStyleVar();

		// 컴파일 타임에 타입 비교 수행
		if (!std::is_same<T, TransformComponent>::value)
		{
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (ImGui::Button("+", ImVec2{lineHeight, lineHeight}))
			{
				ImGui::OpenPopup("ComponentSettings");
			}
		}

		bool removeComponent = false;
		if (ImGui::BeginPopup("ComponentSettings"))
		{
			if (ImGui::MenuItem("Remove component"))
				removeComponent = true;

			ImGui::EndPopup();
		}

		if (open)
		{
			uiFunction(component);
			ImGui::TreePop();
		}

		if (removeComponent)
			entity.removeComponent<T>();
	}
}

void SceneHierarchyPanel::drawComponents(Entity entity)
{
	if (entity.hasComponent<TagComponent>())
	{
		auto &tag = entity.getComponent<TagComponent>().m_Tag;

		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		strncpy_s(buffer, sizeof(buffer), tag.c_str(), sizeof(buffer));

		std::string label = "##Tag" + std::to_string(entity.getUUID());
		// ## 뒤의 text는 ImGui 내부의 ID로 사용.
		if (ImGui::InputText(label.c_str(), buffer, sizeof(buffer)))
		{
			tag = std::string(buffer);
		}
	}

	ImGui::SameLine();
	ImGui::PushItemWidth(-1);

	if (ImGui::Button("Add Component"))
	{
		ImGui::OpenPopup("AddComponent");
	}

	ImGui::SetNextWindowSizeConstraints(ImVec2(300, 200), ImVec2(600, 400));
	if (ImGui::BeginPopup("AddComponent"))
	{
		const char *text = "Component";
		float windowWidth = ImGui::GetWindowSize().x;
		float textWidth = ImGui::CalcTextSize(text).x;
		float textPosX = (windowWidth - textWidth) * 0.5f;

		ImGui::SetCursorPosX(textPosX);
		ImGui::Text("%s", text);
		ImGui::Separator();

		displayAddComponentEntry<CameraComponent>("Camera");
		displayAddComponentEntry<ScriptComponent>("Script");
		displayAddComponentEntry<MeshRendererComponent>("Mesh Renderer");
		displayAddComponentEntry<LightComponent>("Light");
		displayAddComponentEntry<RigidbodyComponent>("Rigidbody");

		bool hasCollider = m_SelectionContext.hasComponent<BoxColliderComponent>() ||
						   m_SelectionContext.hasComponent<SphereColliderComponent>() ||
						   m_SelectionContext.hasComponent<CapsuleColliderComponent>() ||
						   m_SelectionContext.hasComponent<CylinderColliderComponent>();

		if (!hasCollider)
		{
			displayAddComponentEntry<BoxColliderComponent>("Box Collider");
			displayAddComponentEntry<SphereColliderComponent>("Sphere Collider");
			displayAddComponentEntry<CapsuleColliderComponent>("Capsule Collider");
			displayAddComponentEntry<CylinderColliderComponent>("Cylinder Collider");
		}

		ImGui::EndPopup();
	}
	ImGui::PopItemWidth();

	drawComponent<TransformComponent>("Transform", entity, [this, entity](auto &component) mutable {
		// Update Recursively
		drawVec3Control("Position", component.m_Position);
		auto &rotation = glm::degrees(component.m_Rotation);
		drawVec3Control("Rotation", rotation);
		component.m_Rotation = glm::radians(rotation);
		drawVec3Control("Scale", component.m_Scale, 1.0f);
		updateTransforms(entity);
	});

	drawComponent<CameraComponent>("Camera", entity, [](auto &component) {
		auto &camera = component.m_Camera;

		float perspectiveVerticalFov = glm::degrees(camera.getFov());
		if (ImGui::DragFloat("Vertical FOV", &perspectiveVerticalFov))
			camera.setFov(glm::radians(perspectiveVerticalFov));

		float perspectiveNear = camera.getNear();
		if (ImGui::DragFloat("Near", &perspectiveNear))
			camera.setNear(perspectiveNear);

		float perspectiveFar = camera.getFar();
		if (ImGui::DragFloat("Far", &perspectiveFar))
			camera.setFar(perspectiveFar);
	});

	drawComponent<MeshRendererComponent>("MeshRenderer", entity, [entity, scene = m_Context](auto &component) mutable {
		uint32_t meshType = component.type;
		std::shared_ptr<RenderingComponent> rc = component.m_RenderingComponent;

		const char *meshTypeStrings[] = {"Box", "Sphere", "Plane", "Ground", "None", "Model"};
		const char *currentMeshTypeString = meshTypeStrings[(int)meshType];

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, 100.0f);

		ImGui::Text("Primitive Type");
		ImGui::NextColumn();
		if (ImGui::BeginCombo("##MeshTypeCombo", currentMeshTypeString))
		{
			for (int32_t i = 0; i < 5; ++i)
			{
				bool isSelected = currentMeshTypeString == meshTypeStrings[i];

				if (ImGui::Selectable(meshTypeStrings[i], isSelected))
				{
					currentMeshTypeString = meshTypeStrings[i];
					// model 정보 바꾸기

					component.type = i;
					if (i == 4) // None
					{
					}
					else
					{
						component.m_RenderingComponent =
							RenderingComponent::createRenderingComponent(scene->getDefaultModel(i));
						component.path.clear();
					}
				}
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::Columns(1);

		// Drag & Drop Model
		// drawDragDropUI("Model");
		ImGui::Button("Model", ImVec2(200.0f, 0.0f));
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t *path = (const wchar_t *)payload->Data;
				std::filesystem::path filePath(path);
				if (filePath.extension().string() == ".gltf" || filePath.extension().string() == ".glb" ||
					filePath.extension().string() == ".obj")
				{
					std::shared_ptr<Model> model = Model::createModel(filePath.string(), scene->getDefaultMaterial());
					component.m_RenderingComponent = RenderingComponent::createRenderingComponent(model);
					component.type = 4;
					component.path = filePath.string();
				}
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::Button("Material", ImVec2(200.0f, 0.0f));
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t *path = (const wchar_t *)payload->Data;
				std::filesystem::path filePath(path);
				if (filePath.extension().string() == ".gltf" || filePath.extension().string() == ".glb" ||
					filePath.extension().string() == ".obj")
				{
					// std::shared_ptr<Model> model = Model::createModel(filePath.string(),
					// scene->getDefaultMaterial()); component.m_RenderingComponent =
					// RenderingComponent::createRenderingComponent(model);

					component.m_RenderingComponent->updateMaterial(
						Model::createModel(filePath.string(), scene->getDefaultMaterial()));
					component.type = 4;
					component.path = filePath.string();
				}
			}
			ImGui::EndDragDropTarget();
		}
		auto &materials = rc->getMaterials();
		if (materials.size() != 1)
		{
			std::shared_ptr<Material> material = materials[1];
			Albedo &albedo = material->getAlbedo();
			drawVec3Control("Albedo", albedo.albedo);
			drawCheckBox("Albedo Flag", albedo.flag);

		NormalMap &normalMap = material->getNormalMap();
		drawCheckBox("Normal Flag", normalMap.flag);

		Roughness &roughness = material->getRoughness();
		drawFloatControl("Roughness", roughness.roughness);
		drawCheckBox("Roughness Flag", roughness.flag);

		Metallic &metalic = material->getMetallic();
		drawFloatControl("Metallic", metalic.metallic);
		drawCheckBox("Metallic Flag", metalic.flag);

		AOMap &aoMap = material->getAOMap();
		drawFloatControl("AOMap", aoMap.ao);
		drawCheckBox("AOMap Flag", aoMap.flag);

		HeightMap &heightMap = material->getHeightMap();
		drawFloatControl("HeightMap", heightMap.height);
		drawCheckBox("HeightMap Flag", heightMap.flag);
	});

	drawComponent<LightComponent>("Light", entity, [entity, scene = m_Context](auto &component) mutable {
		auto &tc = entity.getComponent<TransformComponent>();

		Light *light = component.m_Light.get();

		uint32_t lightType = light->type;
		const char *lightTypeStrings[] = {"Point", "Spotlight", "Directional"};
		const char *currentLightTypeString = lightTypeStrings[(int)lightType];
		if (ImGui::BeginCombo("Light Type", currentLightTypeString))
		{
			for (int32_t i = 0; i < 3; ++i)
			{
				bool isSelected = currentLightTypeString == lightTypeStrings[i];

				if (ImGui::Selectable(lightTypeStrings[i], isSelected))
				{
					currentLightTypeString = lightTypeStrings[i];
					light->type = i;
				}

				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		bool onShadow = light->onShadowMap == 1 ? true : false;
		ImGui::Checkbox("Shadow Map", &onShadow);
		light->onShadowMap = onShadow == true ? 1 : 0;

		drawVec3Control("Position", light->position);
		tc.m_Position = light->position;
		drawVec3Control("Direction", light->direction);

		ImGui::Spacing();
		drawColorControl("Color", light->color);

		ImGui::Spacing();
		drawFloatControl("Ambient Strength", scene->getAmbientStrength());
		drawFloatControl("Intesntiy", light->intensity);
		drawFloatControl("Inner Cutoff", light->innerCutoff);
		drawFloatControl("Outer Cutoff", light->outerCutoff);
	});

	drawComponent<ScriptComponent>("Script", entity, [entity, scene = m_Context](auto &component) mutable {
		bool scriptClassExists = ScriptingEngine::entityClassExists(component.m_ClassName);

		UI::ScopedStyleColor textColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.3f, 1.0f), !scriptClassExists);

		static char buffer[64];
		strcpy(buffer, component.m_ClassName.c_str());

		if (ImGui::InputText("Class", buffer, sizeof(buffer)))
		{
			component.m_ClassName = buffer;
			return;
		}

		bool sceneRunning = scene->isRunning();

		if (sceneRunning)
		{
			std::shared_ptr<ScriptInstance> scriptInstance = ScriptingEngine::getEntityScriptInstance(entity.getUUID());
			if (scriptInstance)
			{
				const auto &fields = scriptInstance->getScriptClass()->getFields();
				for (const auto &[name, field] : fields)
				{
					if (field.m_Type == EScriptFieldType::FLOAT)
					{
						float data = scriptInstance->getFieldValue<float>(name);
						if (ImGui::DragFloat(name.c_str(), &data))
						{
							scriptInstance->setFieldValue(name, data);
						}
					}
				}
			}
		}
		else
		{
			if (scriptClassExists)
			{
				std::shared_ptr<ScriptClass> entityClass = ScriptingEngine::getEntityClass(component.m_ClassName);
				const auto &fields = entityClass->getFields();

				auto &entityFields = ScriptingEngine::getScriptFieldMap(entity);
				for (const auto &[name, field] : fields)
				{
					if (entityFields.find(name) != entityFields.end())
					{
						ScriptFieldInstance &scriptField = entityFields.at(name);

						if (field.m_Type == EScriptFieldType::FLOAT)
						{
							float data = scriptField.getValue<float>();
							if (ImGui::DragFloat(name.c_str(), &data))
								scriptField.setValue<float>(data);
						}
					}
					else
					{
						if (field.m_Type == EScriptFieldType::FLOAT)
						{
							float data = 0.0f;
							if (ImGui::DragFloat(name.c_str(), &data))
							{
								ScriptFieldInstance &fieldInstance = entityFields[name];
								fieldInstance.m_Field = field;
								fieldInstance.setValue<float>(data);
							}
						}
					}
				}
			}
		}
	});
	drawComponent<RigidbodyComponent>("Rigidbody", entity, [entity, scene = m_Context](auto &component) mutable {
		drawFloatControl("Mass", component.m_Mass);
		drawFloatControl("Drag", component.m_Damping);
		drawFloatControl("Angular Drag", component.m_AngularDamping);

		// bool useGravity = component.m_UseGravity ? true : false;
		// ImGui::Checkbox("Gravity", &useGravity);
		// component.m_UseGravity = useGravity ? true : false;
		drawCheckBox("Gravity", component.m_UseGravity);

		// FreezePos
		// FreezeRot
		// BodyType

		if (scene->isRunning())
		{
			BodyDef bdDef;
			bdDef.m_linearDamping = component.m_Damping;
			bdDef.m_angularDamping = component.m_AngularDamping;
			bdDef.m_useGravity = component.m_UseGravity;

			Rigidbody *body = (Rigidbody *)component.body;
			body->setRBComponentValue(bdDef);

			// 관성텐서 추가된다면 setMassData로 교체
			body->setMass(component.m_Mass);
		}
	});
	drawComponent<BoxColliderComponent>("BoxCollider", entity, [](auto &component) {
		drawVec3Control("Center", component.m_Center);
		drawVec3Control("Size", component.m_Size);
		drawCheckBox("IsTrigger", component.m_IsTrigger);

		// Runtime 중 수정 기능
	});
	drawComponent<SphereColliderComponent>("SphereCollider", entity, [](auto &component) {
		drawVec3Control("Center", component.m_Center);
		drawFloatControl("Radius", component.m_Radius);
		drawCheckBox("IsTrigger", component.m_IsTrigger);

		// Runtime 중 수정 기능
	});
	drawComponent<CapsuleColliderComponent>("CapsuleCollider", entity, [](auto &component) {
		drawVec3Control("Center", component.m_Center);
		drawFloatControl("Radius", component.m_Radius);
		drawFloatControl("Radius", component.m_Height);
		drawCheckBox("IsTrigger", component.m_IsTrigger);

		// Runtime 중 수정 기능
	});
	drawComponent<CylinderColliderComponent>("CylinderCollider", entity, [](auto &component) {
		drawVec3Control("Center", component.m_Center);
		drawFloatControl("Radius", component.m_Radius);
		drawFloatControl("Radius", component.m_Height);
		drawCheckBox("IsTrigger", component.m_IsTrigger);

		// Runtime 중 수정 기능
	});
}

template <typename T> void SceneHierarchyPanel::displayAddComponentEntry(const std::string &entryName)
{
	if (!m_SelectionContext.hasComponent<T>())
	{
		if (ImGui::Selectable(entryName.c_str()))
		{
			m_SelectionContext.addComponent<T>();
			ImGui::CloseCurrentPopup();
		}
	}
}
} // namespace ale