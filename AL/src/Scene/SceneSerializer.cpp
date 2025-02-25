#include "alpch.h"

#include "Renderer/RenderingComponent.h"
#include "Renderer/SAComponent.h"

#include "Scene/Component.h"
#include "Scene/Entity.h"
#include "Scene/SceneSerializer.h"

#include "Scripting/ScriptingEngine.h"

#include <yaml-cpp/yaml.h>

namespace YAML
{
template <> struct convert<glm::vec2>
{
	static Node encode(const glm::vec2 &rhs)
	{
		Node node;
		node.push_back(rhs.x);
		node.push_back(rhs.y);
		node.SetStyle(EmitterStyle::Flow);
		return node;
	}

	static bool decode(const Node &node, glm::vec2 &rhs)
	{
		if (!node.IsSequence() || node.size() != 2)
			return false;

		rhs.x = node[0].as<float>();
		rhs.y = node[1].as<float>();
		return true;
	}
};

template <> struct convert<glm::vec3>
{
	static Node encode(const glm::vec3 &rhs)
	{
		Node node;
		node.push_back(rhs.x);
		node.push_back(rhs.y);
		node.push_back(rhs.z);
		node.SetStyle(EmitterStyle::Flow);
		return node;
	}

	static bool decode(const Node &node, glm::vec3 &rhs)
	{
		if (!node.IsSequence() || node.size() != 3)
			return false;

		rhs.x = node[0].as<float>();
		rhs.y = node[1].as<float>();
		rhs.z = node[2].as<float>();
		return true;
	}
};

template <> struct convert<glm::vec4>
{
	static Node encode(const glm::vec4 &rhs)
	{
		Node node;
		node.push_back(rhs.x);
		node.push_back(rhs.y);
		node.push_back(rhs.z);
		node.push_back(rhs.w);
		node.SetStyle(EmitterStyle::Flow);
		return node;
	}

	static bool decode(const Node &node, glm::vec4 &rhs)
	{
		if (!node.IsSequence() || node.size() != 4)
			return false;

		rhs.x = node[0].as<float>();
		rhs.y = node[1].as<float>();
		rhs.z = node[2].as<float>();
		rhs.w = node[3].as<float>();
		return true;
	}
};

template <> struct convert<ale::UUID>
{
	static Node encode(const ale::UUID &uuid)
	{
		Node node;
		node.push_back((uint64_t)uuid);
		return node;
	}

	static bool decode(const Node &node, ale::UUID &uuid)
	{
		uuid = node.as<uint64_t>();
		return true;
	}
};

template <> struct convert<ale::AnimationState>
{
	static Node encode(const ale::AnimationState &state)
	{
		Node node;

		node["stateName"] = state.stateName;
		node["animationName"] = state.animationName;
		node["looping"] = state.looping;
		node["interruptible"] = state.interruptible;
		node["defaultBlendTime"] = state.defaultBlendTime;
	
		return node;
	};

	static bool decode(const Node &node, ale::AnimationState &state)
	{
		if (!node.IsMap())
			return false;

		state.stateName = node["stateName"].as<std::string>();
		state.animationName = node["animationName"].as<std::string>();
		state.looping = node["looping"].as<bool>();
		state.interruptible = node["interruptible"].as<bool>();
		state.defaultBlendTime = node["defaultBlendTime"].as<float>();

		return true;
	};
};

template <> struct convert<ale::AnimationStateTransition>
{
	static Node encode(const ale::AnimationStateTransition &transition)
	{
		Node node;

		node["fromState"] = transition.fromState;
		node["toState"] = transition.toState;
		node["conditionName"] = transition.conditionName;
		node["blendTime"] = transition.blendTime;
		node["invertCondition"] = transition.invertCondition;
	
		return node;
	};

	static bool decode(const Node &node, ale::AnimationStateTransition &transition)
	{
		if (!node.IsMap())
			return false;

		transition.fromState = node["fromState"].as<std::string>();
		transition.toState = node["toState"].as<std::string>();
		transition.conditionName = node["conditionName"].as<std::string>();
		transition.blendTime = node["blendTime"].as<float>();
		transition.invertCondition = node["invertCondition"].as<bool>();

		return true;
	};
};

} // namespace YAML

namespace ale
{
#define WRITE_SCRIPT_FIELD(FieldType, Type)                                                                            \
	case EScriptFieldType::FieldType:                                                                                  \
		out << scriptField.getValue<Type>();                                                                           \
		break

#define READ_SCRIPT_FIELD(FieldType, Type)                                                                             \
	case EScriptFieldType::FieldType: {                                                                                \
		Type data = scriptField["Data"].as<Type>();                                                                    \
		fieldInstance.setValue(data);                                                                                  \
		break;                                                                                                         \
	}

YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec2 &v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
	return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec3 &v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
	return out;
}

YAML::Emitter &operator<<(YAML::Emitter &out, const glm::vec4 &v)
{
	out << YAML::Flow;
	out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
	return out;
}

SceneSerializer::SceneSerializer(const std::shared_ptr<Scene> &scene) : m_Scene(scene)
{
}

static void serializeEntity(YAML::Emitter &out, Entity entity, Scene *scene)
{
	out << YAML::BeginMap;
	out << YAML::Key << "Entity" << YAML::Value << entity.getUUID();

	// KEY, KEY - VALUE
	if (entity.hasComponent<TagComponent>())
	{
		out << YAML::Key << "TagComponent";
		out << YAML::BeginMap;
		auto &tag = entity.getComponent<TagComponent>().m_Tag;
		out << YAML::Key << "Tag" << YAML::Value << tag;
		out << YAML::EndMap; // Tag
	}

	// TransformComponent
	if (entity.hasComponent<TransformComponent>())
	{
		out << YAML::Key << "TransformComponent";
		out << YAML::BeginMap;
		auto &tf = entity.getComponent<TransformComponent>();
		out << YAML::Key << "Position" << YAML::Value << tf.m_Position;
		out << YAML::Key << "Rotation" << YAML::Value << tf.m_Rotation;
		out << YAML::Key << "Scale" << YAML::Value << tf.m_Scale;
		out << YAML::EndMap; // Transform
	}

	// RelationshipComponent
	if (entity.hasComponent<RelationshipComponent>())
	{
		out << YAML::Key << "RelationshipComponent";
		out << YAML::BeginMap;

		auto &rc = entity.getComponent<RelationshipComponent>();

		// 1) Parent
		// 부모가 없는 경우 entt::null일 텐데, 이를 0(또는 어떤 특별한 값)으로 처리
		if (rc.parent == entt::null)
		{
			out << YAML::Key << "Parent" << YAML::Value << 0;
		}
		else
		{
			Entity parentEntity{rc.parent, scene};
			out << YAML::Key << "Parent" << YAML::Value << parentEntity.getUUID();
		}

		// 2) Children
		out << YAML::Key << "Children" << YAML::Value << YAML::BeginSeq;
		for (auto child : rc.children)
		{
			Entity childEntity{child, scene};
			out << childEntity.getUUID(); // 자식 UUID
		}
		out << YAML::EndSeq;

		out << YAML::EndMap; // RelationshipComponent
	}
	// CameraComponent
	if (entity.hasComponent<CameraComponent>())
	{
		out << YAML::Key << "CameraComponent";
		out << YAML::BeginMap;

		auto &cc = entity.getComponent<CameraComponent>();
		auto &camera = cc.m_Camera;
		out << YAML::Key << "Camera";
		out << YAML::BeginMap;
		out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.getFov();
		out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.getNear();
		out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.getFar();
		out << YAML::EndMap;

		out << YAML::Key << "Primary" << YAML::Value << cc.m_Primary;
		out << YAML::Key << "FixedAspectRatio" << YAML::Value << cc.m_FixedAspectRatio;

		out << YAML::EndMap; // Camera
	}
	// MeshRendererComponent
	if (entity.hasComponent<MeshRendererComponent>())
	{
		out << YAML::Key << "MeshRendererComponent";
		out << YAML::BeginMap;

		auto &mc = entity.getComponent<MeshRendererComponent>();
		out << YAML::Key << "MeshType" << YAML::Value << mc.type;

		if (!mc.path.empty())
		{
			out << YAML::Key << "Path" << YAML::Value << mc.path;
		}
		out << YAML::Key << "MatPath" << YAML::Value << mc.matPath;
		out << YAML::Key << "IsMatChanged" << YAML::Value << mc.isMatChanged;

		out << YAML::EndMap;
	}
	// ScriptComponent
	if (entity.hasComponent<ScriptComponent>())
	{
		auto &scriptComponent = entity.getComponent<ScriptComponent>();

		out << YAML::Key << "ScriptComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "ClassName" << YAML::Value << scriptComponent.m_ClassName;

		std::shared_ptr<ScriptClass> entityClass = ScriptingEngine::getEntityClass(scriptComponent.m_ClassName);
		const auto &fields = entityClass->getFields();

		if (fields.size() > 0)
		{
			out << YAML::Key << "ScriptFields" << YAML::Value;
			auto &entityFields = ScriptingEngine::getScriptFieldMap(entity);
			out << YAML::BeginSeq;

			for (const auto &[name, field] : fields)
			{
				if (entityFields.find(name) == entityFields.end())
					continue;
				out << YAML::BeginMap;
				out << YAML::Key << "Name" << YAML::Value << name;
				out << YAML::Key << "Type" << YAML::Value << utils::scriptFieldTypeToString(field.m_Type);

				out << YAML::Key << "Data" << YAML::Value;
				ScriptFieldInstance &scriptField = entityFields.at(name);

				switch (field.m_Type)
				{
					WRITE_SCRIPT_FIELD(FLOAT, float);
					WRITE_SCRIPT_FIELD(DOUBLE, double);
					WRITE_SCRIPT_FIELD(BOOL, bool);
					WRITE_SCRIPT_FIELD(CHAR, char);
					WRITE_SCRIPT_FIELD(BYTE, int8_t);
					WRITE_SCRIPT_FIELD(SHORT, int16_t);
					WRITE_SCRIPT_FIELD(INT, int32_t);
					WRITE_SCRIPT_FIELD(LONG, int64_t);
					WRITE_SCRIPT_FIELD(UBYTE, uint8_t);
					WRITE_SCRIPT_FIELD(USHORT, uint16_t);
					WRITE_SCRIPT_FIELD(UINT, uint32_t);
					WRITE_SCRIPT_FIELD(ULONG, uint64_t);
					WRITE_SCRIPT_FIELD(VECTOR2, glm::vec2);
					WRITE_SCRIPT_FIELD(VECTOR3, glm::vec3);
					WRITE_SCRIPT_FIELD(VECTOR4, glm::vec4);
					WRITE_SCRIPT_FIELD(ENTITY, UUID);
				}
				out << YAML::EndMap;
			}
			out << YAML::EndSeq;
		}

		out << YAML::EndMap; // End ScriptComponent
	}

	// LightComponent
	if (entity.hasComponent<LightComponent>())
	{
		out << YAML::Key << "LightComponent";
		out << YAML::BeginMap;

		auto &lc = entity.getComponent<LightComponent>();
		Light &light = *lc.m_Light.get();
		out << YAML::Key << "Type" << YAML::Value << light.type;
		out << YAML::Key << "ShadowMap" << YAML::Value << light.onShadowMap;
		out << YAML::Key << "Position" << YAML::Value << light.position;
		out << YAML::Key << "Direction" << YAML::Value << light.direction;
		out << YAML::Key << "Color" << YAML::Value << light.color;
		out << YAML::Key << "Intensity" << YAML::Value << light.intensity;
		out << YAML::Key << "InnerCutoff" << YAML::Value << light.innerCutoff;
		out << YAML::Key << "OuterCutoff" << YAML::Value << light.outerCutoff;
		out << YAML::EndMap;
	}
	// RigidbodyComponent
	if (entity.hasComponent<RigidbodyComponent>())
	{
		out << YAML::Key << "RigidbodyComponent";
		out << YAML::BeginMap;
		auto &rb = entity.getComponent<RigidbodyComponent>();
		out << YAML::Key << "Mass" << YAML::Value << rb.m_Mass;
		out << YAML::Key << "Drag" << YAML::Value << rb.m_Damping;
		out << YAML::Key << "AngularDrag" << YAML::Value << rb.m_AngularDamping;
		out << YAML::Key << "UseGravity" << YAML::Value << rb.m_UseGravity;
		out << YAML::EndMap; // Rigidbody
	}
	// BoxColliderComponent
	if (entity.hasComponent<BoxColliderComponent>())
	{
		out << YAML::Key << "BoxColliderComponent";
		out << YAML::BeginMap;
		auto &bc = entity.getComponent<BoxColliderComponent>();
		out << YAML::Key << "Center" << YAML::Value << bc.m_Center;
		out << YAML::Key << "Size" << YAML::Value << bc.m_Size;
		out << YAML::Key << "IsTrigger" << YAML::Value << bc.m_IsTrigger;
		out << YAML::EndMap; // BoxCollider
	}
	// SphereColliderComponent
	if (entity.hasComponent<SphereColliderComponent>())
	{
		out << YAML::Key << "SphereColliderComponent";
		out << YAML::BeginMap;
		auto &sc = entity.getComponent<SphereColliderComponent>();
		out << YAML::Key << "Center" << YAML::Value << sc.m_Center;
		out << YAML::Key << "Radius" << YAML::Value << sc.m_Radius;
		out << YAML::Key << "IsTrigger" << YAML::Value << sc.m_IsTrigger;
		out << YAML::EndMap; // SphereCollider
	}
	// CapsuleColliderComponent
	if (entity.hasComponent<CapsuleColliderComponent>())
	{
		out << YAML::Key << "CapsuleColliderComponent";
		out << YAML::BeginMap;
		auto &cc = entity.getComponent<CapsuleColliderComponent>();
		out << YAML::Key << "Center" << YAML::Value << cc.m_Center;
		out << YAML::Key << "Radius" << YAML::Value << cc.m_Radius;
		out << YAML::Key << "Height" << YAML::Value << cc.m_Height;
		out << YAML::Key << "IsTrigger" << YAML::Value << cc.m_IsTrigger;
		out << YAML::EndMap; // CapusuleCollider
	}
	// CylinderColliderComponent
	if (entity.hasComponent<CylinderColliderComponent>())
	{
		out << YAML::Key << "CylinderColliderComponent";
		out << YAML::BeginMap;
		auto &cc = entity.getComponent<CylinderColliderComponent>();
		out << YAML::Key << "Center" << YAML::Value << cc.m_Center;
		out << YAML::Key << "Radius" << YAML::Value << cc.m_Radius;
		out << YAML::Key << "Height" << YAML::Value << cc.m_Height;
		out << YAML::Key << "IsTrigger" << YAML::Value << cc.m_IsTrigger;
		out << YAML::EndMap; // CylinderCollider
	}
	// SKeletalAnimatorComponent / SAComponent animation
	if (entity.hasComponent<SkeletalAnimatorComponent>())
	{
		out << YAML::Key << "SkeletalAnimatorComponent";
		out << YAML::BeginMap;
		auto& sa = entity.getComponent<SkeletalAnimatorComponent>();

		out << YAML::Key << "SpeedFactor" << YAML::Value << sa.m_SpeedFactor;
		out << YAML::Key << "Repeats" << YAML::Value << sa.sac->getRepeatAll();

		auto& stateManager = sa.sac->getStateManager();
		out << YAML::Key << "AnimationStateManager";
			out << YAML::BeginMap;
			auto& states = stateManager->getStates();
			auto& transitions = stateManager->getTransitions();

			YAML::Node statesNode(YAML::NodeType::Map), transitionsNode(YAML::NodeType::Sequence);
			for (const auto& pair : states)
				statesNode[pair.first] = pair.second;

			for (const auto& transition : transitions) 
				transitionsNode.push_back(transition);

			out << YAML::Key << "AnimationStates" << YAML::Value << statesNode;
			out << YAML::Key << "AnimationTransitions" << YAML::Value << transitionsNode;
			out << YAML::EndMap; // AnimationStateManager;
		out << YAML::EndMap; // SkeletalAnimatorComponent;
	}

	out << YAML::EndMap; // End Serialize Entity
}

void SceneSerializer::serialize(const std::string &filepath)
{
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "Scene" << YAML::Value << "Untitled";
	out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
	m_Scene->m_Registry.view<entt::entity>().each([&](auto entityID) {
		Entity entity{entityID, m_Scene.get()};
		if (!entity)
		{
			return;
		}
		serializeEntity(out, entity, m_Scene.get());
	});

	out << YAML::EndSeq;
	out << YAML::EndMap;
	std::ofstream fout(filepath);
	fout << out.c_str();
}

void SceneSerializer::serializeRuntime(const std::string &filepath)
{
}

bool SceneSerializer::deserialize(const std::string &filepath)
{
	YAML::Node data;

	try
	{
		data = YAML::LoadFile(filepath);
	}
	catch (const std::exception &e)
	{
		AL_CORE_ERROR("Failed to load .ale file '{0}'\n    {1}", filepath, e.what());
		return false;
	}

	// Scene
	if (!data["Scene"])
	{
		return false;
	}

	// Entities
	auto entities = data["Entities"];
	if (entities)
	{
		for (auto entity : entities)
		{
			// IDComponent
			uint64_t uuid = entity["Entity"].as<uint64_t>();

			// TagComponent
			std::string name;
			auto tagComponent = entity["TagComponent"];
			if (tagComponent)
			{
				name = tagComponent["Tag"].as<std::string>();
			}

			AL_CORE_TRACE("Entity deserialized as ID: {0}, Tag: {1}", uuid, name);

			Entity deserializedEntity = m_Scene->createEntityWithUUID(uuid, name);

			// TransformComponent
			auto tfComponent = entity["TransformComponent"];
			if (tfComponent)
			{
				auto &tf = deserializedEntity.getComponent<TransformComponent>();
				tf.m_Position = tfComponent["Position"].as<glm::vec3>();
				tf.m_Rotation = tfComponent["Rotation"].as<glm::vec3>();
				tf.m_Scale = tfComponent["Scale"].as<glm::vec3>();
				tf.m_WorldTransform = tf.getTransform();
			}

			// RelationshipComponent
			auto relationshipComponent = entity["RelationshipComponent"];
			if (relationshipComponent)
			{
				uint64_t parentUUID = relationshipComponent["Parent"].as<uint64_t>();
				auto childrenNode = relationshipComponent["Children"];
				std::vector<uint64_t> childList;
				if (childrenNode)
				{
					for (auto childUUIDNode : childrenNode)
					{
						childList.push_back(childUUIDNode.as<uint64_t>());
					}
				}
				// 임시 보관
				RelationshipData temp{};
				temp.entityUUID = uuid;
				temp.parentUUID = parentUUID;
				temp.childrenUUIDs = childList;
				relationshipMap[uuid] = temp;
			}

			// CameraComponent
			auto cameraComponent = entity["CameraComponent"];
			if (cameraComponent)
			{
				auto &cc = deserializedEntity.addComponent<CameraComponent>();
				auto &cameraProps = cameraComponent["Camera"];

				cc.m_Camera.setFov(cameraProps["PerspectiveFOV"].as<float>());
				cc.m_Camera.setNear(cameraProps["PerspectiveNear"].as<float>());
				cc.m_Camera.setFar(cameraProps["PerspectiveFar"].as<float>());

				cc.m_Primary = cameraComponent["Primary"].as<bool>();
				cc.m_FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
			}

			// MeshRendererComponent
			auto meshComponent = entity["MeshRendererComponent"];
			if (meshComponent)
			{
				auto &mc = deserializedEntity.addComponent<MeshRendererComponent>();
				// type에 따라 Primitive Mesh 생성
				mc.type = meshComponent["MeshType"].as<uint32_t>();
				mc.isMatChanged = meshComponent["IsMatChanged"].as<bool>();
				std::shared_ptr<Model> model;
				switch (mc.type)
				{
				case 1:
					model = m_Scene->getBoxModel();
					break;
				case 2:
					model = m_Scene->getSphereModel();
					break;
				case 3:
					model = m_Scene->getPlaneModel();
					break;
				case 4:
					model = m_Scene->getGroundModel();
					break;
				case 5:
					model = m_Scene->getCapsuleModel();
					break;
				case 6:
					model = m_Scene->getCylinderModel();
					break;
				case 7:
					mc.path = meshComponent["Path"].as<std::string>();
					model = Model::createModel(mc.path, m_Scene->getDefaultMaterial());
					break;
				// 그 외의 이상한 값은 box로 임의로 처리
				default:
					model = m_Scene->getBoxModel();
					break;
				}
				mc.matPath = meshComponent["MatPath"].as<std::string>();

				mc.m_RenderingComponent = RenderingComponent::createRenderingComponent(model);

				mc.cullSphere = mc.m_RenderingComponent->getCullSphere();
				// cullTree에 추가 sphere
				m_Scene->insertEntityInCullTree(deserializedEntity);

				if (mc.isMatChanged)
				{
					mc.m_RenderingComponent->updateMaterial(
						Model::createModel(mc.matPath, m_Scene->getDefaultMaterial()));
				}
			}

			auto lightComponent = entity["LightComponent"];
			if (lightComponent)
			{
				Light &light = *deserializedEntity.addComponent<LightComponent>().m_Light.get();
				light.type = lightComponent["Type"].as<uint32_t>();
				light.onShadowMap = lightComponent["ShadowMap"].as<uint32_t>();
				light.position = lightComponent["Position"].as<glm::vec3>();
				light.direction = lightComponent["Direction"].as<glm::vec3>();
				light.color = lightComponent["Color"].as<glm::vec3>();
				light.intensity = lightComponent["Intensity"].as<float>();
				light.innerCutoff = lightComponent["InnerCutoff"].as<float>();
				light.outerCutoff = lightComponent["OuterCutoff"].as<float>();
			}

			// ScriptComponent
			auto scriptComponent = entity["ScriptComponent"];
			if (scriptComponent)
			{
				auto &sc = deserializedEntity.addComponent<ScriptComponent>();
				sc.m_ClassName = scriptComponent["ClassName"].as<std::string>();

				auto scriptFields = scriptComponent["ScriptFields"];
				if (scriptFields)
				{
					std::shared_ptr<ScriptClass> entityClass = ScriptingEngine::getEntityClass(sc.m_ClassName);
					if (entityClass)
					{
						const auto &fields = entityClass->getFields();
						auto &entityFields = ScriptingEngine::getScriptFieldMap(deserializedEntity);

						for (auto scriptField : scriptFields)
						{
							std::string name = scriptField["Name"].as<std::string>();
							std::string typeString = scriptField["Type"].as<std::string>();
							EScriptFieldType type = utils::scriptFieldTypeFromString(typeString);

							ScriptFieldInstance &fieldInstance = entityFields[name];
							if (fields.find(name) == fields.end())
								continue;

							fieldInstance.m_Field = fields.at(name);

							switch (type)
							{
								READ_SCRIPT_FIELD(FLOAT, float);
								READ_SCRIPT_FIELD(DOUBLE, double);
								READ_SCRIPT_FIELD(BOOL, bool);
								READ_SCRIPT_FIELD(CHAR, char);
								READ_SCRIPT_FIELD(BYTE, int8_t);
								READ_SCRIPT_FIELD(SHORT, int16_t);
								READ_SCRIPT_FIELD(INT, int32_t);
								READ_SCRIPT_FIELD(LONG, int64_t);
								READ_SCRIPT_FIELD(UBYTE, uint8_t);
								READ_SCRIPT_FIELD(USHORT, uint16_t);
								READ_SCRIPT_FIELD(UINT, uint32_t);
								READ_SCRIPT_FIELD(ULONG, uint64_t);
								READ_SCRIPT_FIELD(VECTOR2, glm::vec2);
								READ_SCRIPT_FIELD(VECTOR3, glm::vec3);
								READ_SCRIPT_FIELD(VECTOR4, glm::vec4);
								READ_SCRIPT_FIELD(ENTITY, UUID);
							}
						}
					}
				}
			}

			// SkeletalAnimatorComponent / SAComponent animation
			auto skeletalAnimatorComponent = entity["SkeletalAnimatorComponent"];
			if (skeletalAnimatorComponent)
			{
				auto& sa = deserializedEntity.addComponent<SkeletalAnimatorComponent>();
				sa.m_SpeedFactor = skeletalAnimatorComponent["SpeedFactor"].as<float>();
				sa.m_Repeats	 = skeletalAnimatorComponent["Repeats"].as<std::vector<bool>>();
			
				for (size_t repeatIndex = 0; repeatIndex < sa.m_Repeats.size(); ++repeatIndex)
				{
					sa.sac->setRepeat(sa.m_Repeats[repeatIndex], repeatIndex);
				}

				auto animationStateManager = skeletalAnimatorComponent["AnimationStateManager"];
				if (animationStateManager)
				{
					// AnimationStates 처리
					auto statesNode = animationStateManager["AnimationStates"];
					if (statesNode && statesNode.size() > 0)
					{
						auto& stateManager = sa.sac->getStateManager();
						std::unordered_map<std::string, AnimationState> states;
						for (auto it = statesNode.begin(); it != statesNode.end(); ++it)
						{
							std::string key = it->first.as<std::string>();
							AnimationState state = it->second.as<AnimationState>();
							states[key] = state;
						}
						stateManager->setStates(std::move(states));
					}
			
					// AnimationTransitions 처리
					auto transitionsNode = animationStateManager["AnimationTransitions"];
					if (transitionsNode && transitionsNode.size() > 0)
					{
						auto& stateManager = sa.sac->getStateManager();
						std::vector<AnimationStateTransition> transitions =
							transitionsNode.as<std::vector<AnimationStateTransition>>();
						stateManager->setTransitions(std::move(transitions));
					}
				}
			}

			// RigidbodyComponent
			auto rbComponent = entity["RigidbodyComponent"];
			if (rbComponent)
			{
				auto &rb = deserializedEntity.addComponent<RigidbodyComponent>();
				rb.m_Mass = rbComponent["Mass"].as<float>();
				rb.m_Damping = rbComponent["Drag"].as<float>();
				rb.m_AngularDamping = rbComponent["AngularDrag"].as<float>();
				rb.m_UseGravity = rbComponent["UseGravity"].as<bool>();
			}
			// BoxColliderComponent
			auto bcComponent = entity["BoxColliderComponent"];
			if (bcComponent)
			{
				auto &bc = deserializedEntity.addComponent<BoxColliderComponent>();
				bc.m_Center = bcComponent["Center"].as<glm::vec3>();
				bc.m_Size = bcComponent["Size"].as<glm::vec3>();
				bc.m_IsTrigger = bcComponent["IsTrigger"].as<bool>();
			}
			// SphereColliderComponent
			auto scComponent = entity["SphereColliderComponent"];
			if (scComponent)
			{
				auto &sc = deserializedEntity.addComponent<SphereColliderComponent>();
				sc.m_Center = scComponent["Center"].as<glm::vec3>();
				sc.m_Radius = scComponent["Radius"].as<float>();
				sc.m_IsTrigger = scComponent["IsTrigger"].as<bool>();
			}
			// CapsuleColliderComponent
			auto capcComponent = entity["CapsuleColliderComponent"];
			if (capcComponent)
			{
				auto &cc = deserializedEntity.addComponent<CapsuleColliderComponent>();
				cc.m_Center = capcComponent["Center"].as<glm::vec3>();
				cc.m_Radius = capcComponent["Radius"].as<float>();
				cc.m_Height = capcComponent["Height"].as<float>();
				cc.m_IsTrigger = capcComponent["IsTrigger"].as<bool>();
			}
			// CylinderColliderComponent
			auto cycComponent = entity["CylinderColliderComponent"];
			if (cycComponent)
			{
				auto &cc = deserializedEntity.addComponent<CylinderColliderComponent>();
				cc.m_Center = cycComponent["Center"].as<glm::vec3>();
				cc.m_Radius = cycComponent["Radius"].as<float>();
				cc.m_Height = cycComponent["Height"].as<float>();
				cc.m_IsTrigger = cycComponent["IsTrigger"].as<bool>();
			}
		}
		// // 2차 pass: relationshipTempMap을 이용해 실제 엔티티 연결
		for (auto &[uuid, tempData] : relationshipMap)
		{
			Entity e = m_Scene->getEntityByUUID(uuid);
			auto &rc = e.getComponent<RelationshipComponent>();

			// 부모 연결
			if (tempData.parentUUID != 0) // 0이면 부모 없음
			{
				Entity parentEntity = m_Scene->getEntityByUUID(tempData.parentUUID);
				rc.parent = parentEntity;
			}
			else
			{
				rc.parent = entt::null;
			}

			// 자식 연결
			for (auto childUUID : tempData.childrenUUIDs)
			{
				Entity childEntity = m_Scene->getEntityByUUID(childUUID);
				if (childEntity)
				{
					rc.children.push_back((entt::entity)childEntity);
				}
			}
		}
	}
	return true;
}

bool SceneSerializer::deserializeRuntime(const std::string &filepath)
{
	return true;
}

} // namespace ale