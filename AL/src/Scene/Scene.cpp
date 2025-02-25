#include "Scene/Scene.h"
#include "Scene/Component.h"
#include "Scene/CullTree.h"
#include "Scene/Entity.h"
#include "Scene/ScriptableEntity.h"

#include "Core/App.h"

#include "Renderer/RenderingComponent.h"

#include "Scripting/ScriptingEngine.h"

#include "Physics/Fixture.h"
#include "Physics/Rigidbody.h"
#include "Physics/World.h"

#include "Physics/Shape/BoxShape.h"
#include "Physics/Shape/CapsuleShape.h"
#include "Physics/Shape/CylinderShape.h"
#include "Physics/Shape/SphereShape.h"

namespace ale
{
Scene::~Scene()
{
}

template <typename... Component>
static void copyComponent(entt::registry &dst, entt::registry &src,
						  const std::unordered_map<UUID, entt::entity> &enttMap)
{
	(
		[&]() {
			auto view = src.view<Component>();
			for (auto srcEntity : view)
			{
				entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).m_ID);

				auto &srcComponent = src.get<Component>(srcEntity);
				dst.emplace_or_replace<Component>(dstEntity, srcComponent);
			}
		}(),
		...);
}

template <typename... Component>
static void copyComponent(ComponentGroup<Component...>, entt::registry &dst, entt::registry &src,
						  const std::unordered_map<UUID, entt::entity> &enttMap)
{
	copyComponent<Component...>(dst, src, enttMap);
}

template <typename... Component> static void copyComponentIfExists(Entity dst, Entity src)
{
	(
		[&]() {
			if (src.hasComponent<Component>())
				dst.addOrReplaceComponent<Component>(src.getComponent<Component>());
		}(),
		...);
}

template <typename... Component> static void copyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
{
	copyComponentIfExists<Component...>(dst, src);
}

std::shared_ptr<Scene> Scene::copyScene(std::shared_ptr<Scene> scene)
{
	std::shared_ptr<Scene> newScene = createScene();

	newScene->m_ViewportWidth = scene->m_ViewportWidth;
	newScene->m_ViewportHeight = scene->m_ViewportHeight;

	auto &srcRegistry = scene->m_Registry;
	auto &dstRegistry = newScene->m_Registry;
	std::unordered_map<UUID, entt::entity> enttMap;

	auto idView = srcRegistry.view<IDComponent>();
	for (auto e : idView)
	{
		UUID uuid = srcRegistry.get<IDComponent>(e).m_ID;
		const auto &name = srcRegistry.get<TagComponent>(e).m_Tag;
		Entity newEntity = newScene->createEntityWithUUID(uuid, name);
		enttMap[uuid] = (entt::entity)newEntity;
	}
	copyComponent(AllComponents{}, dstRegistry, srcRegistry, enttMap);

	newScene->m_cullTree = scene->m_cullTree;

	auto view = newScene->m_Registry.view<MeshRendererComponent>();
	for (auto entityHandle : view)
	{
		auto &mesh = dstRegistry.get<MeshRendererComponent>(entityHandle);
		newScene->m_cullTree.changeEntityHandle(mesh.nodeId, static_cast<uint32_t>(entityHandle));
	}
	newScene->initScene();
	return newScene;
}

std::shared_ptr<Scene> Scene::createScene()
{
	std::shared_ptr<Scene> scene = std::shared_ptr<Scene>(new Scene());
	scene->initScene();
	return scene;
}

Entity Scene::createEntity(const std::string &name)
{
	return createEntityWithUUID(UUID(), name);
}

Entity Scene::createEntityWithUUID(UUID uuid, const std::string &name)
{
	Entity entity = {m_Registry.create(), this};
	entity.addComponent<IDComponent>(uuid);
	entity.addComponent<TransformComponent>();
	entity.addComponent<RelationshipComponent>();
	auto &tag = entity.addComponent<TagComponent>();
	tag.m_Tag = name.empty() ? "Entity" : name;

	m_EntityMap[uuid] = entity;

	return entity;
}

void Scene::insertDestroyEntity(Entity entity)
{
	m_DestroyQueue.push(entity);
}

void Scene::destroyEntities()
{
	while (!m_DestroyQueue.empty())
	{
		Entity entity{m_DestroyQueue.front(), this};
		destroyEntity(entity);
		m_DestroyQueue.pop();
	}
}

void Scene::destroyEntity(Entity entity)
{
	if (!entity || !m_Registry.valid(entity))
		return;

	if (entity.hasComponent<RelationshipComponent>())
	{
		auto &relationship = entity.getComponent<RelationshipComponent>();

		// 자식 목록 복사(참조값이 파괴 중에 변동되면 안되므로)
		auto childrenCopy = relationship.children;

		// 자식들에 대해 destroyEntity를 재귀 호출
		for (auto childEntt : childrenCopy)
		{
			if (m_Registry.valid(childEntt))
			{
				Entity childEntity{childEntt, this};
				destroyEntity(childEntity);
			}
		}

		// 2) 부모의 자식 목록에서 자신을 제거
		if (relationship.parent != entt::null)
		{
			Entity parentEntity{relationship.parent, this};
			auto &parentRC = parentEntity.getComponent<RelationshipComponent>();

			// parentRC.children 에서 entity의 entt::entity 핸들을 제거
			auto it = std::find(parentRC.children.begin(), parentRC.children.end(), (entt::entity)entity);
			if (it != parentRC.children.end())
				parentRC.children.erase(it);
		}
	}

	removeEntityInCullTree(entity);
	m_EntityMap.erase(entity.getUUID());
	m_Registry.destroy(entity);
}

Entity Scene::createPrimitiveMeshEntity(const std::string &name, uint32_t idx)
{
	Entity entity = createEntity(name);

	auto &mc = entity.addComponent<MeshRendererComponent>();
	std::shared_ptr<Model> &model = getDefaultModel(idx);

	mc.type = idx;
	mc.m_RenderingComponent = RenderingComponent::createRenderingComponent(model);
	mc.cullSphere = mc.m_RenderingComponent->getCullSphere();

	insertEntityInCullTree(entity);

	return entity;
}

void Scene::onRuntimeStart()
{
	m_IsRunning = true;

	onPhysicsStart();

	{
		ScriptingEngine::onRuntimeStart(this);

		auto view = m_Registry.view<ScriptComponent>();

		for (auto e : view)
		{
			Entity entity = {e, this};
			ScriptingEngine::onCreateEntity(entity);
		}
	}
}

void Scene::onRuntimeStop()
{
	m_IsRunning = false;

	onPhysicsStop();

	ScriptingEngine::onRuntimeStop();
}

void Scene::onUpdateEditor(EditorCamera &camera)
{
	setCamPos(camera.getPosition());
	findMoveObject();
	renderScene(camera);
}

void Scene::onUpdateRuntime(Timestep ts)
{
	if (!m_IsPaused /*&& m_StepFrames-- > 0*/)
	{
		// update scripts
		{
			// Script
			auto view = m_Registry.view<ScriptComponent>();
			for (auto e : view)
			{
				Entity entity = {e, this};
				ScriptingEngine::onUpdateEntity(entity, ts);
			}

			// Native Script
			m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto &nsc) {
				if (!nsc.instance)
				{
					nsc.instance = nsc.instantiateScript();
					nsc.instance->m_Entity = Entity{entity, this};
					nsc.instance->onCreate();
				}
				nsc.instance->onUpdate(ts);
			});
		}
		// update Physics
		{
			m_World->startFrame();
			// Run physics
			m_World->runPhysics(ts);
			// set transforms of entity by body
			auto view = m_Registry.view<RigidbodyComponent>();
			for (auto e : view)
			{
				Entity entity = {e, this};
				auto &tf = entity.getComponent<TransformComponent>();
				auto &rb = entity.getComponent<RigidbodyComponent>();
				auto &mr = entity.getComponent<MeshRendererComponent>();

				Rigidbody *body = (Rigidbody *)rb.body;

				tf.m_Position = body->getTransform().position;
				tf.m_Rotation = glm::eulerAngles(body->getTransform().orientation);
				tf.m_WorldTransform = tf.getTransform();
			}
		}

		// update animations
		{
			auto view = m_Registry.view<SkeletalAnimatorComponent>();
			
			for (auto e : view)
			{
				Entity entity = {e, this};
				auto& sa = entity.getComponent<SkeletalAnimatorComponent>();

				SAComponent* sac = sa.sac.get();
				if (sa.m_IsPlaying)
					sac->updateAnimation(ts * sa.m_SpeedFactor, 0);
			}
		}
	}

	// find main camera
	Camera *mainCamera = nullptr;
	{
		auto view = m_Registry.view<TransformComponent, CameraComponent>();
		for (auto entity : view)
		{
			auto camera = view.get<CameraComponent>(entity);

			if (camera.m_Primary)
			{
				auto &tc = view.get<TransformComponent>(entity);
				camera.m_Camera.updateSceneCamera(tc.m_Position, tc.m_Rotation);
				mainCamera = &camera.m_Camera;
				break;
			}
		}
	}

	if (mainCamera)
	{
		Renderer &renderer = App::get().getRenderer();
		setCamPos(mainCamera->getPosition());
		findMoveObject();
		renderer.beginScene(this, *mainCamera);
	}
	else
	{
		// AL_CORE_ERROR("No Camera!");
		Renderer &renderer = App::get().getRenderer();
		renderer.biginNoCamScene();
	}

	// imguilayer::renderDrawData
}

void Scene::onViewportResize(uint32_t width, uint32_t height)
{
	if (m_ViewportWidth == width && m_ViewportHeight == height)
		return;

	m_ViewportWidth = width;
	m_ViewportHeight = height;

	// Resize non Fixed Aspect ratio camera
	auto view = m_Registry.view<CameraComponent>();
	for (auto entity : view)
	{
		auto &cameraComponent = view.get<CameraComponent>(entity);
		if (!cameraComponent.m_FixedAspectRatio)
			cameraComponent.m_Camera.setViewportSize(width, height); // set viewport size
	}
}

Entity Scene::duplicateEntity(Entity entity)
{
	std::string name = entity.getComponent<TagComponent>().m_Tag;
	Entity newEntity = createEntity(name);
	copyComponentIfExists(AllComponents{}, newEntity, entity);
	return newEntity;
}

void Scene::step(int32_t frames)
{
	m_StepFrames = frames;
}

void Scene::initScene()
{
	m_defaultTextures.albedo = Texture::createDefaultTexture(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	m_defaultTextures.normal = Texture::createDefaultTexture(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	m_defaultTextures.roughness = Texture::createDefaultTexture(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
	m_defaultTextures.metallic = Texture::createDefaultTexture(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	m_defaultTextures.ao = Texture::createDefaultTexture(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	m_defaultTextures.height = Texture::createDefaultTexture(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

	m_defaultMaterial = Material::createMaterial(
		{glm::vec3(1.0f, 1.0f, 1.0f), m_defaultTextures.albedo, false}, {m_defaultTextures.normal, false},
		{0.5f, m_defaultTextures.roughness, false}, {0.0f, m_defaultTextures.metallic, false},
		{1.0f, m_defaultTextures.ao, false}, {0.0f, m_defaultTextures.height, false});

	m_boxModel = Model::createBoxModel(m_defaultMaterial);
	m_sphereModel = Model::createSphereModel(m_defaultMaterial);
	m_planeModel = Model::createPlaneModel(m_defaultMaterial);
	m_groundModel = Model::createGroundModel(m_defaultMaterial);
	m_capsuleModel = Model::createCapsuleModel(m_defaultMaterial);
	m_cylinderModel = Model::createCylinderModel(m_defaultMaterial);

	m_cullTree.setScene(this);
}

void Scene::renderScene(EditorCamera &camera)
{
	Renderer &renderer = App::get().getRenderer();
	// Draw Models
	// renderer.drawFrame(this);
	renderer.beginScene(this, camera);
}

void Scene::onPhysicsStart()
{
	// create world
	m_World = new World();

	// body
	auto view = m_Registry.view<RigidbodyComponent>();
	for (auto e : view)
	{
		Entity entity = {e, this};
		auto &tf = entity.getComponent<TransformComponent>();
		auto &rb = entity.getComponent<RigidbodyComponent>();

		BodyDef bdDef;
		bdDef.m_type = EBodyType::DYNAMIC_BODY;
		bdDef.m_position = tf.m_Position;
		bdDef.m_orientation = glm::quat(tf.m_Rotation);
		bdDef.m_linearDamping = rb.m_Damping;
		bdDef.m_angularDamping = rb.m_AngularDamping;
		bdDef.m_gravityScale = 15.0f;
		bdDef.m_useGravity = rb.m_UseGravity;
		bdDef.m_posFreeze = rb.m_FreezePos;
		bdDef.m_rotFreeze = rb.m_FreezeRot;

		// create body
		Rigidbody *body = m_World->createBody(bdDef);
		// set fixed rotation
		// set body
		rb.body = body;

		if (entity.hasComponent<BoxColliderComponent>())
		{
			auto &bc = entity.getComponent<BoxColliderComponent>();

			// create shape
			BoxShape boxShape;
			// set shape vertices
			boxShape.setVertices(bc.m_Center, bc.m_Size);

			// set mass data(mass, inertia mass)
			float h = bc.m_Size.y;
			float w = bc.m_Size.x;
			float d = bc.m_Size.z;
			float Ixx = (1.0f / 12.0f) * (h * h + d * d) * rb.m_Mass;
			float Iyy = (1.0f / 12.0f) * (w * w + d * d) * rb.m_Mass;
			float Izz = (1.0f / 12.0f) * (w * w + h * h) * rb.m_Mass;
			glm::mat3 m(glm::vec3(Ixx, 0.0f, 0.0f), glm::vec3(0.0f, Iyy, 0.0f), glm::vec3(0.0f, 0.0f, Izz));

			body->setMassData(rb.m_Mass, m);

			FixtureDef fDef;
			fDef.shape = boxShape.clone();
			fDef.friction = 0.7f;
			fDef.restitution = 0.4f;

			// create fixture
			body->createFixture(&fDef);
		}

		// SphereColliderComponent
		if (entity.hasComponent<SphereColliderComponent>())
		{
			auto &sc = entity.getComponent<SphereColliderComponent>();

			// create shape
			SphereShape spShape;
			// set shape vertices
			spShape.setShapeFeatures(sc.m_Center, sc.m_Radius);

			// set mass data(mass, inertia mass)
			float r = spShape.m_radius;
			float val = (2.0f / 5.0f) * rb.m_Mass * r * r;
			glm::mat3 m(glm::vec3(val, 0.0f, 0.0f), glm::vec3(0.0f, val, 0.0f), glm::vec3(0.0f, 0.0f, val));

			body->setMassData(rb.m_Mass, m);

			FixtureDef fDef;
			fDef.shape = spShape.clone();
			fDef.friction = 0.4f;
			fDef.restitution = 0.8f;

			// create fixture
			body->createFixture(&fDef);
		}

		// CapsuleColliderComponent
		if (entity.hasComponent<CapsuleColliderComponent>())
		{
			auto &cc = entity.getComponent<CapsuleColliderComponent>();

			// create shape
			CapsuleShape csShape;
			// set shape vertices
			csShape.setShapeFeatures(cc.m_Center, cc.m_Radius, cc.m_Height);

			// set mass data(mass, inertia mass)

			float mh = rb.m_Mass * 0.25f;
			float r = csShape.m_radius;
			float h = csShape.m_height;
			float d = (3.0f * r / 8.0f);
			float val = (2.0f / 5.0f) * mh * r * r + (h / 2.0f * d * d);
			glm::mat3 ih(glm::vec3(val, 0.0f, 0.0f), glm::vec3(0.0f, val, 0.0f), glm::vec3(0.0f, 0.0f, val));

			float mc = rb.m_Mass * 0.75f;
			float Ixx = (1.0f / 12.0f) * (3.0f * r * r + h * h) * mc;
			float Iyy = Ixx;
			float Izz = (1.0f / 2.0f) * (r * r) * mc;
			glm::mat3 ic(glm::vec3(Ixx, 0.0f, 0.0f), glm::vec3(0.0f, Iyy, 0.0f), glm::vec3(0.0f, 0.0f, Izz));

			float mass = mh * 2.0f + mc;
			glm::mat3 m = ih * 2.0f + ic;

			body->setMassData(mass, m);

			FixtureDef fDef;
			fDef.shape = csShape.clone();
			fDef.friction = 0.4f;
			fDef.restitution = 0.4f;

			// create fixture
			body->createFixture(&fDef);
		}

		// CylinderColliderComponent
		if (entity.hasComponent<CylinderColliderComponent>())
		{
			auto &cc = entity.getComponent<CylinderColliderComponent>();

			// create shape
			CylinderShape cyShape;
			// set shape vertices
			cyShape.setShapeFeatures(cc.m_Center, cc.m_Radius, cc.m_Height);

			// set mass data(mass, inertia mass)
			float r = cyShape.m_radius;
			float h = cyShape.m_height;
			float Ixx = (1.0f / 12.0f) * (3.0f * r * r + h * h) * rb.m_Mass;
			float Iyy = (1.0f / 2.0f) * (r * r) * rb.m_Mass;
			float Izz = Ixx;
			glm::mat3 m(glm::vec3(Ixx, 0.0f, 0.0f), glm::vec3(0.0f, Iyy, 0.0f), glm::vec3(0.0f, 0.0f, Izz));

			body->setMassData(rb.m_Mass, m);

			FixtureDef fDef;
			fDef.shape = cyShape.clone();
			fDef.friction = 0.4f;
			fDef.restitution = 0.4f;

			// create fixture
			body->createFixture(&fDef);
		}
	}
}

void Scene::onPhysicsStop()
{
	// delete world
}

std::shared_ptr<Model> Scene::getDefaultModel(int32_t idx)
{
	// ASSERT idx
	switch (idx)
	{
	case 1:
		return m_boxModel;
	case 2:
		return m_sphereModel;
	case 3:
		return m_planeModel;
	case 4:
		return m_groundModel;
	case 5:
		return m_capsuleModel;
	case 6:
		return m_cylinderModel;
	default:
		return nullptr;
	}
}

Entity Scene::findEntityByName(std::string_view name)
{
	auto view = m_Registry.view<TagComponent>();
	for (auto entity : view)
	{
		const TagComponent &tc = view.get<TagComponent>(entity);
		if (tc.m_Tag == name)
			return Entity{entity, this};
	}
	return {};
}

Entity Scene::getEntityByUUID(UUID uuid)
{
	if (m_EntityMap.find(uuid) != m_EntityMap.end())
		return {m_EntityMap.at(uuid), this};

	return {};
}

void Scene::insertEntityInCullTree(Entity &entity)
{
	if (!entity.hasComponent<MeshRendererComponent>())
	{
		return;
	}

	auto &mc = entity.getComponent<MeshRendererComponent>();
	if (mc.m_RenderingComponent != nullptr && mc.nodeId == NULL_NODE)
	{
		TransformComponent &tc = entity.getComponent<TransformComponent>();

		CullSphere sphere(tc.getTransform() * glm::vec4(mc.cullSphere.center, 1.0f),
						  mc.cullSphere.radius * tc.getMaxScale());

		mc.nodeId = m_cullTree.createNode(sphere, static_cast<uint32_t>(entity));
		mc.cullState = ECullState::CULL;
	}
}

void Scene::printCullTree()
{
	m_cullTree.printCullTree(m_cullTree.getRootNodeId());
}

void Scene::removeEntityInCullTree(Entity &entity)
{
	if (entity.hasComponent<MeshRendererComponent>())
	{
		auto &mc = entity.getComponent<MeshRendererComponent>();
		if (mc.m_RenderingComponent == nullptr)
			return;

		m_cullTree.destroyNode(mc.nodeId);
		mc.nodeId = NULL_NODE;
	}
}

void Scene::frustumCulling(const Frustum &frustum)
{
	m_cullTree.updateTree();

	int32_t root = m_cullTree.getRootNodeId();
	if (root != NULL_NODE)
	{
		m_cullTree.frustumCulling(frustum, root);
	}
}

void Scene::initFrustumDrawFlag()
{
	int32_t root = m_cullTree.getRootNodeId();
	if (root != NULL_NODE)
	{
		m_cullTree.setRenderDisable(root);
	}
}

void Scene::findMoveObject()
{
	auto view = m_Registry.view<MeshRendererComponent, TransformComponent>();
	for (auto e : view)
	{
		auto &transform = view.get<TransformComponent>(e);
		auto &mesh = view.get<MeshRendererComponent>(e);
		if (mesh.m_RenderingComponent == nullptr)
			continue;

		float limit = transform.getMaxScale() * mesh.cullSphere.radius * 0.1f;
		limit = limit * limit;

		if (glm::length2(transform.m_Position - transform.m_LastPosition) > limit)
		{
			transform.m_LastPosition = transform.m_Position;
			transform.m_isMoved = true;
		}
	}
}

void Scene::setNoneInCullTree(Entity &entity)
{
	if (entity.hasComponent<MeshRendererComponent>() == false)
	{
		auto &mc = entity.getComponent<MeshRendererComponent>();
		mc.cullState = (mc.cullState | ECullState::NONE);
	}
}

void Scene::unsetNoneInCullTree(Entity &entity)
{
	if (entity.hasComponent<MeshRendererComponent>() == false)
	{
		auto &mc = entity.getComponent<MeshRendererComponent>();
		mc.cullState = (mc.cullState & ECullState::RENDER);
	}
}

// 컴파일 타임에 조건 확인
template <typename T> void Scene::onComponentAdded(Entity entity, T &component)
{
	static_assert(sizeof(T) == 0);
}

template <> void Scene::onComponentAdded<IDComponent>(Entity entity, IDComponent &component)
{
}

template <> void Scene::onComponentAdded(Entity entity, TagComponent &component)
{
}

template <> void Scene::onComponentAdded(Entity entity, TransformComponent &component)
{
}

template <> void Scene::onComponentAdded(Entity entity, RelationshipComponent &component)
{
}

template <> void Scene::onComponentAdded<CameraComponent>(Entity entity, CameraComponent &component)
{
	if (m_ViewportWidth > 0 && m_ViewportHeight > 0)
		component.m_Camera.setViewportSize(m_ViewportWidth, m_ViewportHeight);
}

template <> void Scene::onComponentAdded<MeshRendererComponent>(Entity entity, MeshRendererComponent &component)
{
	component.type = 0;

	// 이미 SAC가 존재하는 경우 새로 생긴 모델 갱신
	if (entity.hasComponent<SkeletalAnimatorComponent>())
	{
		auto& sa = entity.getComponent<SkeletalAnimatorComponent>();

		auto* sac = sa.sac.get();
		if (component.m_RenderingComponent != nullptr)
			sac->setModel(component.m_RenderingComponent->getModel());
	}
}

template <> void Scene::onComponentAdded<LightComponent>(Entity entity, LightComponent &component)
{
	auto &tc = entity.getComponent<TransformComponent>();

	component.m_Light = std::make_shared<Light>(Light{tc.m_Position, glm::vec3(0.0f, -1.0f, 0.0f),
													  glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, glm::cos(glm::radians(12.5f)),
													  glm::cos(glm::radians(17.5f)), 1, 1, 0, glm::vec2(0.0f, 0.0f)});
}

template <> void Scene::onComponentAdded<RigidbodyComponent>(Entity entity, RigidbodyComponent &component)
{
}

template <> void Scene::onComponentAdded<BoxColliderComponent>(Entity entity, BoxColliderComponent &component)
{
	auto &tc = entity.getComponent<TransformComponent>();

	component.m_Size = tc.m_Scale;
}

template <> void Scene::onComponentAdded<SphereColliderComponent>(Entity entity, SphereColliderComponent &component)
{
}

template <> void Scene::onComponentAdded<CapsuleColliderComponent>(Entity entity, CapsuleColliderComponent &component)
{
}

template <> void Scene::onComponentAdded<CylinderColliderComponent>(Entity entity, CylinderColliderComponent &component)
{
}

template <> void Scene::onComponentAdded<ScriptComponent>(Entity entity, ScriptComponent &component)
{
}

template <> void Scene::onComponentAdded<SkeletalAnimatorComponent>(Entity entity, SkeletalAnimatorComponent &component)
{
	component.sac = std::make_shared<SAComponent>();
	component.m_Repeats = component.sac->getRepeatAll();

	if (entity.hasComponent<MeshRendererComponent>())
	{
		auto& mr = entity.getComponent<MeshRendererComponent>();

		if (mr.m_RenderingComponent != nullptr)
		{
			component.sac->setModel(mr.m_RenderingComponent->getModel());
		}
	}
}
  
void Scene::cleanup()
{
	// delete model
	m_boxModel->cleanup();
	m_sphereModel->cleanup();
	m_planeModel->cleanup();
	m_groundModel->cleanup();
	m_capsuleModel->cleanup();
	m_cylinderModel->cleanup();

	m_defaultMaterial->cleanup();

	auto view = m_Registry.view<MeshRendererComponent>();
	for (auto e : view)
	{
		auto &mesh = view.get<MeshRendererComponent>(e);
		mesh.m_RenderingComponent->cleanup();
	}
}

} // namespace ale