#pragma once

#include "entt.hpp"

#include "Core/Timestep.h"
#include "Core/UUID.h"
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

class Scene
{
  public:
	Scene() = default;
	~Scene();

	static std::shared_ptr<Scene> copyScene(std::shared_ptr<Scene> scene);

	static std::shared_ptr<Scene> createScene();
	Entity createEntity(const std::string &name = "");
	Entity createEntityWithUUID(UUID uuid, const std::string &name = "");
	void destroyEntity(Entity entity);
	void insertDestroyEntity(Entity entity);
	void destroyEntities();

	void onRuntimeStart();
	void onRuntimeStop();

	void onUpdateEditor(EditorCamera &camera);
	void onUpdateRuntime(Timestep ts);
	void onViewportResize(uint32_t width, uint32_t height);

	Entity duplicateEntity(Entity entity);

	void step(int32_t frames);

	void setPaused(bool pause)
	{
		m_IsPaused = pause;
	}

	bool isRunning() const
	{
		return m_IsRunning;
	}

	glm::vec3 &getLightPos()
	{
		return m_lightPos;
	}
	glm::vec3 &getCamPos()
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

	std::shared_ptr<Model> getDefaultModel(int32_t idx);

	Entity findEntityByName(std::string_view name);
	Entity getEntityByUUID(UUID uuid);

	template <typename... Components> auto getAllEntitiesWith()
	{
		return m_Registry.view<Components...>();
	}

	template <typename... Components> auto &getComponent(entt::entity entity)
	{
		return m_Registry.get<Components...>(entity);
	}

	// frustumCulling
	void frustumCulling(const Frustum &frustum);
	void initFrustumDrawFlag();
	void removeEntityInCullTree(int32_t nodeId);
	int32_t insertEntityInCullTree(const CullSphere &sphere, entt::entity entityHandle);
	void printCullTree();

  private:
	template <typename T> void onComponentAdded(Entity entity, T &component);

	void initScene();
	void renderScene(EditorCamera &camera);
	void onPhysicsStart();
	void onPhysicsStop();

	void setCamPos(glm::vec3 &pos)
	{
		m_CameraPos = pos;
	}

  private:
	entt::registry m_Registry;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

	glm::vec3 m_lightPos{0.0f, 1.0f, 0.0f};
	glm::vec3 m_CameraPos{0.0f, 0.0f, 10.0f};
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

	World *m_World = nullptr;

	float m_ambientStrength{0.1f};

	CullTree m_cullTree;

	friend class Entity;
	friend class SceneSerializer;
	friend class SceneHierarchyPanel;
	friend class CullTree;
};
} // namespace ale
