#include "SceneHierarchyPanel.h"
#include "Scene/Component.h"

#include "Renderer/RenderingComponent.h"
#include "Renderer/SAComponent.h"
#include "Scripting/ScriptingEngine.h"

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/misc/cpp/imgui_stdlib.h"

#include "UI/UI.h"

#include "Physics/Rigidbody.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "Core/App.h"

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
					m_Context->createPrimitiveMeshEntity("Box", 1);
				if (ImGui::MenuItem("Sphere"))
					m_Context->createPrimitiveMeshEntity("Sphere", 2);
				if (ImGui::MenuItem("Plane"))
					m_Context->createPrimitiveMeshEntity("Plane", 3);
				if (ImGui::MenuItem("Ground"))
					m_Context->createPrimitiveMeshEntity("Ground", 4);
				if (ImGui::MenuItem("Capsule"))
					m_Context->createPrimitiveMeshEntity("Capsule", 5);
				if (ImGui::MenuItem("Cylinder"))
					m_Context->createPrimitiveMeshEntity("Cylinder", 6);
				ImGui::EndMenu();
			}
			ImGui::EndPopup();
		}

		ImGui::Dummy(ImVec2(ImGui::GetWindowContentRegionMax())); // 높이 50의 빈 영역
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("EntityPayload"))
			{
				Entity *droppedEntity = (Entity *)payload->Data;
				// 빈 영역에 드랍된 경우, 해당 엔티티를 최상위 엔티티로 만듭니다.
				updateRelationship(*droppedEntity);
			}
			ImGui::EndDragDropTarget();
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

static bool decomposeMatrix(const alglm::mat4 &transform, alglm::vec3 &outScale, alglm::vec3 &outRotation,
							alglm::vec3 &outTranslation)
{
	alglm::vec4 perspective;

	alglm::quat &orientation = alglm::quat(alglm::radians(outRotation));

	// alglm::decompose(매트릭스, 스케일, 로테이션, 위치, skew, perspective)
	if (!alglm::decompose(transform, outScale, orientation, outTranslation, perspective))
		return false;

	outRotation = alglm::eulerAngles(orientation);

	return true;
}

void SceneHierarchyPanel::updateRelationship(Entity &newParent, Entity &child)
{
	// 1. 기존 부모에게서 제거
	auto &childRelation = child.getComponent<RelationshipComponent>();
	entt::entity oldParent = childRelation.parent;

	if (oldParent != entt::null)
	{
		auto &oldParentRelation = m_Context->m_Registry.get<RelationshipComponent>(oldParent);
		auto &siblings = oldParentRelation.children;
		// entt::entity e = (entt::entity)child;
		siblings.erase(std::remove(siblings.begin(), siblings.end(), child), siblings.end());
	}

	// 2. 새 부모로 교체
	childRelation.parent = (entt::entity)newParent;

	// 3. 새 부모의 children 목록에 추가
	auto &parentRelation = newParent.getComponent<RelationshipComponent>();
	parentRelation.children.push_back((entt::entity)child);

	// 4. 자식의 위치를 부모 기준 local좌표로 변환
	auto &tc = child.getComponent<TransformComponent>();
	alglm::mat4 parentWorld = newParent.getComponent<TransformComponent>().m_WorldTransform; // 부모의 월드 매트릭스
	alglm::mat4 childWorld = child.getComponent<TransformComponent>().m_WorldTransform; // 자식의 현재 월드 매트릭스
	// tc.m_LocalTransform = alglm::inverse(parentWorld) * childWorld;
	alglm::mat4 childLocalMat = alglm::inverse(parentWorld) * childWorld;

	decomposeMatrix(childLocalMat, tc.m_Scale, tc.m_Rotation, tc.m_Position);
	// updateTransforms(newParent);
}

void SceneHierarchyPanel::updateRelationship(Entity &child)
{
	auto &childRelation = child.getComponent<RelationshipComponent>();
	entt::entity oldParent = childRelation.parent;

	if (oldParent != entt::null)
	{
		auto &oldParentRelation = m_Context->m_Registry.get<RelationshipComponent>(oldParent);
		auto &siblings = oldParentRelation.children;
		entt::entity e = (entt::entity)child;
		siblings.erase(std::remove(siblings.begin(), siblings.end(), e), siblings.end());
	}

	// 부모가 없으므로 null로 설정 (최상위 엔티티)
	childRelation.parent = entt::null;
}

void SceneHierarchyPanel::updateTransforms(Entity entity)
{
	entt::entity top = (entt::entity)entity;
	while (true)
	{
		auto &parentRel = m_Context->m_Registry.get<RelationshipComponent>(top);
		if (parentRel.parent == entt::null)
			break;
		top = parentRel.parent;
	}

	Entity t{top, m_Context.get()};
	updateTransformRecursive(t, alglm::mat4(1.0f));
}

void SceneHierarchyPanel::updateActiveInfo(Entity &entity, bool parentEffectiveActive)
{
	auto &tc = entity.getComponent<TagComponent>();
	// 자신의 effective 활성 상태는 부모의 활성 상태와 자신의 m_selfActive의 곱
	tc.m_isActive = parentEffectiveActive && tc.m_selfActive;

	auto &rc = entity.getComponent<RelationshipComponent>();
	// 자식 엔티티에 대해 재귀 호출: 현재 엔티티의 m_isActive가 자식에게 부모 효과 상태로 전달됨
	for (auto &child : rc.children)
	{
		Entity c{child, m_Context.get()};
		updateActiveInfo(c, tc.m_isActive);
	}
}

void SceneHierarchyPanel::updateTransformRecursive(Entity entity, const alglm::mat4 &parentWorldTransform)
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

static void drawVec3Control(const std::string &label, alglm::vec3 &values, float resetValue = 0.0f,
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

static void drawColorControl(const std::string &label, alglm::vec3 &color, float columnWidth = 100.0f)
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
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{2, 2});						   // 패딩 조정
	ImGui::ColorEdit3("##Color", alglm::value_ptr(color), ImGuiColorEditFlags_DisplayRGB); // 컬러 조정
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
			UUID uuid;
			std::string label = "+##" + std::to_string((uint64_t)uuid);
			if (ImGui::Button(label.c_str(), ImVec2{lineHeight, lineHeight}))
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
		auto &tc = entity.getComponent<TagComponent>();
		auto &tag = tc.m_Tag;

		char buffer[256];
		memset(buffer, 0, sizeof(buffer));
		strncpy_s(buffer, sizeof(buffer), tag.c_str(), sizeof(buffer));

		std::string label = "##Tag" + std::to_string(entity.getUUID());

		if (ImGui::Checkbox("##Active", &tc.m_selfActive))
		{
			// 자식 엔티티일 경우 부모의 effective 활성 상태를 사용하여 업데이트
			bool parentEffectiveActive = true;
			auto &rc = entity.getComponent<RelationshipComponent>();
			if (rc.parent != entt::null)
			{
				Entity parent{rc.parent, m_Context.get()};
				parentEffectiveActive = parent.getComponent<TagComponent>().m_isActive;
			}
			updateActiveInfo(entity, parentEffectiveActive);
		}

		ImGui::SameLine();

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
		if (!m_Context->isRunning())
			ImGui::OpenPopup("AddComponent");
	}

	ImGui::SetNextWindowSizeConstraints(ImVec2(300, 200), ImVec2(600, 400));
	if (!m_Context->isRunning())
	{
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
			displayAddComponentEntry<SkeletalAnimatorComponent>("Animator");

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
	}
	ImGui::PopItemWidth();

	drawComponent<TransformComponent>("Transform", entity, [this, entity](auto &component) mutable {
		// Update Recursively
		drawVec3Control("Position", component.m_Position);
		auto &rotation = alglm::degrees(component.m_Rotation);
		drawVec3Control("Rotation", rotation);
		component.m_Rotation = alglm::radians(rotation);
		drawVec3Control("Scale", component.m_Scale, 1.0f);
		updateTransforms(entity);
	});

	drawComponent<CameraComponent>("Camera", entity, [](auto &component) {
		auto &camera = component.m_Camera;

		float perspectiveVerticalFov = alglm::degrees(camera.getFov());
		if (ImGui::DragFloat("Vertical FOV", &perspectiveVerticalFov))
			camera.setFov(alglm::radians(perspectiveVerticalFov));

		float perspectiveNear = camera.getNear();
		if (ImGui::DragFloat("Near", &perspectiveNear))
			camera.setNear(perspectiveNear);

		float perspectiveFar = camera.getFar();
		if (ImGui::DragFloat("Far", &perspectiveFar))
			camera.setFar(perspectiveFar);

		ImGui::Button("Skybox", ImVec2(200.0f, 0.0f));
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t *path = (const wchar_t *)payload->Data;
				std::filesystem::path filePath(path);
				if (filePath.extension().string() == ".hdr")
				{
					component.skyboxPath = filePath.string();
					std::cout << "skybox: " << filePath.string() << '\n';
					Renderer &renderer = App::get().getRenderer();
					renderer.updateSkybox(filePath.string());
				}
			}
			ImGui::EndDragDropTarget();
		}
	});

	drawComponent<SkeletalAnimatorComponent>("Animator", entity, [this, &entity](auto &component) {
		if (!entity.hasComponent<MeshRendererComponent>())
		{
			ImGui::Text("Animator needs MeshRenderComponent");
			return;
		}
		auto& mr = entity.getComponent<MeshRendererComponent>();
		if (!mr.m_RenderingComponent)
		{
			ImGui::Text("Animator needs Model on MeshRenderComponent");
			return ;
		}

		SAComponent *sac = (SAComponent *)component.sac.get();
		// 1. 현재 애니메이션 버튼/박스
		ImGui::Text("Current Animation:");
		std::string currentAnimationName = sac->getCurrentAnimationName();
		if (ImGui::Button(currentAnimationName.c_str(), ImVec2(200, 30)))
		{
			// 클릭 시 추가 동작을 넣을 수 있음
		}

		ImGui::Spacing();

		// 2. 애니메이션 목록 (더블클릭하면 선택)
		ImGui::Text("Animation List:");
		ImGui::BeginChild("AnimationList", ImVec2(200, 100), true);
		std::shared_ptr<SkeletalAnimations> animationList = sac->getAnimations();
		if (animationList != nullptr)
		{
			for (int i = 0; i < animationList->size(); i++)
			{
				// 항목 선택: 더블클릭하면 현재 애니메이션에 반영
				ImGui::Selectable((*animationList)[i].getName().c_str(), sac->getCurrentAnimationIndex() == i);
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
					sac->setCurrentAnimation(&(*animationList)[i]);
			}
		}
		ImGui::EndChild();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		SAData keyframe = sac->getData();
		float totalDuration = std::max(sac->getDuration(), 0.0f);
		float timelineProgress;

		// 현재 진행률 계산 (현재시간을 기반으로)
		if (totalDuration == 0.0f)
			totalDuration = 1.0f;
		timelineProgress = (keyframe.m_CurrentKeyFrameTime - keyframe.m_FirstKeyFrameTime) / totalDuration;
		timelineProgress = std::clamp(timelineProgress, 0.0f, 1.0f);

		// 3. 애니메이션 재생바 (재생, 정지, 멈춤, 맨앞, 맨뒤)
		// button icon id

		ImGui::Text("Playback Controls:");
		// 맨앞 버튼
		if (ImGui::Button("<<", ImVec2(40, 30)))
		{
			keyframe.m_CurrentKeyFrameTime = keyframe.m_FirstKeyFrameTime;
			component.m_IsPlaying = false;
		}
		ImGui::SameLine();
		// 재생 / 멈춤 버튼
		if (component.m_IsPlaying)
		{
			if (ImGui::Button("Pause", ImVec2(60, 30)))
			{
				component.m_IsPlaying = false;
			}
		}
		else
		{
			if (ImGui::Button("Play", ImVec2(60, 30)))
			{
				component.m_IsPlaying = true;
				sac->start();
			}
		}
		ImGui::SameLine();
		// 정지 버튼
		if (ImGui::Button("Stop", ImVec2(60, 30)))
		{
			component.m_IsPlaying = false;
			keyframe.m_CurrentKeyFrameTime = keyframe.m_FirstKeyFrameTime;
			timelineProgress = 0.0f;
		}
		ImGui::SameLine();
		// 맨뒤 버튼
		if (ImGui::Button(">>", ImVec2(40, 30)))
		{
			keyframe.m_CurrentKeyFrameTime = keyframe.m_LastKeyFrameTime;
			timelineProgress = 1.0f;
			component.m_IsPlaying = false;
		}

		// 6. 재생속도배수 (플레이백 스피드 조절)
		ImGui::SameLine();
		if (ImGui::Button("-", ImVec2(30, 30)))
		{
			component.m_SpeedFactor = std::max(0.1f, component.m_SpeedFactor - 0.5f);
		}
		ImGui::SameLine();
		// 현재 속도 텍스트
		float customVerticalPadding = 5.0f;
		ImVec2 currentPadding = ImGui::GetStyle().FramePadding;
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(currentPadding.x, customVerticalPadding));

		ImGui::PushItemWidth(40.0f);
		ImGui::DragFloat("##Speed", &component.m_SpeedFactor, 0.1f, 0.0f, 0.0f, "%.1f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();
		ImGui::SameLine();
		ImGui::PushID(20);
		if (ImGui::Button("+", ImVec2(30, 30)))
		{
			if (component.m_SpeedFactor == 0.1f)
				component.m_SpeedFactor = 0.5f;
			else
				component.m_SpeedFactor += 0.5f;
		}
		ImGui::PopID();
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		// 4. 애니메이션 타임라인
		// 타임라인 위젯의 위치와 크기 설정
		ImVec2 timelinePos = ImGui::GetCursorScreenPos();
		float timelineWidth = 300.0f;
		float timelineHeight = 20.0f;

		// 드래그 입력: 마우스 위치에 따라 진행률 업데이트
		// ImGui::SetCursorScreenPos(ImVec2(buttonCenter.x - buttonRadius, buttonCenter.y - buttonRadius));
		ImGui::InvisibleButton("TimelineDrag", ImVec2(timelineWidth, timelineHeight));
		if (ImGui::IsItemActive())
		{
			ImVec2 mousePos = ImGui::GetIO().MousePos;
			timelineProgress = (mousePos.x - timelinePos.x) / timelineWidth;
			timelineProgress = std::clamp(timelineProgress, 0.0f, 1.0f);

			// 진행률에 따른 현재시간 업데이트
			keyframe.m_CurrentKeyFrameTime = keyframe.m_FirstKeyFrameTime + timelineProgress * totalDuration;
			sac->setData(0, keyframe);
			component.m_IsTimelineDrag = true;
		}
		if (ImGui::IsItemDeactivated())
			component.m_IsTimelineDrag = false;

		// 타임라인 버튼(원형)의 위치 계산
		float buttonRadius = timelineHeight * 0.3f;
		float buttonX = timelinePos.x + timelineProgress * timelineWidth;
		ImVec2 buttonCenter(buttonX, timelinePos.y + timelineHeight * 0.5f);

		ImDrawList *draw_list = ImGui::GetWindowDrawList();
		draw_list->AddLine(ImVec2(timelinePos.x, timelinePos.y + timelineHeight * 0.5f),
						   ImVec2(timelinePos.x + timelineWidth, timelinePos.y + timelineHeight * 0.5f),
						   IM_COL32(200, 200, 200, 255), 2.0f);
		draw_list->AddCircleFilled(buttonCenter, buttonRadius, IM_COL32(239, 109, 128, 255));

		// Dummy로 위젯 크기 확보
		ImGui::Dummy(ImVec2(timelineWidth, timelineHeight));

		ImGui::Spacing();

		// 5. 애니메이션 속성 (테스트용: 반복 체크박스)
		bool repeat;
		int index = sac->getCurrentAnimationIndex();
		if (index != NON_CURRENT_ANIMATION_INT)
		{
			repeat = sac->getRepeat(index);

			ImGui::Text("Animation Properties:");
			ImGui::Checkbox("loop", &repeat);

			sac->setRepeat(repeat, index);
		}

		ImGui::Separator();
		ImGui::Spacing();
		// StateManager
		if (entity.hasComponent<ScriptComponent>())
		{
			std::shared_ptr<AnimationStateManager> stateManager = sac->getStateManager();
			auto& states = stateManager->getStates();
			auto& transitions = stateManager->getTransitions();

			// current transition
			char currentTransitionName[128];
			snprintf(currentTransitionName, 128, "%s -> %s",
				stateManager->prevState.stateName.c_str(),
				stateManager->currentState.stateName.c_str());
			ImGui::Button(currentTransitionName, ImVec2(200, 30));
			ImGui::Spacing();
			
			// 좌측: State 리스트
			ImGui::BeginChild("StatesList", ImVec2(150, 150), true);
			static std::string selectedStateKey;
			for (auto &kv : states)
			{
				const std::string &key = kv.first;
				// AnimationState& state = kv.second;
				if (ImGui::Selectable(key.c_str(), selectedStateKey == key))
				{
					selectedStateKey = key;
				}
				if (ImGui::BeginDragDropSource())
				{
					std::string payload = key;
					ImGui::SetDragDropPayload("STATE_ITEM", &payload, sizeof(std::string));
					ImGui::Text("%s", key);
					ImGui::EndDragDropSource();
				}
			}
			ImGui::EndChild();
			ImGui::SameLine();

			// 우측: Transition 리스트
			static int selectedTransitionIndex = -1;

			ImGui::BeginChild("TransitionsList", ImVec2(150, 150), true);

			for (int i = 0; i < transitions.size(); i++)
			{
				AnimationStateTransition &trans = transitions[i];
				char label[128];
				snprintf(label, 128, "From:%s > To:%s", trans.fromState.c_str(), trans.toState.c_str());
				if (ImGui::Selectable(label, selectedTransitionIndex == i))
				{
					selectedTransitionIndex = i;
				}
			}
			ImGui::EndChild();

			if (ImGui::Button("Add Transition", ImVec2(100, 30)))
			{
				AnimationStateTransition newTransition;
				newTransition.fromState = "";
				newTransition.toState = "";
				newTransition.blendTime = 0.5f;
				newTransition.conditionName = "";

				transitions.push_back(newTransition);
				selectedTransitionIndex = static_cast<int>(transitions.size()) - 1;
			}
			ImGui::SameLine();
			if (ImGui::Button("Delete Transition", ImVec2(120, 30)))
			{
				stateManager->deleteTransition(transitions[selectedTransitionIndex]);
				selectedTransitionIndex = -1;
			}

			ImGui::Separator();

			// State 수정창
			if (!selectedStateKey.empty())
			{
				AnimationState &state = states[selectedStateKey];
				ImGui::Text("State Details:");
				ImGui::Columns(2, "StateColumns", false);

				// State Name
				ImGui::Text("State Name:");
				ImGui::NextColumn();
				ImGui::SetNextItemWidth(-1);
				{
					char buf[256];
					strncpy(buf, state.stateName.c_str(), sizeof(buf));
					if (ImGui::InputText("##StateName", buf, sizeof(buf)))
					{
						std::string newVal(buf);
						if (state.stateName != newVal)
						{
							state.stateName = newVal;
							component.m_IsChanged = true;
						}
					}
				}
				ImGui::NextColumn();

				// Animation Name (ReadOnly)
				ImGui::Text("Animation Name:");
				ImGui::NextColumn();
				ImGui::SetNextItemWidth(-1);
				{
					char buf[256];
					strncpy(buf, state.animationName.c_str(), sizeof(buf));
					if (ImGui::InputText("##AnimationName", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly))
					{
						std::string newVal(buf);
						if (state.animationName != newVal)
						{
							state.animationName = newVal;
							component.m_IsChanged = true;
						}
					}
				}
				ImGui::NextColumn();

				// Looping
				ImGui::Text("Looping:");
				ImGui::NextColumn();
				if (ImGui::Checkbox("##Looping", &state.looping))
					component.m_IsChanged = true;
				ImGui::NextColumn();

				// Interruptible
				ImGui::Text("Interruptible:");
				ImGui::NextColumn();
				if (ImGui::Checkbox("##Interruptible", &state.interruptible))
					component.m_IsChanged = true;
				ImGui::NextColumn();

				// Default Blend Time
				ImGui::Text("Default Blend Time:");
				ImGui::NextColumn();
				ImGui::SetNextItemWidth(-1);
				if (ImGui::InputFloat("##DefaultBlendTime", &state.defaultBlendTime, 0.0f, 0.0f, "%.2f"))
					component.m_IsChanged = true;
				ImGui::Columns(1);
			}

			if (!selectedStateKey.empty() &&
				selectedTransitionIndex >= 0 && selectedTransitionIndex < transitions.size())
				ImGui::Separator();

			// Transition 수정창
			if (selectedTransitionIndex >= 0 && selectedTransitionIndex < transitions.size())
			{
				AnimationStateTransition &transition = transitions[selectedTransitionIndex];
				ImGui::Text("Transition Details:");
				ImGui::Columns(2, "TransitionColumns", false);

				// From State (드래그앤드롭 타겟)
				ImGui::Text("From State:");
				ImGui::NextColumn();
				{
					std::string displayFrom = transition.fromState.empty() ? "From State" : transition.fromState;
					if (ImGui::Button(displayFrom.c_str(), ImVec2(-1, 0)))
					{
						// 클릭 시 동작은 없고, 드래그앤드롭으로 값 할당
					}
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("STATE_ITEM"))
						{
							const char *payloadStr = (const char *)payload->Data;
							if (transition.fromState != payloadStr)
							{
								transition.fromState = std::string(payloadStr);
								component.m_IsChanged = true;
							}
						}
						ImGui::EndDragDropTarget();
					}
				}
				ImGui::NextColumn();

				// To State (드래그앤드롭 타겟)
				ImGui::Text("To State:");
				ImGui::NextColumn();
				{
					std::string displayTo = transition.toState.empty() ? "To State" : transition.toState;
					if (ImGui::Button(displayTo.c_str(), ImVec2(-1, 0)))
					{
						// 클릭 시 동작 없음
					}
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("STATE_ITEM"))
						{
							const char *payloadStr = (const char *)payload->Data;
							if (transition.toState != payloadStr)
							{
								transition.toState = std::string(payloadStr);
								component.m_IsChanged = true;
							}
						}
						ImGui::EndDragDropTarget();
					}
				}
				ImGui::NextColumn();

				// Blend Time
				ImGui::Text("Blend Time:");
				ImGui::NextColumn();
				ImGui::SetNextItemWidth(-1);
				if (ImGui::InputFloat("##BlendTime", &transition.blendTime, 0.0f, 0.0f, "%.2f"))
					component.m_IsChanged = true;
				ImGui::NextColumn();

				ImGui::Text("Invert Condition:");
				ImGui::NextColumn();
				// 버튼의 텍스트는 현재 invertCondition 값에 따라 "Enabled" 또는 "Disabled"로 표시
				if (ImGui::Button(transition.invertCondition ? "Enabled" : "Disabled", ImVec2(60, 30)))
				{
					transition.invertCondition = !transition.invertCondition;
					component.m_IsChanged = true;
				}
				ImGui::NextColumn();

				// Condition (기존 방식, 드래그앤드롭으로 메소드 할당)
				ImGui::Text("Condition:");
				ImGui::NextColumn();
				{
					std::string condMethod =
						transition.conditionName.empty() ? "Drop Method Here" : transition.conditionName;
					if (ImGui::Button(condMethod.c_str(), ImVec2(-1, 0)))
					{
						// 클릭 동작 없음
					}
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("METHOD_ITEM"))
						{
							std::string methodStr = std::string((const char *)payload->Data);
							if (transition.conditionName != methodStr)
							{
								transition.conditionName = methodStr;
								if (component.m_Methods.find(methodStr) != component.m_Methods.end())
									transition.condition = component.m_Methods[methodStr];
								component.m_IsChanged = true;
							}
						}
						ImGui::EndDragDropTarget();
					}
				}
				ImGui::Columns(1);
			}

			// (아래에서 component.m_IsChanged를 이용해 저장이나 후처리 작업을 진행할 수 있습니다.)
			if (component.m_IsChanged)
			{
				AL_CORE_INFO("something is changed");
				component.m_IsChanged = false;
			}

			ImGui::Separator();

			// 하단: Methods 리스트 (드래그앤드롭 가능)
			ImGui::Text("Available Methods:");
			if (ImGui::Button("Reload", ImVec2(80, 20)))
			{
				component.m_Methods = ScriptingEngine::getBooleanMethods(entity);
			}
			ImGui::BeginChild("MethodsList", ImVec2(200, 150), true);
			for (auto &pair : component.m_Methods)
			{
				ImGui::Selectable(pair.first.c_str());
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
				{
					std::string payload = pair.first;
					ImGui::SetDragDropPayload("METHOD_ITEM", &payload, payload.size());
					ImGui::Text("%s", pair.first.c_str());
					ImGui::EndDragDropSource();
				}
			}
			ImGui::EndChild();
		}
		else
		{
			ImGui::Button("?##statemanager_help", ImVec2(20, 20));
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("StateManager is need ScriptComponent");
			}
		}
	});

	drawComponent<MeshRendererComponent>("MeshRenderer", entity, [entity, scene = m_Context](auto &component) mutable {
		uint32_t meshType = component.type;
		std::shared_ptr<RenderingComponent> rc = component.m_RenderingComponent;

		const char *meshTypeStrings[] = {"None", "Box", "Sphere", "Plane", "Ground", "Capsule", "Cylinder", "Model"};
		const char *currentMeshTypeString = meshTypeStrings[(int)meshType];

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, 100.0f);

		ImGui::Text("Primitive Type");
		ImGui::NextColumn();
		if (ImGui::BeginCombo("##MeshTypeCombo", currentMeshTypeString))
		{
			for (int32_t i = 0; i < 8; ++i)
			{
				bool isSelected = currentMeshTypeString == meshTypeStrings[i];

				if (ImGui::Selectable(meshTypeStrings[i], isSelected))
				{
					currentMeshTypeString = meshTypeStrings[i];
					// model 정보 바꾸기

					component.type = i;
					if (i == 0 || i == 7)
					{
						break;
					}
					else
					{
						component.m_RenderingComponent->cleanup();
						component.m_RenderingComponent =
							RenderingComponent::createRenderingComponent(scene->getDefaultModel(i));
						component.path.clear();
						component.matPath.clear();
						component.isMatChanged = false;
						scene->insertEntityInCullTree(entity);
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
					component.m_RenderingComponent->cleanup();
					component.m_RenderingComponent = RenderingComponent::createRenderingComponent(model);
					component.type = 7;
					component.path = filePath.string();
					component.isMatChanged = false;
					scene->insertEntityInCullTree(entity);

					// SAC가 존재하는 경우 모델 갱신
					if (entity.hasComponent<SkeletalAnimatorComponent>())
					{
						auto &sa = entity.getComponent<SkeletalAnimatorComponent>();
						auto *sac = sa.sac.get();

						if (component.m_RenderingComponent != nullptr)
							sac->setModel(component.m_RenderingComponent->getModel());
					}
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
					component.m_RenderingComponent->updateMaterial(
						Model::createModel(filePath.string(), scene->getDefaultMaterial()));
					component.matPath = filePath.string();
					component.isMatChanged = true;

					// SAC가 존재하는 경우 모델 갱신
					if (entity.hasComponent<SkeletalAnimatorComponent>())
					{
						auto &sa = entity.getComponent<SkeletalAnimatorComponent>();

						auto *sac = sa.sac.get();
						if (component.m_RenderingComponent != nullptr)
							sac->setModel(component.m_RenderingComponent->getModel());
					}
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (rc != nullptr)
		{
			auto &materials = rc->getMaterials();

			static int32_t idx = 0;
			ImGui::DragInt("Material Index", &idx, 0.1f, 0, materials.size() - 1, "%d");
			idx = std::clamp(idx, 0, static_cast<int32_t>(materials.size()) - 1);

			std::shared_ptr<Material> &material = materials[idx];
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
		}
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
					if (field.m_Type == EScriptFieldType::BOOL)
					{
						bool data = scriptInstance->getFieldValue<bool>(name);
						if (ImGui::Checkbox(name.c_str(), &data))
						{
							scriptInstance->setFieldValue(name, data);
						}
					}
					if (field.m_Type == EScriptFieldType::STRING)
					{
						char buffer[128];
						memset(buffer, 0, sizeof(buffer));
						MonoString* monoStr = (MonoString*)scriptInstance->getFieldValue<MonoString*>(name);
						if (monoStr)
						{
							strncpy_s(buffer, sizeof(buffer), utils::monoStringToString2(monoStr).c_str(), sizeof(buffer));
							if (ImGui::InputText(name.c_str(), buffer, sizeof(buffer)))
							{
								scriptInstance->setFieldValue(name, utils::stringToMonoString(std::string(buffer)));
							}
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
		drawCheckBox("Gravity", component.m_UseGravity);

		const char *bodyTypeStrings[] = {"Static", "Dynamic"};
		const char *currentBodyTypeString = bodyTypeStrings[(int)component.m_Type];

		if (ImGui::BeginCombo("##MeshTypeCombo", currentBodyTypeString))
		{
			for (int32_t i = 0; i < 2; ++i)
			{
				bool isSelected = currentBodyTypeString == bodyTypeStrings[i];

				if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
				{
					currentBodyTypeString = bodyTypeStrings[i];
					if (i == 0)
						component.m_Type = RigidbodyComponent::EBodyType::Static;
					else
						component.m_Type = RigidbodyComponent::EBodyType::Dynamic;
				}
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		// FreezePos
		ImGui::Text("FreezePos");
		ImGui::SameLine();
		bool freezePosX = component.m_FreezePos.x == 1 ? false : true;
		ImGui::PushID("PX");
		if (ImGui::Checkbox("X", &freezePosX))
		{
			component.m_FreezePos.x = freezePosX ? 0 : 1;
		}
		ImGui::PopID();

		ImGui::SameLine();
		bool freezePosY = component.m_FreezePos.y == 1 ? false : true;
		ImGui::PushID("PY");
		if (ImGui::Checkbox("Y", &freezePosY))
		{
			component.m_FreezePos.y = freezePosY ? 0 : 1;
		}
		ImGui::PopID();

		ImGui::SameLine();
		bool freezePosZ = component.m_FreezePos.z == 1 ? false : true;
		ImGui::PushID("PZ");
		if (ImGui::Checkbox("Z", &freezePosZ))
		{
			component.m_FreezePos.z = freezePosZ ? 0 : 1;
		}
		ImGui::PopID();

		// FreezeRot
		ImGui::Text("FreezeRot");
		ImGui::SameLine();
		ImGui::PushID("RX");
		bool freezeRotX = component.m_FreezeRot.x == 1 ? false : true;
		if (ImGui::Checkbox("X", &freezeRotX))
		{
			component.m_FreezeRot.x = freezeRotX ? 0 : 1;
		}
		ImGui::PopID();

		ImGui::SameLine();
		bool freezeRotY = component.m_FreezeRot.y == 1 ? false : true;
		ImGui::PushID("RY");
		if (ImGui::Checkbox("Y", &freezeRotY))
		{
			component.m_FreezeRot.y = freezeRotY ? 0 : 1;
		}
		ImGui::PopID();

		ImGui::SameLine();
		bool freezeRotZ = component.m_FreezeRot.z == 1 ? false : true;
		ImGui::PushID("RZ");
		if (ImGui::Checkbox("Z", &freezeRotZ))
		{
			component.m_FreezeRot.z = freezeRotZ ? 0 : 1;
		}
		ImGui::PopID();
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
		drawCheckBox("IsActive", component.m_IsActive);
		drawFloatControl("Friction", component.m_Friction);
		drawFloatControl("Restitution", component.m_Restitution);
		// Runtime 중 수정 기능
	});
	drawComponent<SphereColliderComponent>("SphereCollider", entity, [](auto &component) {
		drawVec3Control("Center", component.m_Center);
		drawFloatControl("Radius", component.m_Radius);
		drawCheckBox("IsTrigger", component.m_IsTrigger);
		drawCheckBox("IsActive", component.m_IsActive);
		drawFloatControl("Friction", component.m_Friction);
		drawFloatControl("Restitution", component.m_Restitution);
		// Runtime 중 수정 기능
	});
	drawComponent<CapsuleColliderComponent>("CapsuleCollider", entity, [](auto &component) {
		drawVec3Control("Center", component.m_Center);
		drawFloatControl("Radius", component.m_Radius);
		drawFloatControl("Height", component.m_Height);
		drawCheckBox("IsTrigger", component.m_IsTrigger);
		drawCheckBox("IsActive", component.m_IsActive);
		drawFloatControl("Friction", component.m_Friction);
		drawFloatControl("Restitution", component.m_Restitution);
		// Runtime 중 수정 기능
	});
	drawComponent<CylinderColliderComponent>("CylinderCollider", entity, [](auto &component) {
		drawVec3Control("Center", component.m_Center);
		drawFloatControl("Radius", component.m_Radius);
		drawFloatControl("Height", component.m_Height);
		drawCheckBox("IsTrigger", component.m_IsTrigger);
		drawCheckBox("IsActive", component.m_IsActive);
		drawFloatControl("Friction", component.m_Friction);
		drawFloatControl("Restitution", component.m_Restitution);
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

template <> void SceneHierarchyPanel::displayAddComponentEntry<MeshRendererComponent>(const std::string &entryName)
{
	if (!m_SelectionContext.hasComponent<MeshRendererComponent>())
	{
		if (ImGui::Selectable(entryName.c_str()))
		{
			auto &mc = m_SelectionContext.addComponent<MeshRendererComponent>();
			std::shared_ptr<Model> &model = m_Context->getBoxModel();

			mc.type = 1;
			mc.m_RenderingComponent = RenderingComponent::createRenderingComponent(model);
			mc.cullSphere = mc.m_RenderingComponent->getCullSphere();

			m_Context->insertEntityInCullTree(m_SelectionContext);
			ImGui::CloseCurrentPopup();
		}
	}
}
} // namespace ale