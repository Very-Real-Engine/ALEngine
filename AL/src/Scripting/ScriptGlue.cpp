#include "alpch.h"

#include "Scripting/ScriptingEngine.h"
#include "Scripting/ScriptingGlue.h"

#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include "Core/UUID.h"

#include "Scene/Entity.h"
#include "Scene/Scene.h"

#include "mono/metadata/object.h"
#include "mono/metadata/reflection.h"

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

// Input
static bool Input_isKeyDown(KeyCode keycode)
{
	return Input::isKeyPressed(keycode);
}

// Physics
static void RigidbodyComponent_addForce(UUID entityID, alglm::vec3 *force)
{
	Scene *scene = ScriptingEngine::getSceneContext();
	Entity entity = scene->getEntityByUUID(entityID);

	Rigidbody *body = (Rigidbody *)entity.getComponent<RigidbodyComponent>().body;
	body->registerForce(*force);
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

			MonoType *managedType =
				mono_reflection_type_from_name(managedTypename.data(), ScriptingEngine::getCoreAssemblyImage());
			if (!managedType)
			{
				// AL_CORE_ERROR("Could not find component type");
				return;
			}
			s_EntityHasComponentFuncs[managedType] = [](Entity entity) { return entity.hasComponent<Component>(); };
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
	ADD_INTERNAL_CALL(getScriptInstance);

	ADD_INTERNAL_CALL(TransformComponent_getPosition);
	ADD_INTERNAL_CALL(TransformComponent_setPosition);
	ADD_INTERNAL_CALL(TransformComponent_getRotation);
	ADD_INTERNAL_CALL(TransformComponent_setRotation);

	ADD_INTERNAL_CALL(RigidbodyComponent_addForce);

	ADD_INTERNAL_CALL(Input_isKeyDown);
}
} // namespace ale