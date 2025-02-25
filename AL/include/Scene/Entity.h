#pragma once

#include "Scene/Component.h"
#include "Scene/Scene.h"

namespace ale
{
/**
 * @class Entity
 * @brief ECS(Entity Component System)에서 엔티티를 나타내는 클래스.
 */
class Entity
{
  public:
	/// @brief Entity 기본 생성자
	Entity() = default;

	/**
	 * @brief Entity 생성자.
	 * @param handle entt 엔티티 핸들.
	 * @param scene 이 엔티티가 속한 씬(Scene) 객체.
	 */
	Entity(entt::entity handle, Scene *scene);

	/**
	 * @brief 복사 생성자.
	 * @param other 복사할 다른 Entity 객체.
	 */
	Entity(const Entity &other) = default;

	/**
	 * @brief 새로운 컴포넌트를 추가합니다.
	 * @tparam T 추가할 컴포넌트 타입.
	 * @tparam Args 컴포넌트 생성자에 전달할 인자들.
	 * @param args 컴포넌트 생성자 인자.
	 * @return T& 추가된 컴포넌트의 참조.
	 */
	template <typename T, typename... Args> T &addComponent(Args &&...args)
	{
		// ASSERT NOT HAS COMPONENT
		T &component = m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
		m_Scene->onComponentAdded<T>(*this, component);
		return component;
	}

	/**
	 * @brief 컴포넌트를 추가하거나 기존 컴포넌트를 교체합니다.
	 * @tparam T 추가 또는 교체할 컴포넌트 타입.
	 * @tparam Args 컴포넌트 생성자에 전달할 인자들.
	 * @param args 컴포넌트 생성자 인자.
	 * @return T& 추가되거나 교체된 컴포넌트의 참조.
	 */
	template <typename T, typename... Args> T &addOrReplaceComponent(Args &&...args)
	{
		// ASSERT NOT HAS COMPONENT
		T &component = m_Scene->m_Registry.emplace_or_replace<T>(m_EntityHandle, std::forward<Args>(args)...);
		m_Scene->onComponentAdded<T>(*this, component);
		return component;
	}

	/**
	 * @brief 특정 타입의 컴포넌트를 가져옵니다.
	 * @tparam T 가져올 컴포넌트 타입.
	 * @return T& 요청된 컴포넌트의 참조.
	 */
	template <typename T> T &getComponent()
	{
		// ASSERT HAS COMPONENT
		return m_Scene->m_Registry.get<T>(m_EntityHandle);
	}

	/**
	 * @brief 특정 타입의 컴포넌트를 제거합니다.
	 * @tparam T 제거할 컴포넌트 타입.
	 */
	template <typename T> void removeComponent()
	{
		// ASSERT HAS COMPONENT
		m_Scene->m_Registry.remove<T>(m_EntityHandle);
	}

	/**
	 * @brief MeshRendererComponent의 특수화된 삭제 함수.
	 */
	template <> void removeComponent<MeshRendererComponent>()
	{
		m_Scene->removeEntityInCullTree(*this);
		m_Scene->m_Registry.remove<MeshRendererComponent>(m_EntityHandle);
	}

	/**
	 * @brief 엔티티가 특정 컴포넌트를 가지고 있는지 확인합니다.
	 * @tparam T 확인할 컴포넌트 타입.
	 * @return true 컴포넌트를 가지고 있음.
	 * @return false 컴포넌트를 가지고 있지 않음.
	 */
	template <typename T> bool hasComponent()
	{
		return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
	}

	/**
	 * @brief 엔티티의 UUID를 반환합니다.
	 * @return UUID 엔티티의 UUID.
	 */
	UUID getUUID()
	{
		return getComponent<IDComponent>().m_ID;
	}

	/**
	 * @brief 엔티티의 태그(이름)를 반환합니다.
	 * @return const std::string& 엔티티의 태그.
	 */
	const std::string &getTag()
	{
		return getComponent<TagComponent>().m_Tag;
	}

	/**
	 * @brief 엔티티가 유효한지 확인하는 연산자.
	 * @return true 유효한 엔티티.
	 * @return false 유효하지 않은 엔티티.
	 */
	operator bool() const
	{
		return m_EntityHandle != entt::null;
	}

	/**
	 * @brief entt::entity로 변환하는 연산자.
	 * @return entt::entity 엔티티 핸들.
	 */
	operator entt::entity() const
	{
		return m_EntityHandle;
	}

	/**
	 * @brief uint32_t로 변환하는 연산자.
	 * @return uint32_t 엔티티 핸들 값을 반환.
	 */
	operator uint32_t() const
	{
		return (uint32_t)m_EntityHandle;
	}

	/**
	 * @brief 두 엔티티가 동일한지 비교하는 연산자.
	 * @param other 비교할 다른 엔티티.
	 * @return true 동일한 엔티티.
	 * @return false 다른 엔티티.
	 */
	bool operator==(const Entity &other) const
	{
		return m_EntityHandle == other.m_EntityHandle && m_Scene == other.m_Scene;
	}

	/**
	 * @brief 두 엔티티가 다른지 비교하는 연산자.
	 * @param other 비교할 다른 엔티티.
	 * @return true 다른 엔티티.
	 * @return false 동일한 엔티티.
	 */
	bool operator!=(const Entity &other) const
	{
		return !(*this == other);
	}

  private:
	entt::entity m_EntityHandle{entt::null};
	Scene *m_Scene = nullptr;
};
} // namespace ale
