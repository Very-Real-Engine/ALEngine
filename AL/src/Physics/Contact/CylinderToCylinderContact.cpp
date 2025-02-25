#include "Physics/Contact/CylinderToCylinderContact.h"

namespace ale
{
CylinderToCylinderContact::CylinderToCylinderContact(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA,
													 int32_t indexB)
	: Contact(fixtureA, fixtureB, indexA, indexB) {};

Contact *CylinderToCylinderContact::create(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB)
{
	void *memory = PhysicsAllocator::m_blockAllocator.allocateBlock(sizeof(CylinderToCylinderContact));
	return new (static_cast<CylinderToCylinderContact *>(memory))
		CylinderToCylinderContact(fixtureA, fixtureB, indexA, indexB);
}

alglm::vec3 CylinderToCylinderContact::supportA(const ConvexInfo &cylinder, alglm::vec3 dir)
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

alglm::vec3 CylinderToCylinderContact::supportB(const ConvexInfo &cylinder, alglm::vec3 dir)
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

void CylinderToCylinderContact::findCollisionPoints(const ConvexInfo &cylinderA, const ConvexInfo &cylinderB,
													CollisionInfo &collisionInfo, EpaInfo &epaInfo,
													SimplexArray &simplexArray)
{
	// std::cout << "cylinder vs cylinder!!\n";
	// clipping
	Face refFace, incFace;

	setCylinderFace(refFace, cylinderA, epaInfo.normal);
	setCylinderFace(incFace, cylinderB, -epaInfo.normal);

	ContactPolygon contactPolygon;
	computeContactPolygon(contactPolygon, refFace, incFace);

	// 폴리곤의 각 꼭지점 -> 충돌점 여러 개
	buildManifoldFromPolygon(collisionInfo, refFace, incFace, contactPolygon, epaInfo);
}
} // namespace ale