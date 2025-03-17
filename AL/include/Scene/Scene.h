#pragma once

#include "entt.hpp"

#include "Core/Timestep.h"
#include "Core/UUID.h"
#include <alglm/include/alglm.h>
#include <glm/glm.hpp>

#include "Renderer/EditorCamera.h"
#include "Renderer/Material.h"

#include <queue>

namespace ale
{
class Entity;
class Model;
class CullTree;
class World;

struct Frustum;

/**
 * @class Scene
 * @brief ECS(Entity Component System) 기반의 씬(Scene) 관리 클래스.
 */
class Scene
{
  public:
	/// @brief Scene 기본 생성자.
	Scene() = default;

	/// @brief Scene 기본 소멸자.
	~Scene();

	/**
	 * @brief 기존 씬을 복사하여 새 씬을 생성합니다.
	 * @param scene 복사할 씬 객체.
	 * @return std::shared_ptr<Scene> 복사된 씬 객체.
	 */
	static std::shared_ptr<Scene> copyScene(std::shared_ptr<Scene> scene);

	/**
	 * @brief 새로운 씬을 생성합니다.
	 * @return std::shared_ptr<Scene> 생성된 씬 객체.
	 */
	static std::shared_ptr<Scene> createScene();

	/**
	 * @brief 새로운 엔티티를 생성합니다.
	 * @param name 엔티티 이름 (기본값: 빈 문자열).
	 * @return Entity 생성된 엔티티 객체.
	 */
	Entity createEntity(const std::string &name = "");

	/**
	 * @brief 특정 UUID를 가진 엔티티를 생성합니다.
	 * @param uuid 엔티티의 UUID.
	 * @param name 엔티티 이름.
	 * @return Entity 생성된 엔티티 객체.
	 */
	Entity createEntityWithUUID(UUID uuid, const std::string &name = "");

	/**
	 * @brief 엔티티를 삭제합니다.
	 * @param entity 삭제할 엔티티 객체.
	 */
	void destroyEntity(Entity entity);

	/**
	 * @brief 엔티티를 삭제 대기열에 추가합니다.
	 * @param entity 삭제할 엔티티 객체.
	 */
	void insertDestroyEntity(Entity entity);

	/// @brief 씬 내의 모든 엔티티를 삭제합니다.
	void destroyEntities();

	/// @brief 씬을 정리합니다.
	void cleanup();

	/**
	 * @brief 기본 도형 메시 엔티티를 생성합니다.
	 * @param name 엔티티 이름.
	 * @param idx 기본 메시 인덱스.
	 * @return Entity 생성된 엔티티 객체.
	 */
	Entity createPrimitiveMeshEntity(const std::string &name, uint32_t idx);

	/// @brief 런타임 시작 시 호출됩니다.
	void onRuntimeStart();

	/// @brief 런타임 종료 시 호출됩니다.
	void onRuntimeStop();

  /**
   * @brief 에디터에서 추가 렌더링이 필요한 조건에 렌더링 적용합니다.
   * @param Timestep 시간
   * @param bool init 초기화 기본값:false
   */
	void preRenderEditor(const Timestep& ts, bool init = false);
	/**
	 * @brief 에디터에서 씬을 업데이트합니다.
	 * @param camera 에디터 카메라.
	 */
	void onUpdateEditor(EditorCamera &camera);

	/**
	 * @brief 런타임에서 씬을 업데이트합니다.
	 * @param ts 시간 간격(Timestep).
	 */
	void onUpdateRuntime(Timestep ts);

	/**
	 * @brief 뷰포트 크기 변경 시 호출됩니다.
	 * @param width 새로운 너비.
	 * @param height 새로운 높이.
	 */
	void onViewportResize(uint32_t width, uint32_t height);

	/**
	 * @brief 엔티티를 복제합니다.
	 * @param entity 복제할 엔티티 객체.
	 * @return Entity 복제된 엔티티 객체.
	 */
	Entity duplicateEntity(Entity entity);

	/**
	 * @brief 물리 시뮬레이션을 지정된 프레임 수만큼 실행합니다.
	 * @param frames 실행할 프레임 수.
	 */
	void step(int32_t frames);

	/**
	 * @brief 씬의 실행 상태를 설정합니다.
	 * @param pause true이면 일시 정지, false이면 실행.
	 */
	void setPaused(bool pause)
	{
		m_IsPaused = pause;
	}

	/**
	 * @brief 씬이 실행 중인지 확인합니다.
	 * @return true 실행 중.
	 * @return false 실행 중이 아님.
	 */
	bool isRunning() const
	{
		return m_IsRunning;
	}

	alglm::vec3 &getCamPos()
	{
		return m_CameraPos;
	}
	float &getAmbientStrength()
	{
		return m_ambientStrength;
	}
	std::shared_ptr<Material> &getDefaultMaterial()
	{
		return m_defaultMaterial;
	}
	std::shared_ptr<Model> &getBoxModel()
	{
		return m_boxModel;
	}
	std::shared_ptr<Model> &getSphereModel()
	{
		return m_sphereModel;
	}
	std::shared_ptr<Model> &getPlaneModel()
	{
		return m_planeModel;
	}
	std::shared_ptr<Model> &getGroundModel()
	{
		return m_groundModel;
	}
	std::shared_ptr<Model> &getCapsuleModel()
	{
		return m_capsuleModel;
	}
	std::shared_ptr<Model> &getCylinderModel()
	{
		return m_cylinderModel;
	}

	std::shared_ptr<Model> getDefaultModel(int32_t idx);

	/**
	 * @brief 특정 엔티티를 이름으로 찾습니다.
	 * @param name 찾을 엔티티의 이름.
	 * @return Entity 찾은 엔티티 객체.
	 */
	Entity findEntityByName(std::string_view name);

	/**
	 * @brief 특정 UUID를 가진 엔티티를 찾습니다.
	 * @param uuid 찾을 엔티티의 UUID.
	 * @return Entity 찾은 엔티티 객체.
	 */
	Entity getEntityByUUID(UUID uuid);

	/**
	 * @brief 특정 컴포넌트를 가진 모든 엔티티를 가져옵니다.
	 * @tparam Components 검색할 컴포넌트들.
	 * @return auto 컴포넌트를 가진 엔티티 뷰.
	 */
	template <typename... Components> auto getAllEntitiesWith()
	{
		return m_Registry.view<Components...>();
	}

	/**
	 * @brief 특정 엔티티에서 컴포넌트를 가져옵니다.
	 * @tparam Components 가져올 컴포넌트들.
	 * @param entity 대상 엔티티.
	 * @return auto& 가져온 컴포넌트의 참조.
	 */
	template <typename... Components> auto &getComponent(entt::entity entity)
	{
		return m_Registry.get<Components...>(entity);
	}

	template <typename Component> auto tryGet(entt::entity entity)
	{
		return m_Registry.try_get<Component>(entity);
	}

	// frustumCulling
	void frustumCulling(const Frustum &frustum);
	void initFrustumDrawFlag();
	void removeEntityInCullTree(Entity &entity);
	void insertEntityInCullTree(Entity &entity);
	void setNoneInCullTree(Entity &entity);
	void unsetNoneInCullTree(Entity &entity);
	void printCullTree();

	void removeColliderShaderResourceManager(Entity &entity);

  private:
	template <typename T> void onComponentAdded(Entity entity, T &component);

	void initScene();
	void renderScene(EditorCamera &camera);
	void onPhysicsStart();
	void onPhysicsStop();
	void findMoveObject();

	void setCamPos(alglm::vec3 &pos)
	{
		m_CameraPos = pos;
	}

  private:
	entt::registry m_Registry;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

	alglm::vec3 m_lightPos{0.0f, 1.0f, 0.0f};
	alglm::vec3 m_CameraPos{0.0f, 0.0f, 10.0f};
	bool m_IsPaused = false;
	bool m_IsRunning = false;
	int32_t m_StepFrames = 0;

	std::unordered_map<UUID, entt::entity> m_EntityMap;
	std::queue<entt::entity> m_DestroyQueue;

	DefaultTextures m_defaultTextures;
	std::shared_ptr<Material> m_defaultMaterial;
	std::shared_ptr<Model> m_boxModel;
	std::shared_ptr<Model> m_sphereModel;
	std::shared_ptr<Model> m_planeModel;
	std::shared_ptr<Model> m_groundModel;
	std::shared_ptr<Model> m_capsuleModel;
	std::shared_ptr<Model> m_cylinderModel;
	std::shared_ptr<Model> m_colliderBoxModel;

	World *m_World = nullptr;

	float m_ambientStrength{0.1f};

	CullTree m_cullTree;

	friend class Entity;
	friend class SceneSerializer;
	friend class SceneHierarchyPanel;
	friend class CullTree;
};
} // namespace ale
