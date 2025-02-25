#pragma once

#include "Core/UUID.h"

#include "Renderer/Model.h"
#include "Renderer/Texture.h"

#include "Scene/SceneCamera.h"

#include "Scene/entt.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

namespace ale
{
/**
 * @struct IDComponent
 * @brief 개체(Entity)의 고유 ID를 저장하는 컴포넌트.
 */
struct IDComponent
{
	UUID m_ID;

	IDComponent() = default;
	IDComponent(const IDComponent &) = default;
};

/**
 * @struct TagComponent
 * @brief 개체(Entity)의 태그(이름) 및 활성 상태를 저장하는 컴포넌트.
 */
struct TagComponent
{
	std::string m_Tag;
	bool m_isActive = true;	  // 에디터 상 활성 여부
	bool m_selfActive = true; // 부모와 관계없이 자신의 활성 여부

	TagComponent() = default;
	TagComponent(const TagComponent &) = default;
	TagComponent(const std::string &tag) : m_Tag(tag)
	{
	}
};

/**
 * @struct TransformComponent
 * @brief 개체의 위치, 회전, 크기 정보를 저장하는 컴포넌트.
 */
struct TransformComponent
{
	// 속성
	glm::vec3 m_Position = {0.0f, 0.0f, 0.0f};
	glm::vec3 m_Rotation = {0.0f, 0.0f, 0.0f};
	glm::vec3 m_Scale = {1.0f, 1.0f, 1.0f};
	glm::vec3 m_LastPosition = {0.0f, 0.0f, 0.0f};

	bool m_isMoved = false;

	glm::mat4 m_WorldTransform = glm::mat4(1.0f);

	// 생성자
	TransformComponent() = default;
	TransformComponent(const TransformComponent &) = default;
	TransformComponent(const glm::vec3 &position) : m_Position(position), m_LastPosition(position)
	{
	}

	/**
	 * @brief 현재 변환 정보를 행렬 형태로 반환합니다.
	 * @return glm::mat4 변환 행렬.
	 */
	glm::mat4 getTransform() const
	{
		glm::mat4 rotation = glm::toMat4(glm::quat(m_Rotation));

		return glm::translate(glm::mat4(1.0f), m_Position) * rotation * glm::scale(glm::mat4(1.0f), m_Scale);
	}

	/**
	 * @brief 크기 중 가장 큰 값을 반환합니다.
	 * @return float 최대 크기 값.
	 */
	float getMaxScale()
	{
		return std::max(m_Scale.x, std::max(m_Scale.y, m_Scale.z));
	}
};

/**
 * @struct RelationshipComponent
 * @brief 개체 간의 부모-자식 관계를 정의하는 컴포넌트.
 */
struct RelationshipComponent
{
	entt::entity parent = entt::null;
	std::vector<entt::entity> children;

	RelationshipComponent() = default;
	RelationshipComponent(const RelationshipComponent &) = default;
};

// RENDERER
// Mesh Renderer - Cube, Sphere, Cylinder, Capsule

class RenderingComponent;

/**
 * @struct MeshRendererComponent
 * @brief 메시 렌더링을 위한 컴포넌트.
 */
struct MeshRendererComponent
{
	std::shared_ptr<RenderingComponent> m_RenderingComponent;
	uint32_t type;
	std::string path = "";
	std::string matPath = "";
	bool isMatChanged = false;

	// Culling
	int32_t nodeId = NULL_NODE;
	CullSphere cullSphere;
	ECullState cullState;

	MeshRendererComponent() = default;
	MeshRendererComponent(const MeshRendererComponent &) = default;
};

/**
 * @struct LightComponent
 * @brief 빛 설정을 위한 컴포넌트.
 */
struct LightComponent
{
	// Color
	std::shared_ptr<Light> m_Light;

	// Type - Directional, Spot, Point
	LightComponent() = default;
	LightComponent(const LightComponent &) = default;
};

/**
 * @struct CameraComponent
 * @brief 카메라 설정을 위한 컴포넌트.
 */
struct CameraComponent
{
	SceneCamera m_Camera;
	bool m_Primary = true;
	bool m_FixedAspectRatio = false;
	std::string skyboxPath = "";

	CameraComponent() = default;
	CameraComponent(const CameraComponent &) = default;
};

/**
 * @struct RigidbodyComponent
 * @brief 물리 엔진의 리지드바디(Rigidbody) 설정을 위한 컴포넌트.
 */
struct RigidbodyComponent
{
	// FLAG
	glm::vec3 m_FreezePos = {1, 1, 1};
	glm::vec3 m_FreezeRot = {1, 1, 1};

	void *body = nullptr;

	/**
	 * @enum EBodyType
	 * @brief 리지드바디의 유형을 정의하는 열거형.
	 */
	enum class EBodyType
	{
		Static = 0,
		Dynamic,
		Kinematic
	};
	EBodyType m_Type = EBodyType::Dynamic;

	float m_Mass = 1.0f;
	float m_Damping = 0.001f;
	float m_AngularDamping = 0.001f;
	bool m_UseGravity = true;

	RigidbodyComponent() = default;
	RigidbodyComponent(const RigidbodyComponent &) = default;
};

/**
 * @struct BoxColliderComponent
 * @brief 박스 콜라이더 설정을 위한 컴포넌트.
 */
struct BoxColliderComponent
{
	glm::vec3 m_Center = {0.0f, 0.0f, 0.0f};
	glm::vec3 m_Size = {1.0f, 1.0f, 1.0f};
	bool m_IsTrigger = false;

	BoxColliderComponent() = default;
	BoxColliderComponent(const BoxColliderComponent &) = default;
};

/**
 * @struct SphereColliderComponent
 * @brief 구 콜라이더 설정을 위한 컴포넌트.
 */
struct SphereColliderComponent
{
	glm::vec3 m_Center;
	float m_Radius;
	bool m_IsTrigger = false;

	SphereColliderComponent() = default;
	SphereColliderComponent(const SphereColliderComponent &) = default;
};

/**
 * @struct CapsuleColliderComponent
 * @brief 캡슐 콜라이더 설정을 위한 컴포넌트.
 */
struct CapsuleColliderComponent
{
	glm::vec3 m_Center;
	float m_Radius;
	float m_Height;

	bool m_IsTrigger = false;

	CapsuleColliderComponent() = default;
	CapsuleColliderComponent(const CapsuleColliderComponent &) = default;
};

/**
 * @struct CylinderColliderComponent
 * @brief 원기둥 콜라이더 설정을 위한 컴포넌트.
 */
struct CylinderColliderComponent
{
	glm::vec3 m_Center;
	float m_Radius;
	float m_Height;

	bool m_IsTrigger = false;

	CylinderColliderComponent() = default;
	CylinderColliderComponent(const CylinderColliderComponent &) = default;
};

/**
 * @struct ScriptComponent
 * @brief 스크립트를 지정하는 컴포넌트.
 */
struct ScriptComponent
{
	std::string m_ClassName;

	ScriptComponent() = default;
	ScriptComponent(const ScriptComponent &) = default;
};

class ScriptableEntity;

/**
 * @struct NativeScriptComponent
 * @brief 네이티브 스크립트 컴포넌트.
 */
struct NativeScriptComponent
{
	ScriptableEntity *instance = nullptr;

	ScriptableEntity *(*instantiateScript)();
	void (*destroyScript)(NativeScriptComponent *);

	/**
	 * @brief 특정 스크립트 클래스를 바인딩합니다.
	 * @tparam T 바인딩할 스크립트 클래스.
	 */
	template <typename T> void bind()
	{
		instantiateScript = []() {
			return static_cast<ScriptableEntity *>(new T());
		} destroyScript = [](NativeScriptComponent *nsc) {
			delete nsc->instance;
			nsc->instance = nullptr;
		}
	}
};

template <typename... Component> struct ComponentGroup
{
};

/**
 * @typedef AllComponents
 * @brief 엔티티에 포함될 수 있는 모든 컴포넌트 그룹.
 */
using AllComponents =
	ComponentGroup<TransformComponent, RelationshipComponent, MeshRendererComponent, TextureComponent, CameraComponent,
				   ScriptComponent, LightComponent, RigidbodyComponent, BoxColliderComponent, SphereColliderComponent,
				   CapsuleColliderComponent, CylinderColliderComponent>;

} // namespace ale
