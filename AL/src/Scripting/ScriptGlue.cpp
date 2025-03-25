#include "alpch.h"

#include "Scripting/ScriptingEngine.h"
#include "Scripting/ScriptingGlue.h"

#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include "Core/UUID.h"

#include "Scene/Entity.h"
#include "Scene/Scene.h"

#include "mono/jit/jit.h"
#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/attrdefs.h"
#include "mono/metadata/mono-debug.h"
#include "mono/metadata/threads.h"

#include "Physics/Rigidbody.h"

namespace ale
{
namespace utils
{

std::string monoStringToString(MonoString *string)
{
	char *cStr = mono_string_to_utf8(string);
	std::string str(cStr);
	mono_free(cStr);
	return str;
}

} // namespace utils

static std::unordered_map<MonoType *, std::function<bool(Entity)>> s_EntityHasComponentFuncs;
static std::unordered_map<MonoType *, std::function<std::vector<uint64_t>(Scene*)>> s_EntityViewComponentsFuncs;

// static std::unordered_map<std::pair<UUID, MonoType *>, MonoObject *> s_EntityComponentInstanceMap;

#define ADD_INTERNAL_CALL(Name) mono_add_internal_call("ALEngine.InternalCalls::" #Name, Name)

// Native Function API

// Log
static void nativeLog(MonoString *string, int parameter)
{
	std::string str = utils::monoStringToString(string);
	std::cout << str << '\n';
}

static void nativeLog_Vector(alglm::vec3 *parameter, alglm::vec3 *outResult)
{
	// AL_CORE_WARN("Value: {0}", *parameter);
	*outResult = alglm::normalize(*parameter);
}

// Entity
static bool Entity_hasComponent(UUID entityID, MonoReflectionType *componentType)
{
	// ASSERT
	Scene *scene = ScriptingEngine::getSceneContext();
	Entity entity = scene->getEntityByUUID(entityID);

	MonoType *managedType = mono_reflection_type_get_type(componentType);
	return s_EntityHasComponentFuncs.at(managedType)(entity);
}

// static MonoObject *Entity_getComponent(UUID entityID, MonoReflectionType *componentType)
// {
// 	AL_CORE_TRACE("Entity_getComponent");

// 	// 1. MonoReflectionType에서 MonoType 얻기
// 	MonoType *monoType = mono_reflection_type_get_type(componentType);

// 	auto key = std::make_pair(entityID, monoType);
// 	auto it = s_EntityComponentInstanceMap.find(key);
// 	if (it != s_EntityComponentInstanceMap.end())
// 	{
// 		// 이미 있다면 반환
// 		return it->second;
// 	}

// 	// 2. 실제 엔티티가 이 컴포넌트를 가지고 있는지(즉 HasComponent) 체크
// 	Scene *scene = ScriptingEngine::getSceneContext();
// 	Entity entity = scene->getEntityByUUID(entityID);
// 	if (!s_EntityHasComponentFuncs.at(monoType)(entity))
// 	{
// 		return nullptr;
// 	}

// 	// 3. 새 C# 객체를 생성 (ScriptingEngine 내부에 "어떤 C# 클래스와 대응되는지" 등록이 필요)
// 	MonoObject *newComponentInstance = ScriptingEngine::createManagedComponentInstance(entity, monoType);

// 	// 4. 캐싱
// 	s_EntityComponentInstanceMap[key] = newComponentInstance;

// 	return newComponentInstance;
// }

static uint64_t Entity_findEntityByName(MonoString *name)
{
	char *nameCStr = mono_string_to_utf8(name);

	Scene *scene = ScriptingEngine::getSceneContext();
	Entity entity = scene->findEntityByName(nameCStr);
	mono_free(nameCStr);

	if (!entity)
	{
		return 0;
	}
	return entity.getUUID();
}

static MonoArray* Entity_findEntitiesByComponent(UUID entityID, MonoReflectionType *componentType)
{
	Scene *scene = ScriptingEngine::getSceneContext();
	Entity entity = scene->getEntityByUUID(entityID);

	MonoType *managedType = mono_reflection_type_get_type(componentType);
	auto it = s_EntityViewComponentsFuncs.find(managedType);
	if(it == s_EntityViewComponentsFuncs.end())
	{
		// 매핑에 해당 타입이 없다면, 빈 배열 반환
		MonoDomain* domain = mono_domain_get();
		MonoClass* ulongClass = mono_class_from_name(mono_get_corlib(), "System", "UInt64");
		return mono_array_new(domain, ulongClass, 0);
	}

	// 람다 호출: 해당 컴포넌트를 가진 모든 엔티티의 UUID 벡터를 얻습니다.
	std::vector<uint64_t> uuids = it->second(scene);

	// Mono 도메인과 System.UInt64 (C#의 ulong) 클래스를 얻습니다.
	MonoDomain* domain = mono_domain_get();
	MonoClass* ulongClass = mono_class_from_name(mono_get_corlib(), "System", "UInt64");

	// UUID 개수에 맞춰 MonoArray 생성 (System.UInt64[])
	MonoArray* array = mono_array_new(domain, ulongClass, static_cast<int>(uuids.size()));

	// 각 UUID 값을 배열에 채워 넣습니다.
	for (int i = 0; i < static_cast<int>(uuids.size()); ++i)
	{
		mono_array_set(array, uint64_t, i, uuids[i]);
	}

	return array;
}

static MonoObject *getScriptInstance(UUID entityID)
{
	return ScriptingEngine::getManagedInstance(entityID);
}

// Transform
static void TransformComponent_getPosition(UUID entityID, alglm::vec3 *outPosition)
{
	Scene *scene = ScriptingEngine::getSceneContext();
	Entity entity = scene->getEntityByUUID(entityID);

	*outPosition = entity.getComponent<TransformComponent>().m_Position;
}

static void TransformComponent_setPosition(UUID entityID, alglm::vec3 *position)
{
	Scene *scene = ScriptingEngine::getSceneContext();
	Entity entity = scene->getEntityByUUID(entityID);

	auto &tc = entity.getComponent<TransformComponent>();
	tc.m_Position = *position;
	tc.m_WorldTransform = tc.getTransform();
}

static void TransformComponent_getRotation(UUID entityID, alglm::vec3 *outRotation)
{
	Scene *scene = ScriptingEngine::getSceneContext();
	Entity entity = scene->getEntityByUUID(entityID);

	*outRotation = entity.getComponent<TransformComponent>().m_Rotation;
}

static void TransformComponent_setRotation(UUID entityID, alglm::vec3 *outRotation)
{
	Scene *scene = ScriptingEngine::getSceneContext();
	Entity entity = scene->getEntityByUUID(entityID);

	auto &tc = entity.getComponent<TransformComponent>();
	tc.m_Rotation = *outRotation;
	tc.m_WorldTransform = tc.getTransform();
}

static void Animator_getAnimations(UUID entityID, MonoArray* outAnimations)
{
	Scene *scene = ScriptingEngine::getSceneContext();
	Entity entity = scene->getEntityByUUID(entityID);

	SkeletalAnimatorComponent& sac = entity.getComponent<SkeletalAnimatorComponent>();
	auto sa = sac.sac;

	auto& animations = sa->getAnimations();

	MonoDomain* domain = mono_domain_get();
	MonoClass* stringClass = mono_class_from_name(mono_get_corlib(), "System", "String");
	int length = static_cast<int>((*animations).size());

	AL_CORE_INFO("ANIMATIONSIZE:{0}", length);

	outAnimations = mono_array_new(domain, stringClass, length);

	for (int i = 0; i < length; ++i)
	{
		MonoString* monoStr = mono_string_new(domain, (*animations)[i].getName().c_str());
		mono_array_set(outAnimations, MonoString*, i, monoStr);
	}
}

static void Animator_runAnimation(UUID entityID, int index)
{
	Scene *scene = ScriptingEngine::getSceneContext();
	Entity entity = scene->getEntityByUUID(entityID);

	SkeletalAnimatorComponent& sac = entity.getComponent<SkeletalAnimatorComponent>();
	auto sa = sac.sac;

	sa->start(index);
}

static void Animator_setRepeat(UUID entityID, bool repeat, int index)
{
	Scene *scene = ScriptingEngine::getSceneContext();
	Entity entity = scene->getEntityByUUID(entityID);

	SkeletalAnimatorComponent& sac = entity.getComponent<SkeletalAnimatorComponent>();
	auto sa = sac.sac;

	sa->setRepeat(repeat, index);
}

static void Animator_onInverse(UUID entityID)
{
	Scene *scene = ScriptingEngine::getSceneContext();
	Entity entity = scene->getEntityByUUID(entityID);

	SkeletalAnimatorComponent& sac = entity.getComponent<SkeletalAnimatorComponent>();
	auto sa = sac.sac;

	sa->setInverse(true, sa->getCurrentAnimationIndex());
}

static void Animator_offInverse(UUID entityID)
{
	Scene *scene = ScriptingEngine::getSceneContext();
	Entity entity = scene->getEntityByUUID(entityID);

	SkeletalAnimatorComponent& sac = entity.getComponent<SkeletalAnimatorComponent>();
	auto sa = sac.sac;

	sa->setInverse(false, sa->getCurrentAnimationIndex());
}

// Input
static bool Input_isKeyDown(KeyCode keycode)
{
	return Input::isKeyPressed(keycode);
}

static bool Input_isMouseLeftPressed()
{
	return Input::isMouseButtonPressed(0);
}

static bool Input_isMouseRightPressed()
{
	return Input::isMouseButtonPressed(1);
}

static void Input_getMousePos(alglm::vec2 *pos)
{
	*pos = Input::getMousePosition();
}

// Physics
static void RigidbodyComponent_addForce(UUID entityID, alglm::vec3 *force)
{
	Scene *scene = ScriptingEngine::getSceneContext();
	Entity entity = scene->getEntityByUUID(entityID);

	Rigidbody *body = (Rigidbody *)entity.getComponent<RigidbodyComponent>().body;
	body->registerForce(*force);
}

static void RigidbodyComponent_getPosition(UUID entityID, alglm::vec3 *outPosition)
{
	Scene *scene = ScriptingEngine::getSceneContext();
	Entity entity = scene->getEntityByUUID(entityID);

	auto* rb = (Rigidbody *)entity.getComponent<RigidbodyComponent>().body;
	*outPosition = rb->getPosition();
}

static void RigidbodyComponent_setPosition(UUID entityID, alglm::vec3 *position)
{
	Scene *scene = ScriptingEngine::getSceneContext();
	Entity entity = scene->getEntityByUUID(entityID);

	auto* rb = (Rigidbody *)entity.getComponent<RigidbodyComponent>().body;
	rb->setPositionNoFreeze(*position);
}

static void RigidbodyComponent_getRotation(UUID entityID, alglm::quat *outRotation)
{
	Scene *scene = ScriptingEngine::getSceneContext();
	Entity entity = scene->getEntityByUUID(entityID);

	auto* rb = (Rigidbody *)entity.getComponent<RigidbodyComponent>().body;
	*outRotation = rb->getOrientation();
}

static void RigidbodyComponent_setRotation(UUID entityID, alglm::quat *rotation)
{
	Scene *scene = ScriptingEngine::getSceneContext();
	Entity entity = scene->getEntityByUUID(entityID);

	auto* rb = (Rigidbody *)entity.getComponent<RigidbodyComponent>().body;
	rb->setOrientation(*rotation);
}

static int RigidbodyComponent_getTouchedNum(UUID entityID)
{
	Scene *scene = ScriptingEngine::getSceneContext();
	Entity entity = scene->getEntityByUUID(entityID);

	Rigidbody *rb = (Rigidbody *)entity.getComponent<RigidbodyComponent>().body;
	return rb->getTouchNum();
}

// ScriptComponent
static void ScriptComponent_getField(UUID entityID, MonoString* fieldName, bool* ret)
{
	Scene* scene = ScriptingEngine::getSceneContext();

	auto scriptInstance = ScriptingEngine::getEntityScriptInstance(entityID);
	*ret = scriptInstance->getFieldValue<bool>(utils::monoStringToString(fieldName).c_str());
}

static void ScriptComponent_setField(UUID entityID, MonoString* fieldName, bool value)
{
	Scene* scene = ScriptingEngine::getSceneContext();

	auto scriptInstance = ScriptingEngine::getEntityScriptInstance(entityID);
	scriptInstance->setFieldValue<bool>(utils::monoStringToString(fieldName).c_str(), value);
}

static void ScriptComponent_activate(UUID entityID)
{
	Scene* scene = ScriptingEngine::getSceneContext();
	Entity entity = scene->getEntityByUUID(entityID);

	auto& tc = entity.getComponent<TagComponent>();
	tc.m_isActive = true;
	tc.m_selfActive = true;
}

static void ScriptComponent_deactivate(UUID entityID)
{
	Scene* scene = ScriptingEngine::getSceneContext();
	Entity entity = scene->getEntityByUUID(entityID);

	auto& tc = entity.getComponent<TagComponent>();
	tc.m_isActive = false;
	tc.m_selfActive = false;
}

static bool BoxCollider_IsTriggered(UUID entityID, MonoString* targetEntityName)
{
	Scene* scene = ScriptingEngine::getSceneContext();
	Entity entity = scene->getEntityByUUID(entityID);

	auto& bc = entity.getComponent<BoxColliderComponent>();
	//*targetEntityName
	return bc.m_IsTrigger;
}

// Component 별로 HasComponentFunction handle 등록.
template <typename... Component> static void registerComponent()
{
	(
		[]() {
			std::string_view typeName = typeid(Component).name();
			size_t pos = typeName.find_last_of(':');
			std::string_view structName = typeName.substr(pos + 1);
			std::string managedTypename = fmt::format("ALEngine.{}", structName);

			Scene* scene = ScriptingEngine::getSceneContext();

			MonoType *managedType =
				mono_reflection_type_from_name(managedTypename.data(), ScriptingEngine::getCoreAssemblyImage());
			if (!managedType)
			{
				// AL_CORE_ERROR("Could not find component type");
				return;
			}
			s_EntityHasComponentFuncs[managedType] = [](Entity entity) { return entity.hasComponent<Component>(); };
			s_EntityViewComponentsFuncs[managedType] = [](Scene* scene) -> std::vector<uint64_t> { 
				std::vector<uint64_t> uuids;
				// entt view를 통해 MyComponent를 가진 엔티티들을 순회
				auto &view = scene->getAllEntitiesWith<IDComponent, Component>();
				for(auto &entity : view)
				{
					uuids.push_back(view.get<IDComponent>(entity).m_ID);
				}
				return uuids;
			};
		}(),
		...);
}

template <typename... Component> static void registerComponent(ComponentGroup<Component...>)
{
	registerComponent<Component...>();
}

void ScriptGlue::registerComponents()
{
	s_EntityHasComponentFuncs.clear();
	registerComponent(AllComponents{});
}

void ScriptGlue::registerFunctions()
{
	ADD_INTERNAL_CALL(nativeLog);
	ADD_INTERNAL_CALL(nativeLog_Vector);

	ADD_INTERNAL_CALL(Entity_hasComponent);
	// ADD_INTERNAL_CALL(Entity_getComponent);
	ADD_INTERNAL_CALL(Entity_findEntityByName);
	ADD_INTERNAL_CALL(Entity_findEntitiesByComponent);
	ADD_INTERNAL_CALL(getScriptInstance);

	ADD_INTERNAL_CALL(TransformComponent_getPosition);
	ADD_INTERNAL_CALL(TransformComponent_setPosition);
	ADD_INTERNAL_CALL(TransformComponent_getRotation);
	ADD_INTERNAL_CALL(TransformComponent_setRotation);

	ADD_INTERNAL_CALL(ScriptComponent_getField);
	ADD_INTERNAL_CALL(ScriptComponent_setField);
	ADD_INTERNAL_CALL(ScriptComponent_activate);
	ADD_INTERNAL_CALL(ScriptComponent_deactivate);

	ADD_INTERNAL_CALL(RigidbodyComponent_getPosition);
	ADD_INTERNAL_CALL(RigidbodyComponent_setPosition);
	ADD_INTERNAL_CALL(RigidbodyComponent_getRotation);
	ADD_INTERNAL_CALL(RigidbodyComponent_setRotation);
	ADD_INTERNAL_CALL(RigidbodyComponent_addForce);
	ADD_INTERNAL_CALL(RigidbodyComponent_getTouchedNum);

	ADD_INTERNAL_CALL(Animator_getAnimations);
	ADD_INTERNAL_CALL(Animator_runAnimation);
	ADD_INTERNAL_CALL(Animator_setRepeat);
	ADD_INTERNAL_CALL(Animator_onInverse);
	ADD_INTERNAL_CALL(Animator_offInverse);

	ADD_INTERNAL_CALL(Input_isKeyDown);
	ADD_INTERNAL_CALL(Input_isMouseLeftPressed);
	ADD_INTERNAL_CALL(Input_isMouseRightPressed);
	ADD_INTERNAL_CALL(Input_getMousePos);
}
} // namespace ale