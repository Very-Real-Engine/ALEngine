#pragma once

#include <queue>

#include "Physics/Shape/BoxShape.h"
#include "Physics/Shape/SphereShape.h"

namespace ale
{
class World;
class Fixture;
struct FixtureDef;
struct ContactLink;

enum class EBodyType
{
	STATIC_BODY = 0,
	KINEMATIC_BODY,
	DYNAMIC_BODY
};

enum class EBodyFlag
{
	ISLAND = (1 << 0),
};

struct BodyDef
{
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

class Rigidbody
{
  public:
	Rigidbody(const BodyDef *bd, World *world);
	~Rigidbody();

	void scale(float scale);
	void translate(float distance);
	void addForce(const alglm::vec3 &force);
	void addTorque(const alglm::vec3 &torque);
	void addGravity();
	void integrate(float duration);
	void updateSweep();
	void registerForce(const alglm::vec3 &force);
	void createFixture(Shape *shape);
	void createFixture(const FixtureDef *fd);
	void addForceAtPoint(const alglm::vec3 &force, const alglm::vec3 &point);
	void addForceAtBodyPoint(const alglm::vec3 &force, const alglm::vec3 &point);
	void clearAccumulators();
	void synchronizeFixtures();
	void calculateDerivedData();
	void calculateForceAccum();

	bool hasFlag(EBodyFlag flag);
	bool shouldCollide(const Rigidbody *other) const;

	// getter function
	float getInverseMass() const;
	int32_t getTransformId() const;
	int32_t getIslandIndex() const;
	int32_t getBodyId() const;
	EBodyType getType() const;
	ContactLink *getContactLinks();
	alglm::vec3 getPointInWorldSpace(const alglm::vec3 &point) const;
	const alglm::vec3 &getPosition() const;
	const alglm::quat &getOrientation() const;
	const Transform &getTransform() const;
	const alglm::mat4 &getTransformMatrix() const;
	const alglm::vec3 &getLinearVelocity() const;
	const alglm::vec3 &getAngularVelocity() const;
	const alglm::vec3 &getAcceleration() const;
	const alglm::mat3 &getInverseInertiaTensorWorld() const;

	void setFlag(EBodyFlag flag);
	void unsetFlag(EBodyFlag flag);
	void setMassData(float mass, const alglm::mat3 &inertiaTensor);
	void setMass(float mass);
	void setPosition(alglm::vec3 &position);
	void setIslandIndex(int32_t idx);
	void setOrientation(alglm::quat &orientation);
	void setAcceleration(const alglm::vec3 &acceleration);
	void setContactLinks(ContactLink *contactLink);
	void setLinearVelocity(alglm::vec3 &linearVelocity);
	void setAngularVelocity(alglm::vec3 &angularVelocity);
	void setSleep(float duration);
	void setAwake();
	void setRBComponentValue(BodyDef &bdDef);
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