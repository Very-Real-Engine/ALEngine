#pragma once

#include "Core/Base.h"
#include "Scene/Entity.h"
#include "Scene/Scene.h"

namespace ale
{
/**
 * @class SceneHierarchyPanel
 * @brief 씬 계층 구조 패널(Scene Hierarchy Panel)을 관리하는 클래스.
 *        씬 내의 엔티티를 시각적으로 표시하고, 선택 및 편집할 수 있도록 지원합니다.
 */
class SceneHierarchyPanel
{
  public:
	/// @brief SceneHierarchyPanel 생성자.
	SceneHierarchyPanel() = default;

	/**
	 * @brief 특정 씬 컨텍스트를 사용하여 SceneHierarchyPanel을 생성합니다.
	 * @param context 설정할 씬 객체의 공유 포인터.
	 */
	SceneHierarchyPanel(const std::shared_ptr<Scene> &context);

	/**
	 * @brief 현재 씬 컨텍스트를 변경합니다.
	 * @param context 변경할 씬 객체의 공유 포인터.
	 */
	void setContext(const std::shared_ptr<Scene> &context);
	
	/**
	 * @brief ImGui를 사용하여 씬 계층 구조 패널 UI를 렌더링합니다.
	 */
	void onImGuiRender();

	/**
	 * @brief 현재 선택된 엔티티를 반환합니다.
	 * @return Entity 현재 선택된 엔티티.
	 */
	Entity getSelectedEntity() const
	{
		return m_SelectionContext;
	}

	/**
	 * @brief 선택된 엔티티를 설정합니다.
	 * @param entity 선택할 엔티티.
	 */
	void setSelectedEntity(Entity entity);

  private:
	/**
	 * @brief 특정 컴포넌트를 추가하는 옵션을 UI에 표시합니다.
	 * @tparam T 추가할 컴포넌트 타입.
	 * @param entryName UI에서 표시할 컴포넌트 이름.
	 */
	template <typename T> void displayAddComponentEntry(const std::string &entryName);

	/**
	 * @brief 특정 엔티티의 노드를 계층 구조에서 그립니다.
	 * @param entity 표시할 엔티티.
	 */
	void drawEntityNode(Entity entity);

	/**
	 * @brief 특정 엔티티의 컴포넌트 정보를 UI에 표시합니다.
	 * @param entity 표시할 엔티티.
	 */
	void drawComponents(Entity entity);

	/**
	 * @brief 활성 상태 정보를 업데이트합니다.
	 * @param entity 업데이트할 엔티티.
	 * @param parentEffectiveActive 부모의 활성 상태.
	 */
	void updateActiveInfo(Entity &entity, bool parentEffectiveActive);

	/**
	 * @brief 특정 자식 엔티티를 새로운 부모 엔티티에 연결합니다.
	 * @param newParent 새로운 부모 엔티티.
	 * @param child 자식 엔티티.
	 */
	void updateRelationship(Entity &newParent, Entity &child);

	/**
	 * @brief 엔티티의 관계를 업데이트합니다.
	 * @param entity 업데이트할 엔티티.
	 */
	void updateRelationship(Entity &entity);

	/**
	 * @brief 엔티티의 변환(Transform) 정보를 업데이트합니다.
	 * @param entity 업데이트할 엔티티.
	 */
	void updateTransforms(Entity entity);

	/**
	 * @brief 엔티티의 변환을 부모의 변환을 기준으로 재귀적으로 업데이트합니다.
	 * @param entity 업데이트할 엔티티.
	 * @param parentWorldTransform 부모의 월드 변환 행렬.
	 */
	void updateTransformRecursive(Entity entity, const glm::mat4 &parentWorldTransform);

  private:
	std::shared_ptr<Scene> m_Context;
	Entity m_SelectionContext;
};
} // namespace ale
