#pragma once

#include <queue>

#include "Physics/Shape/BoxShape.h"
#include "Physics/Shape/SphereShape.h"

namespace ale
{
/**
 * @file
 * @brief 물리 엔진에서 강체(Rigidbody)를 정의하는 클래스 및 관련 구조체.
 */

class World;
class Fixture;
struct FixtureDef;
struct ContactLink;

/**
 * @enum EBodyType
 * @brief Rigidbody의 유형을 정의하는 열거형.
 */
enum class EBodyType
{
	STATIC_BODY = 0,
	KINEMATIC_BODY,
	DYNAMIC_BODY
};

/**
 * @enum EBodyFlag
 * @brief Rigidbody의 플래그 값을 정의하는 열거형.
 */
enum class EBodyFlag
{
	ISLAND = (1 << 0),
};

/**
 * @struct BodyDef
 * @brief Rigidbody의 초기 설정을 정의하는 구조체.
 */
struct BodyDef
{
	/**
	 * @brief 기본 생성자. 모든 값을 기본 상태로 초기화합니다.
	 */
	BodyDef()
	{
		// userData = nullptr;
		// set position
		// position()
		m_position = alglm::vec3(0.0f);
		// m_angle = 0.0f;
		m_orientation = alglm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		m_linearVelocity = alglm::vec3(0.0f);
		m_angularVelocity = alglm::vec3(0.0f);
		m_linearDamping = 0.0f;
		m_angularDamping = 0.0f;
		m_canSleep = true;
		m_isAwake = true;
		m_useGravity = true;
		m_type = EBodyType::STATIC_BODY;
		m_gravityScale = 15.0f;
		alglm::vec3 m_posFreeze = alglm::vec3(1.0f);
		alglm::vec3 m_rotFreeze = alglm::vec3(1.0f);
	}

	EBodyType m_type;
	alglm::vec3 m_position;
	alglm::quat m_orientation;

	alglm::vec3 m_posFreeze;
	alglm::vec3 m_rotFreeze;
	// float m_angle;

	alglm::vec3 m_linearVelocity;
	alglm::vec3 m_angularVelocity;
	float m_linearDamping;
	float m_angularDamping;
	bool m_canSleep;
	bool m_isAwake;
	bool m_useGravity;
	// void *userData;
	float m_gravityScale;
	int32_t m_xfId;
};

/**
 * @class Rigidbody
 * @brief 물리 엔진에서 강체(Rigidbody)의 동작을 담당하는 클래스.
 */
class Rigidbody
{
  public:
	/**
	 * @brief Rigidbody 생성자.
	 * @param bd Rigidbody 설정 정보.
	 * @param world Rigidbody가 속할 물리 월드.
	 */
	Rigidbody(const BodyDef *bd, World *world);

	/**
	 * @brief Rigidbody 소멸자.
	 */
	~Rigidbody();

	/** @brief 크기 조정. */
	void scale(float scale);

	/** @brief 이동. */
	void translate(float distance);

	/** @brief 외부에서 가해지는 힘을 추가합니다. */
	void addForce(const alglm::vec3 &force);

	/** @brief 외부에서 가해지는 토크(회전력)를 추가합니다. */
	void addTorque(const alglm::vec3 &torque);

	/** @brief 중력을 추가합니다. */
	void addGravity();

	/** @brief Rigidbody의 운동 방정식을 적용합니다. */
	void integrate(float duration);

	/** @brief Sweep 정보를 업데이트합니다. */
	void updateSweep();

	/** @brief 외부 힘을 등록합니다. */
	void registerForce(const alglm::vec3 &force);

	/** @brief 새로운 충돌 형태(Fixture)를 추가합니다. */
	void createFixture(Shape *shape);

	/** @brief 새로운 충돌 형태(Fixture)를 추가합니다. */
	void createFixture(const FixtureDef *fd);

	/** @brief 특정 위치에서 힘을 가합니다. */
	void addForceAtPoint(const alglm::vec3 &force, const alglm::vec3 &point);

	/** @brief 특정 위치에서 힘을 가합니다. */
	void addForceAtBodyPoint(const alglm::vec3 &force, const alglm::vec3 &point);

	/** @brief 힘과 토크 누적값을 초기화합니다. */
	void clearAccumulators();

	/** @brief 충돌 영역을 동기화합니다. */
	void synchronizeFixtures();

	/** @brief Rigidbody의 변환 행렬을 업데이트합니다. */
	void calculateDerivedData();

	/** @brief 누적된 힘을 계산합니다. */
	void calculateForceAccum();

	/** @brief 특정 플래그가 설정되어 있는지 확인합니다. */
	bool hasFlag(EBodyFlag flag);

	/** @brief 두 Rigidbody 간 충돌 가능 여부를 검사합니다. */
	bool shouldCollide(const Rigidbody *other) const;

	/** @brief 역질량을 반환합니다. */
	float getInverseMass() const;

	/** @brief 변환(Transform) ID를 반환합니다. */
	int32_t getTransformId() const;

	/** @brief 아일랜드(연결된 Rigidbody 그룹) 인덱스를 반환합니다. */
	int32_t getIslandIndex() const;

	/** @brief Rigidbody의 고유 ID를 반환합니다. */
	int32_t getBodyId() const;

	/** @brief Rigidbody의 타입을 반환합니다. */
	EBodyType getType() const;

	/** @brief Rigidbody가 포함된 ContactLink 리스트를 반환합니다. */
	ContactLink *getContactLinks();

	/**
	 * @brief Rigidbody의 특정 로컬 좌표를 월드 좌표계로 변환합니다.
	 * @param point 변환할 로컬 좌표.
	 * @return 변환된 월드 좌표.
	 */
	alglm::vec3 getPointInWorldSpace(const alglm::vec3 &point) const;

	/** @brief Rigidbody의 현재 위치를 반환합니다. */
	const alglm::vec3 &getPosition() const;

	/** @brief Rigidbody의 방향(Orientation)을 반환합니다. */
	const alglm::quat &getOrientation() const;

	/** @brief Rigidbody의 Transform을 반환합니다. */
	const Transform &getTransform() const;

	/** @brief Rigidbody의 변환 행렬을 반환합니다. */
	const alglm::mat4 &getTransformMatrix() const;

	/** @brief Rigidbody의 선형 속도를 반환합니다. */
	const alglm::vec3 &getLinearVelocity() const;

	/** @brief Rigidbody의 각속도를 반환합니다. */
	const alglm::vec3 &getAngularVelocity() const;

	/** @brief Rigidbody의 현재 가속도를 반환합니다. */
	const alglm::vec3 &getAcceleration() const;

	/** @brief 월드 공간에서의 역관성 텐서를 반환합니다. */
	const alglm::mat3 &getInverseInertiaTensorWorld() const;

	// Setter functions

	/**
	 * @brief Rigidbody에 특정 플래그를 설정합니다.
	 * @param flag 설정할 플래그.
	 */
	void setFlag(EBodyFlag flag);

	/**
	 * @brief Rigidbody에서 특정 플래그를 해제합니다.
	 * @param flag 해제할 플래그.
	 */
	void unsetFlag(EBodyFlag flag);

	/**
	 * @brief 질량과 관성 텐서를 설정합니다.
	 * @param mass 질량 값.
	 * @param inertiaTensor 관성 텐서.
	 */
	void setMassData(float mass, const alglm::mat3 &inertiaTensor);

	/**
	 * @brief Rigidbody의 질량을 설정합니다.
	 * @param mass 설정할 질량 값.
	 */
	void setMass(float mass);

	/**
	 * @brief Rigidbody의 위치를 설정합니다.
	 * @param position 설정할 위치.
	 */
	void setPosition(alglm::vec3 &position);

	/**
	 * @brief Rigidbody의 아일랜드 인덱스를 설정합니다.
	 * @param idx 설정할 아일랜드 인덱스.
	 */
	void setIslandIndex(int32_t idx);

	/**
	 * @brief Rigidbody의 방향(Orientation)을 설정합니다.
	 * @param orientation 설정할 방향(쿼터니언).
	 */
	void setOrientation(alglm::quat &orientation);

	/**
	 * @brief Rigidbody의 가속도를 설정합니다.
	 * @param acceleration 설정할 가속도.
	 */
	void setAcceleration(const alglm::vec3 &acceleration);

	/**
	 * @brief Rigidbody의 ContactLink 리스트를 설정합니다.
	 * @param contactLink 설정할 ContactLink 포인터.
	 */
	void setContactLinks(ContactLink *contactLink);

	/**
	 * @brief Rigidbody의 선형 속도를 설정합니다.
	 * @param linearVelocity 설정할 선형 속도.
	 */
	void setLinearVelocity(alglm::vec3 &linearVelocity);

	/**
	 * @brief Rigidbody의 각속도를 설정합니다.
	 * @param angularVelocity 설정할 각속도.
	 */
	void setAngularVelocity(alglm::vec3 &angularVelocity);

	/**
	 * @brief Rigidbody를 수면(비활성) 상태로 설정합니다.
	 * @param duration 수면 상태로 유지할 시간.
	 */
	void setSleep(float duration);

	/** @brief Rigidbody를 활성 상태로 설정합니다. */
	void setAwake();

	/**
	 * @brief BodyDef 값을 이용하여 Rigidbody의 속성을 설정합니다.
	 * @param bdDef 설정할 BodyDef 객체.
	 */
	void setRBComponentValue(BodyDef &bdDef);

	/** @brief Rigidbody가 깨어 있는지 여부를 반환합니다. */
	bool isAwake();
	int32_t getTouchNum() const;

	Rigidbody *next;
	Rigidbody *prev;

  protected:
	static int32_t BODY_COUNT;
	static const float START_SLEEP_TIME;

	World *m_world;

	Sweep m_sweep;
	Transform m_xf;
	alglm::vec3 m_linearVelocity;
	alglm::vec3 m_angularVelocity;
	alglm::mat3 m_inverseInertiaTensorWorld;
	alglm::mat3 m_inverseInertiaTensor;
	alglm::mat4 m_transformMatrix;
	alglm::vec3 m_forceAccum;
	alglm::vec3 m_torqueAccum;
	alglm::vec3 m_acceleration;
	alglm::vec3 m_lastFrameAcceleration;
	std::queue<alglm::vec3> m_forceRegistry;

	int32_t m_fixtureCount;
	Fixture *m_fixtures = nullptr;

	// float motion;
	bool m_isAwake;
	bool m_canSleep;
	bool m_useGravity;
	float m_sleepTime;

	float m_inverseMass;
	float m_linearDamping;
	float m_angularDamping;
	float m_gravityScale;
	int32_t m_xfId;
	int32_t m_flags;
	int32_t m_islandIndex;
	int32_t m_bodyID;
	EBodyType m_type;
	alglm::vec3 m_posFreeze;
	alglm::vec3 m_rotFreeze;

	ContactLink *m_contactLinks;

  private:
};
} // namespace ale