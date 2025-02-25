#include "Physics/Contact/CylinderToCapsuleContact.h"

namespace ale
{
CylinderToCapsuleContact::CylinderToCapsuleContact(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB)
	: Contact(fixtureA, fixtureB, indexA, indexB) {};

Contact *CylinderToCapsuleContact::create(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB)
{
	void *memory = PhysicsAllocator::m_blockAllocator.allocateBlock(sizeof(CylinderToCapsuleContact));
	return new (static_cast<CylinderToCapsuleContact *>(memory))
		CylinderToCapsuleContact(fixtureA, fixtureB, indexA, indexB);
}

alglm::vec3 CylinderToCapsuleContact::supportA(const ConvexInfo &cylinder, alglm::vec3 dir)
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

alglm::vec3 CylinderToCapsuleContact::supportB(const ConvexInfo &capsule, alglm::vec3 dir)
{
	float dotResult = alglm::dot(dir, capsule.axes[0]);

	alglm::vec3 move(0.0f);

	if (dotResult > 0)
	{
		move = capsule.axes[0] * capsule.height * 0.5f;
	}
	else if (dotResult < 0)
	{
		move = -capsule.axes[0] * capsule.height * 0.5f;
	}

	return capsule.center + move + dir * capsule.radius;
}

void CylinderToCapsuleContact::findCollisionPoints(const ConvexInfo &cylinder, const ConvexInfo &capsule,
												   CollisionInfo &collisionInfo, EpaInfo &epaInfo,
												   SimplexArray &simplexArray)
{
	if (isCollideToHemisphere(capsule, -epaInfo.normal))
	{
		collisionInfo.normal[0] = epaInfo.normal;
		collisionInfo.seperation[0] = epaInfo.distance;

		if (alglm::dot(capsule.axes[0], collisionInfo.normal[0]) < 0)
		{
			alglm::vec3 hemisphereCenter = capsule.center + capsule.axes[0] * 0.5f * capsule.height;
			collisionInfo.pointB[0] = hemisphereCenter - collisionInfo.normal[0] * capsule.radius;
			collisionInfo.pointA[0] = collisionInfo.pointB[0] + collisionInfo.normal[0] * collisionInfo.seperation[0];
		}
		else
		{
			alglm::vec3 hemisphereCenter = capsule.center - capsule.axes[0] * 0.5f * capsule.height;
			collisionInfo.pointB[0] = hemisphereCenter - collisionInfo.normal[0] * capsule.radius;
			collisionInfo.pointA[0] = collisionInfo.pointB[0] + collisionInfo.normal[0] * collisionInfo.seperation[0];
		}
		++collisionInfo.size;
	}
	else
	{
		Face refFace, incFace;

		setCylinderFace(refFace, cylinder, epaInfo.normal);
		setCapsuleFace(incFace, capsule, -epaInfo.normal);

		ContactPolygon contactPolygon;
		computeContactPolygon(contactPolygon, refFace, incFace);

		buildManifoldFromPolygon(collisionInfo, refFace, incFace, contactPolygon, epaInfo);
	}
}
} // namespace ale