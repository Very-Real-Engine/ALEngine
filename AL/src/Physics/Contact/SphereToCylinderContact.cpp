#include "Physics/Contact/SphereToCylinderContact.h"

namespace ale
{
SphereToCylinderContact::SphereToCylinderContact(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB)
	: Contact(fixtureA, fixtureB, indexA, indexB) {};

Contact *SphereToCylinderContact::create(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB)
{
	void *memory = PhysicsAllocator::m_blockAllocator.allocateBlock(sizeof(SphereToCylinderContact));
	return new (static_cast<SphereToCylinderContact *>(memory))
		SphereToCylinderContact(fixtureA, fixtureB, indexA, indexB);
}

alglm::vec3 SphereToCylinderContact::supportA(const ConvexInfo &sphere, alglm::vec3 dir)
{
	return sphere.center + dir * sphere.radius;
}

alglm::vec3 SphereToCylinderContact::supportB(const ConvexInfo &cylinder, alglm::vec3 dir)
{
	// 원기둥 정보
	alglm::vec3 center = cylinder.center;				   // 중심점
	alglm::vec3 axis = alglm::normalize(cylinder.axes[0]); // 높이 축 (단위 벡터)
	float height = cylinder.height;

	// 1. 축 방향으로 윗면/아랫면 선택

	bool isUpSide = true;
	float dotResult = alglm::dot(dir, axis);

	if (dotResult < 0)
	{
		isUpSide = false;
	}

	alglm::vec3 circleDir = dir - alglm::dot(dir, axis) * axis; // 축에 수직한 방향

	if (alglm::length2(circleDir) > 1e-8f)
	{
		circleDir = alglm::normalize(circleDir); // 정규화

		int32_t maxIdx;
		int32_t segments = 20;

		float max = -FLT_MAX;
		for (int32_t i = 0; i < segments; ++i)
		{
			dotResult = alglm::dot(cylinder.points[i], dir);
			if (dotResult > max)
			{
				maxIdx = i;
				max = dotResult;
			}
		}

		if (isUpSide == false)
		{
			maxIdx += segments;
		}

		return cylinder.points[maxIdx];
	}
	else
	{
		if (isUpSide)
		{
			return center + cylinder.axes[0] * height * 0.5f;
		}
		else
		{
			return center - cylinder.axes[0] * height * 0.5f;
		}
	}
}

void SphereToCylinderContact::findCollisionPoints(const ConvexInfo &sphere, const ConvexInfo &cylinder,
												  CollisionInfo &collisionInfo, EpaInfo &epaInfo,
												  SimplexArray &simplexArray)
{
	collisionInfo.normal[0] = epaInfo.normal;
	collisionInfo.seperation[0] = epaInfo.distance;
	collisionInfo.pointA[0] = sphere.center + epaInfo.normal * sphere.radius;
	collisionInfo.pointB[0] = collisionInfo.pointA[0] - collisionInfo.normal[0] * collisionInfo.seperation[0];
	++collisionInfo.size;
}
} // namespace ale