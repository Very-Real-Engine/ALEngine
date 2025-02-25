#include "Physics/Contact/BoxToCylinderContact.h"

namespace ale
{
BoxToCylinderContact::BoxToCylinderContact(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB)
	: Contact(fixtureA, fixtureB, indexA, indexB) {};

Contact *BoxToCylinderContact::create(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB)
{
	void *memory = PhysicsAllocator::m_blockAllocator.allocateBlock(sizeof(BoxToCylinderContact));
	return new (static_cast<BoxToCylinderContact *>(memory)) BoxToCylinderContact(fixtureA, fixtureB, indexA, indexB);
}

alglm::vec3 BoxToCylinderContact::supportA(const ConvexInfo &box, alglm::vec3 dir)
{
	float dotAxes[3] = {alglm::dot(box.axes[0], dir) > 0 ? 1.0f : -1.0f,
						alglm::dot(box.axes[1], dir) > 0 ? 1.0f : -1.0f,
						alglm::dot(box.axes[2], dir) > 0 ? 1.0f : -1.0f};

	alglm::vec3 point = box.center;
	for (int32_t i = 0; i < 3; ++i)
	{
		point += box.axes[i] * (dotAxes[i] * box.halfSize[i]);
	}

	return point;
}

alglm::vec3 BoxToCylinderContact::supportB(const ConvexInfo &cylinder, alglm::vec3 dir)
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

void BoxToCylinderContact::findCollisionPoints(const ConvexInfo &box, const ConvexInfo &cylinder,
											   CollisionInfo &collisionInfo, EpaInfo &epaInfo,
											   SimplexArray &simplexArray)
{
	// clipping
	Face refFace, incFace;

	setBoxFace(refFace, box, epaInfo.normal);
	setCylinderFace(incFace, cylinder, -epaInfo.normal);

	ContactPolygon contactPolygon;
	computeContactPolygon(contactPolygon, refFace, incFace);

	// 폴리곤의 각 꼭지점 -> 충돌점 여러 개
	buildManifoldFromPolygon(collisionInfo, refFace, incFace, contactPolygon, epaInfo);
}
} // namespace ale