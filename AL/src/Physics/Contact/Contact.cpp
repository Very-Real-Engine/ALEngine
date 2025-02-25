#include "Physics/Contact/Contact.h"
#include "Physics/Shape/ShapeCollisions.h"

namespace ale
{
int32_t operator&(int32_t val, EContactFlag flag)
{
	return val & static_cast<int32_t>(flag);
}

int32_t operator|(int32_t val, EContactFlag flag)
{
	return val | static_cast<int32_t>(flag);
}

int32_t operator~(EContactFlag flag)
{
	return ~static_cast<int32_t>(flag);
}

bool operator==(int32_t val, EContactFlag flag)
{
	return static_cast<int32_t>(flag) == val;
}

contactMemberFunction Contact::createContactFunctions[32] = {
	nullptr,							// 0
	&SphereToSphereContact::create,		// 01
	&BoxToBoxContact::create,			// 10
	&SphereToBoxContact::create,		// 11
	&BoxToBoxContact::create,			// 100
	&SphereToBoxContact::create,		// 101
	&BoxToBoxContact::create,			// 110
	nullptr,							// 111
	&CylinderToCylinderContact::create, // 1000
	&SphereToCylinderContact::create,	// 1001
	&BoxToCylinderContact::create,		// 1010
	nullptr,							// 1011
	&BoxToCylinderContact::create,		// 1100
	nullptr,							// 1101
	nullptr,							// 1110
	nullptr,							// 1111
	&CapsuleToCapsuleContact::create,	// 10000
	&SphereToCapsuleContact::create,	// 10001
	&BoxToCapsuleContact::create,		// 10010
	nullptr,							// 10011
	&BoxToCapsuleContact::create,		// 10100
	nullptr,							// 10101
	nullptr,							// 10110
	nullptr,							// 10111
	&CylinderToCapsuleContact::create,	// 11000
	nullptr,							// 11001
	nullptr,							// 11010
	nullptr,							// 11011
	nullptr,							// 11100
	nullptr,							// 11101
	nullptr,							// 11110
	nullptr,							// 11111
};

Contact::Contact(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB)
	: m_fixtureA(fixtureA), m_fixtureB(fixtureB), m_indexA(indexA), m_indexB(indexB)
{
	m_flags = static_cast<int32_t>(EContactFlag::TOUCHING);

	m_fixtureA = fixtureA;
	m_fixtureB = fixtureB;

	m_indexA = indexA;
	m_indexB = indexB;

	m_prev = nullptr;
	m_next = nullptr;

	m_wasTouched = false;
	m_nodeA.contact = nullptr;
	m_nodeA.prev = nullptr;
	m_nodeA.next = nullptr;
	m_nodeA.other = nullptr;

	m_nodeB.contact = nullptr;
	m_nodeB.prev = nullptr;
	m_nodeB.next = nullptr;
	m_nodeB.other = nullptr;

	m_friction = std::sqrt(m_fixtureA->getFriction() * m_fixtureB->getFriction());
	m_restitution = std::max(m_fixtureA->getRestitution(), m_fixtureB->getRestitution());
}

Contact *Contact::create(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB)
{
	// 각 fixture의 shape의 type 가져오기
	EType type1 = fixtureA->getType();
	EType type2 = fixtureB->getType();

	if (type1 > type2)
	{
		Fixture *tmpFixture = fixtureA;
		int32_t tmpIndex = indexA;
		fixtureA = fixtureB;
		indexA = indexB;
		fixtureB = tmpFixture;
		indexB = tmpIndex;
	}

	return createContactFunctions[type1 | type2](fixtureA, fixtureB, indexA, indexB);
}

void Contact::evaluate(Manifold &manifold, const Transform &transformA, const Transform &transformB, bool isSensor)
{
	Shape *shapeA = m_fixtureA->getShape();
	Shape *shapeB = m_fixtureB->getShape();

	ConvexInfo convexA = shapeA->getShapeInfo(transformA);
	ConvexInfo convexB = shapeB->getShapeInfo(transformB);

	SimplexArray simplexArray;
	CollisionInfo collisionInfo;

	simplexArray.simplexCount = 0;
	collisionInfo.size = 0;

	bool isCollide = getGjkResult(convexA, convexB, simplexArray);

	if (isCollide)
	{
		if (isSensor)
		{
			manifold.pointsCount = 1;
			freeConvexInfo(convexA, convexB);
			return ;
		}

		EpaInfo epaInfo = getEpaResult(convexA, convexB, simplexArray);

		if (epaInfo.distance == -1.0f)
		{
			freeConvexInfo(convexA, convexB);
			return;
		}

		findCollisionPoints(convexA, convexB, collisionInfo, epaInfo, simplexArray);

		generateManifolds(collisionInfo, manifold, m_fixtureA, m_fixtureB);
	}

	freeConvexInfo(convexA, convexB);
}

void Contact::update()
{
	// 이전 프레임에서 두 객체가 충돌중이었는지 확인
	bool touching = false;

	// bodyA, bodyB의 Transform 가져오기
	Rigidbody *bodyA = m_fixtureA->getBody();
	Rigidbody *bodyB = m_fixtureB->getBody();
	const Transform &transformA = bodyA->getTransform();
	const Transform &transformB = bodyB->getTransform();

	bool isSensor = false;
	if (m_fixtureA->isSeonsor() || m_fixtureB->isSeonsor())
	{
		isSensor = true;
		m_manifold.pointsCount = 0;
		evaluate(m_manifold, transformA, transformB, isSensor);
		touching = m_manifold.pointsCount > 0;
	}
	else
	{
		// Evaluate
		// 두 shape의 변환 상태를 적용해 world space에서의 충돌 정보를 계산
		// 1. 두 도형이 실제로 충돌하는지 검사
		// 2. 충돌에 따른 manifold 생성
		// 3. manifold의 내부 값을 impulse를 제외하고 채워줌
		// 4. 실제 충돌이 일어나지 않은 경우 manifold.pointCount = 0인 충돌 생성
		
		// manifold의 충격량 0으로 초기화 및 old manifold 중
		// 같은 충돌이 있는경우 Impulse 재사용
		// id 는 충돌 도형의 type과 vertex 또는 line의 index 정보를 압축하여 결정
		m_manifold.pointsCount = 0;
		evaluate(m_manifold, transformA, transformB, isSensor);
		touching = m_manifold.pointsCount > 0;
		
		for (int32_t i = 0; i < m_manifold.pointsCount; ++i)
		{
			ManifoldPoint &manifoldPoint = m_manifold.points[i];
	
			manifoldPoint.normalImpulse = 0.0f;
			manifoldPoint.tangentImpulse = 0.0f;
		}
	}

	if (isSensor)
	{
		if (m_wasTouched && !touching)
		{
			m_fixtureA->decreaseTouchNum();
			m_fixtureB->decreaseTouchNum();
		}
		if (!m_wasTouched && touching)
		{
			m_fixtureA->increaseTouchNum();
			m_fixtureB->increaseTouchNum();
		}
	}

	if (touching)
	{
		m_flags = m_flags | EContactFlag::TOUCHING;
		m_wasTouched = true;
	}
	else
	{
		m_flags = m_flags & ~EContactFlag::TOUCHING;
		m_wasTouched = false;
	}
}

float Contact::getFriction() const
{
	return m_friction;
}

float Contact::getRestitution() const
{
	return m_restitution;
}

Contact *Contact::getNext()
{
	return m_next;
}

Fixture *Contact::getFixtureA() const
{
	return m_fixtureA;
}

Fixture *Contact::getFixtureB() const
{
	return m_fixtureB;
}

int32_t Contact::getChildIndexA() const
{
	return m_indexA;
}

int32_t Contact::getChildIndexB() const
{
	return m_indexB;
}

ContactLink *Contact::getNodeA()
{
	return &m_nodeA;
}

ContactLink *Contact::getNodeB()
{
	return &m_nodeB;
}

Manifold &Contact::getManifold()
{
	return m_manifold;
}

void Contact::setPrev(Contact *contact)
{
	m_prev = contact;
}

void Contact::setNext(Contact *contact)
{
	m_next = contact;
}

void Contact::setFlag(EContactFlag flag)
{
	m_flags = m_flags | static_cast<int32_t>(flag);
}

void Contact::unsetFlag(EContactFlag flag)
{
	m_flags = m_flags & ~static_cast<int32_t>(flag);
}

bool Contact::hasFlag(EContactFlag flag)
{
	return (m_flags & static_cast<int32_t>(flag)) == static_cast<int32_t>(flag);
}

// manifold functions

Simplex Contact::getSupportPoint(const ConvexInfo &convexA, const ConvexInfo &convexB, alglm::vec3 &dir)
{
	Simplex simplex;
	simplex.a = supportA(convexA, dir);
	simplex.b = supportB(convexB, -dir);
	simplex.diff = simplex.a - simplex.b;

	return simplex;
}

bool Contact::handleLineSimplex(SimplexArray &simplexArray, alglm::vec3 &dir)
{
	// std::cout << "Line GJK\n";
	alglm::vec3 &a = simplexArray.simplices[0].diff;
	alglm::vec3 &b = simplexArray.simplices[1].diff;

	alglm::vec3 ab = b - a;
	alglm::vec3 ao = -a;

	if (isSameDirection(ab, ao))
	{
		alglm::vec3 tmpAxis(0, 1, 0);
		if (isSameDirection(ab, tmpAxis))
		{
			tmpAxis = alglm::vec3(1, 0, 0);

			dir = alglm::normalize(alglm::cross(ab, tmpAxis));
		}
		else
		{
			dir = alglm::normalize(alglm::cross(ab, tmpAxis));
		}
	}
	else
	{
		dir = alglm::normalize(alglm::cross(alglm::cross(ab, ao), ab));
	}

	return false;
}

bool Contact::handleTriangleSimplex(SimplexArray &simplexArray, alglm::vec3 &dir)
{
	alglm::vec3 &a = simplexArray.simplices[0].diff;
	alglm::vec3 &b = simplexArray.simplices[1].diff;
	alglm::vec3 &c = simplexArray.simplices[2].diff;

	alglm::vec3 ab = b - a;
	alglm::vec3 ac = c - a;
	alglm::vec3 bc = c - b;
	alglm::vec3 abc = alglm::cross(ac, ab);

	// 면적이 없으면 반대 dir로 세 번째 점 다시 찾기기
	if (alglm::length2(abc) == 0.0f)
	{
		--simplexArray.simplexCount;
		dir = -dir;
		return false;
	}

	dir = alglm::normalize(alglm::cross(abc, bc));
	if (alglm::dot(dir, -c) > 0.0f)
	{
		// a 제거
		simplexArray.simplices[0] = simplexArray.simplices[1];
		simplexArray.simplices[1] = simplexArray.simplices[2];
		--simplexArray.simplexCount;
		return false;
	}

	dir = alglm::normalize(alglm::cross(ac, abc));
	if (alglm::dot(dir, -c) > 0.0f)
	{
		// b 제거
		simplexArray.simplices[1] = simplexArray.simplices[2];
		--simplexArray.simplexCount;
		return false;
	}

	if (alglm::dot(-a, abc) > 0.0f)
	{
		dir = alglm::normalize(abc);
	}
	else
	{
		dir = alglm::normalize(-abc);
	}

	return false;
}

bool Contact::handleTetrahedronSimplex(SimplexArray &simplexArray, alglm::vec3 &dir)
{
	alglm::vec3 &a = simplexArray.simplices[0].diff;
	alglm::vec3 &b = simplexArray.simplices[1].diff;
	alglm::vec3 &c = simplexArray.simplices[2].diff;
	alglm::vec3 &d = simplexArray.simplices[3].diff;

	alglm::vec3 abc = alglm::cross((b - a), (c - a));
	// 부피가 없으면 반대 방향으로 4번째 점 다시 찾기기
	if (std::abs(alglm::dot((d - a), abc)) < 1e-8f)
	{
		--simplexArray.simplexCount;
		dir = -dir;
		return false;
	}

	// bcd 법선 방향에 원점 있는지 확인
	alglm::vec3 bc = c - b;
	alglm::vec3 bd = d - b;
	alglm::vec3 bcd = alglm::cross(bd, bc);
	if (alglm::dot(bcd, a - c) > 0)
	{
		bcd = -bcd;
	}

	if (alglm::dot(bcd, -d) > 0.0f)
	{
		simplexArray.simplices[0] = simplexArray.simplices[1];
		simplexArray.simplices[1] = simplexArray.simplices[2];
		simplexArray.simplices[2] = simplexArray.simplices[3];
		--simplexArray.simplexCount;
		dir = alglm::normalize(bcd);
		return false;
	}

	// acd 법선 방향에 원점 있는지 확인
	alglm::vec3 ca = a - c;
	alglm::vec3 cd = d - c;
	alglm::vec3 acd = alglm::cross(cd, ca);
	if (alglm::dot(acd, b - c) > 0)
	{
		acd = -acd;
	}

	if (alglm::dot(acd, -d) > 0.0f)
	{
		simplexArray.simplices[1] = simplexArray.simplices[2];
		simplexArray.simplices[2] = simplexArray.simplices[3];
		--simplexArray.simplexCount;
		dir = alglm::normalize(acd);
		return false;
	}

	// abd 법선 방향에 원점 있는지 확인
	alglm::vec3 ab = b - a;
	alglm::vec3 ad = d - a;
	alglm::vec3 abd = alglm::cross(ad, ab);

	if (alglm::dot(abd, c - a) > 0)
	{
		abd = -abd;
	}

	if (alglm::dot(abd, -d) > 0.0f)
	{
		simplexArray.simplices[2] = simplexArray.simplices[3];
		--simplexArray.simplexCount;
		dir = alglm::normalize(abd);
		return false;
	}

	return true;
}

// 가장 최근에 추가된 점 A = simplex.points.back() 로 가정
// simplex가 2, 3, 4개 점일 때 각각 처리 달라짐
bool Contact::handleSimplex(SimplexArray &simplexArray, alglm::vec3 &dir)
{
	switch (simplexArray.simplexCount)
	{
	case 2:
		return handleLineSimplex(simplexArray, dir);
	case 3:
		return handleTriangleSimplex(simplexArray, dir);
	case 4:
		return handleTetrahedronSimplex(simplexArray, dir);
	}
	return false;
}

bool Contact::isDuplicatedPoint(const SimplexArray &simplexArray, const alglm::vec3 &supportPoint)
{
	int32_t size = simplexArray.simplexCount;
	for (int32_t i = 0; i < size; i++)
	{
		if (alglm::length2(simplexArray.simplices[i].diff - supportPoint) < 1e-6f)
		{
			return true;
		}
	}
	return false;
}

bool Contact::checkSphereToSphereCollide(const ConvexInfo &convexA, const ConvexInfo &convexB)
{
	if (alglm::length(convexA.center - convexB.center) < (convexA.radius + convexB.radius))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool Contact::getGjkResult(const ConvexInfo &convexA, const ConvexInfo &convexB, SimplexArray &simplexArray)
{
	const int32_t ITERATION = 64;

	alglm::vec3 dir;

	// 첫 번째 support point 구하기
	if (alglm::length2(convexB.center - convexA.center) < 1e-8f)
	{
		dir = alglm::vec3(1.0f, 0.0f, 0.0f);
	}
	else
	{
		dir = alglm::normalize(convexB.center - convexA.center);
	}

	simplexArray.simplices[0] = getSupportPoint(convexA, convexB, dir);
	++simplexArray.simplexCount;

	alglm::vec3 supportPoint = simplexArray.simplices[0].diff;

	// 두 번째 support point 구하기
	if (alglm::length2(supportPoint) == 0.0f)
	{
		dir = -dir;
		simplexArray.simplices[0] = getSupportPoint(convexA, convexB, dir);
		supportPoint = simplexArray.simplices[0].diff;
	}

	dir = alglm::normalize(-supportPoint);

	int32_t iter = 0;
	while (iter < ITERATION)
	{
		// 새로운 서포트 점
		Simplex simplex = getSupportPoint(convexA, convexB, dir);
		supportPoint = simplex.diff;

		// 만약 newSupport가 direction과 내적(dot)했을 때 0 이하라면
		// 더 이상 원점을 "방향 dir" 쪽에서 감쌀 수 없음 => 충돌X
		if (alglm::dot(supportPoint, dir) < 0 || isDuplicatedPoint(simplexArray, supportPoint))
		{
			return false; // 교차하지 않음
		}

		// 심플렉스에 추가
		simplexArray.simplices[simplexArray.simplexCount] = simplex;
		++simplexArray.simplexCount;

		// 원점을 포함하는지 체크 및 simplex 갱신
		if (handleSimplex(simplexArray, dir))
		{
			// 원점 포함 => 충돌
			return true;
		}
		++iter;
	}

	return false;
}

bool Contact::isSimilarDirection(alglm::vec3 v1, alglm::vec3 v2)
{
	return alglm::dot(v1, v2) > 0.0f;
}

bool Contact::isSameDirection(alglm::vec3 v1, alglm::vec3 v2)
{
	return alglm::length2(alglm::cross(v1, v2)) == 0.0f;
}

EpaInfo Contact::getEpaResult(const ConvexInfo &convexA, const ConvexInfo &convexB, SimplexArray &simplexArray)
{
	const int32_t ITERATION = 64;

	FaceArray faceArray;
	FaceArray newFaceArray;
	int32_t initIdx[12] = {0, 1, 2, 0, 3, 1, 0, 2, 3, 1, 3, 2};

	memcpy(faceArray.faces, initIdx, sizeof(int32_t) * 12);
	faceArray.count = 4;

	// GJK에서 구한 simplex들중 원점에서 가장 가까운 삼각형의 법선과 최소 거리
	int32_t minFace = getFaceNormals(simplexArray, faceArray);
	alglm::vec3 minNormal(0.0f);
	float minDistance = FLT_MAX;

	if (minFace == -1)
	{
		minDistance = -1.0f;
	}

	int32_t iter = -1;

	while (minDistance == FLT_MAX)
	{
		if (++iter > ITERATION)
		{
			minNormal = alglm::vec3(0.0f);
			minDistance = -1.0f;
			break;
		}

		// 최소 거리의 법선, 거리 쿼리
		minNormal = alglm::vec3(faceArray.normals[minFace]);
		minDistance = faceArray.normals[minFace].w;

		// 최소 거리의 법선에 해당하는 supportPoint 쿼리
		Simplex simplex = getSupportPoint(convexA, convexB, minNormal);
		alglm::vec3 supportPoint = simplex.diff;

		// 원점에서 supportPoint까지의 거리
		float supportDistance = alglm::dot(minNormal, supportPoint);

		// supportPoint가 현재 minDistance보다 원점에서 더 멀리있는 경우
		// 다시 원점에서부터 최소거리의 삼각형을 찾음
		if (std::abs(supportDistance - minDistance) > 1e-2f && !isDuplicatedPoint(simplexArray, supportPoint) &&
			simplexArray.simplexCount < MAX_SIMPLEX_COUNT)
		{
			minDistance = FLT_MAX;

			UniqueEdges uniqueEdges;
			void *memory = PhysicsAllocator::m_stackAllocator.allocateStack(sizeof(std::pair<int32_t, int32_t>) *
																			faceArray.count * 3);
			uniqueEdges.edges = static_cast<std::pair<int32_t, int32_t> *>(memory);
			uniqueEdges.size = 0;

			for (int32_t i = 0; i < faceArray.count; i++)
			{
				alglm::vec3 center = (simplexArray.simplices[faceArray.faces[i * 3]].diff +
									  simplexArray.simplices[faceArray.faces[i * 3 + 1]].diff +
									  simplexArray.simplices[faceArray.faces[i * 3 + 2]].diff) /
									 3.0f;
				if (isSimilarDirection(faceArray.normals[i], supportPoint - center))
				{
					int32_t faceIdx = i * 3;
					// 해당 법선의 기존 삼각형의 edge들을 uniqueEdges에 저장
					// 만약 같은 edge가 2번 들어오면 사라질 edge로 판단하여 삭제
					// 1번만 들어오는 edge들만 모아서 새로운 점과 조합하여 새로운 삼각형 생성성
					addIfUniqueEdge(uniqueEdges, faceArray.faces, faceIdx, faceIdx + 1);
					addIfUniqueEdge(uniqueEdges, faceArray.faces, faceIdx + 1, faceIdx + 2);
					addIfUniqueEdge(uniqueEdges, faceArray.faces, faceIdx + 2, faceIdx);

					int32_t normalLastIdx = faceArray.count - 1;
					int32_t faceLastIdx = (normalLastIdx) * 3;
					faceArray.faces[faceIdx] = faceArray.faces[faceLastIdx];
					faceArray.faces[faceIdx + 1] = faceArray.faces[faceLastIdx + 1];
					faceArray.faces[faceIdx + 2] = faceArray.faces[faceLastIdx + 2];
					faceArray.normals[i] = faceArray.normals[normalLastIdx]; // pop-erase

					--faceArray.count;

					--i;
				}
			}

			// uniqueEdge에 있는 edge들과 새로운 점을 조함하여 face 생성

			newFaceArray.count = 0;

			int32_t uniqueEdgesCount = uniqueEdges.size;
			for (int32_t i = 0; i < uniqueEdgesCount; ++i)
			{
				int32_t edgeIdx1 = uniqueEdges.edges[i].first;
				int32_t edgeIdx2 = uniqueEdges.edges[i].second;
				addFaceInFaceArray(newFaceArray, edgeIdx1, edgeIdx2, simplexArray.simplexCount);
			}

			// 새로 추가되는 면이 없다면 종료료
			if (newFaceArray.count == 0)
			{
				minNormal = alglm::vec3(0.0f);
				minDistance = -1.0f;
				break;
			}

			PhysicsAllocator::m_stackAllocator.freeStack();

			// 새로운 점 추가
			simplexArray.simplices[simplexArray.simplexCount] = simplex;
			++simplexArray.simplexCount;

			// 새로운 삼각형의 normal 벡터들과 최소 거리 쿼리
			int32_t newMinFace = getFaceNormals(simplexArray, newFaceArray);
			float oldMinDistance = FLT_MAX;

			if (newMinFace == -1)
			{
				minNormal = alglm::vec3(0.0f);
				minDistance = -1.0f;
				break;
			}

			// 기존 삼각형들 중 가장 거리가 짧은 삼각형 쿼리
			int32_t normalsSize = faceArray.count;
			for (int32_t i = 0; i < normalsSize; i++)
			{
				if (faceArray.normals[i].w < oldMinDistance)
				{
					oldMinDistance = faceArray.normals[i].w;
					minFace = i;
				}
			}

			// 새로운 삼각형들중 가장 짧은 거리가 존재하면 해당 face를 minFace로 설정
			if (newFaceArray.normals[newMinFace].w < oldMinDistance)
			{
				minFace = newMinFace + faceArray.count;
			}

			// 새로운 face, normal 추가
			mergeFaceArray(faceArray, newFaceArray);
		}
	}

	PhysicsAllocator::m_blockAllocator.freeBlock(faceArray.faces, sizeof(int32_t) * faceArray.maxCount * 3);
	PhysicsAllocator::m_blockAllocator.freeBlock(faceArray.normals, sizeof(alglm::vec4) * faceArray.maxCount);
	PhysicsAllocator::m_blockAllocator.freeBlock(newFaceArray.faces, sizeof(int32_t) * newFaceArray.maxCount * 3);
	PhysicsAllocator::m_blockAllocator.freeBlock(newFaceArray.normals, sizeof(alglm::vec4) * newFaceArray.maxCount);

	EpaInfo epaInfo;
	epaInfo.normal = minNormal;
	epaInfo.distance = minDistance;

	return epaInfo;
}

// simplex의 삼각형들의 법선벡터와 삼각형들중 원점에서 가장 멀리 떨어져있는 놈을 찾아서 반환
int32_t Contact::getFaceNormals(SimplexArray &simplexArray, FaceArray &faceArray)
{
	alglm::vec3 center(0.0f);

	int32_t simplexSize = simplexArray.simplexCount;
	for (int32_t i = 0; i < simplexSize; ++i)
	{
		center += simplexArray.simplices[i].diff;
	}

	center = center / static_cast<float>(simplexSize);

	int32_t minTriangle = 0;
	float minDistance = FLT_MAX;

	// 삼각형 순회
	int32_t facesSize = faceArray.count * 3;
	for (int32_t i = 0; i < facesSize; i = i + 3)
	{
		// 삼각형 꼭짓점들
		alglm::vec3 &a = simplexArray.simplices[faceArray.faces[i]].diff;
		alglm::vec3 &b = simplexArray.simplices[faceArray.faces[i + 1]].diff;
		alglm::vec3 &c = simplexArray.simplices[faceArray.faces[i + 2]].diff;

		// 삼각형의 법선
		alglm::vec3 crossResult = alglm::cross(b - a, c - a);

		alglm::vec3 normal = alglm::normalize(crossResult);
		// 삼각형과 원점 사이의 거리
		if (alglm::dot(normal, a - center) < 0)
		{
			normal = -normal;
		}

		float distance = alglm::dot(normal, a);
		if (distance < 0)
		{
			return -1;
		}

		// 법선 벡터 저장
		faceArray.normals[i / 3] = alglm::vec4(normal, distance);

		// 원점과 가장 가까운 삼각형 저장
		if (distance < minDistance)
		{
			minTriangle = i / 3;
			minDistance = distance;
		}
	}

	return minTriangle;
}

void Contact::addIfUniqueEdge(UniqueEdges &uniqueEdges, const int32_t *faces, int32_t p1, int32_t p2)
{
	int32_t size = uniqueEdges.size;
	int32_t i = 0;
	bool removeEdge = false;
	for (; i < size; ++i)
	{
		std::pair<int32_t, int32_t> &edge = uniqueEdges.edges[i];
		if (edge.first == faces[p2] && edge.second == faces[p1])
		{
			removeEdge = true;
			++i;
			break;
		}
	}

	for (; i < size; ++i)
	{
		uniqueEdges.edges[i - 1] = uniqueEdges.edges[i];
	}

	if (removeEdge)
	{
		--uniqueEdges.size;
	}
	else
	{
		uniqueEdges.edges[uniqueEdges.size] = std::make_pair(faces[p1], faces[p2]);
		++uniqueEdges.size;
	}
}

void Contact::generateManifolds(CollisionInfo &collisionInfo, Manifold &manifold, Fixture *m_fixtureA,
								Fixture *m_fixtureB)
{
	/*
		64bit = 27bit(small proxyId) 	 |
				27bit(big proxyId) 		 |
				5bit(small contact part) |
				5bit(big contact part)
	*/
	// static int64_t bitmask = 0xFFFFFFFF & ~0b11111;

	int32_t collisionInfoSize = collisionInfo.size;
	manifold.pointsCount = collisionInfoSize;

	for (int32_t i = 0; i < collisionInfoSize; ++i)
	{
		manifold.points[i].pointA = collisionInfo.pointA[i];
		manifold.points[i].pointB = collisionInfo.pointB[i];
		manifold.points[i].normal = collisionInfo.normal[i];
		manifold.points[i].seperation = collisionInfo.seperation[i];
	}
}

void Contact::buildManifoldFromPolygon(CollisionInfo &collisionInfo, const Face &refFace, const Face &incFace,
									   ContactPolygon &contactPolygon, EpaInfo &epaInfo)
{
	if (contactPolygon.pointsCount == 0)
	{
		return;
	}

	// Incident 면의 plane 구하기 (간단히 incFace.normal, incFace.distance)
	// 혹은 실제로 dot(incFace.normal, incFace.vertices[0]) 등으로 distance를 구해도 됨
	alglm::vec3 &normal = epaInfo.normal;
	float distance = epaInfo.distance;
	float refPlaneDist = refFace.distance;
	float incPlaneDist = incFace.distance;
	alglm::vec3 refN = refFace.normal;
	alglm::vec3 incN = incFace.normal;

	std::sort(
		contactPolygon.points, contactPolygon.points + contactPolygon.pointsCount,
		[&refN](const alglm::vec3 &a, const alglm::vec3 &b) { return alglm::dot(a, refN) < alglm::dot(b, refN); });

	// 각 꼭지점마다 물체 A,B에서의 좌표를 구해 penetration 등 계산
	// 여기서는 "Ref Face plane에서 A 물체 좌표, Incident Face plane에서 B 물체 좌표" 라고 가정
	float denominator = (-(alglm::dot(contactPolygon.points[0], refN) - refPlaneDist));
	float ratio;
	if (denominator < 1e-8f)
	{
		ratio = 0.0f;
	}
	else
	{
		ratio = distance / denominator;
	}

	int32_t polygonCount = contactPolygon.pointsCount;

	for (int32_t i = 0; i < polygonCount; ++i)
	{
		const alglm::vec3 &point = contactPolygon.points[i];

		// B측 point
		alglm::vec3 pointB = point;

		// 침투깊이
		float penentration = ratio * (-(alglm::dot(point, refN) - refPlaneDist));

		// A측 point
		alglm::vec3 pointA = point + normal * penentration;

		// 접촉 정보
		collisionInfo.normal[i] = refN;
		collisionInfo.pointA[i] = pointA;
		collisionInfo.pointB[i] = pointB;
		collisionInfo.seperation[i] = penentration;
		++collisionInfo.size;
	}
}

void Contact::clipPolygonAgainstPlane(ContactPolygon &contactPolygon, const alglm::vec3 &planeNormal, float planeDist)
{
	int32_t polygonCount = contactPolygon.pointsCount;
	if (polygonCount == 0)
		return;

	int32_t idx = 0;

	for (size_t i = 0; i < polygonCount; i++)
	{
		alglm::vec3 &curr = contactPolygon.points[i];
		alglm::vec3 &next = contactPolygon.points[(i + 1) % polygonCount];

		float distCurr = alglm::dot(planeNormal, curr) - planeDist;
		float distNext = alglm::dot(planeNormal, next) - planeDist;
		bool currInside = (distCurr <= 0.0f);
		bool nextInside = (distNext <= 0.0f);

		// CASE1: 둘 다 내부
		if (currInside && nextInside)
		{
			contactPolygon.buffer[idx] = next;
			++idx;
		}

		// CASE2: 밖->안
		else if (!currInside && nextInside)
		{
			float t = distCurr / (distCurr - distNext);
			alglm::vec3 intersect = curr + t * (next - curr);
			contactPolygon.buffer[idx] = intersect;
			++idx;
			contactPolygon.buffer[idx] = next;
			++idx;
		}
		// CASE3: 안->밖
		else if (currInside && !nextInside)
		{
			float t = distCurr / (distCurr - distNext);
			alglm::vec3 intersect = curr + t * (next - curr);
			contactPolygon.buffer[idx] = intersect;
			++idx;
		}
		// CASE4: 둘 다 밖 => nothing
	}

	memcpy(contactPolygon.points, contactPolygon.buffer, sizeof(alglm::vec3) * idx);
	contactPolygon.pointsCount = idx;
}

void Contact::computeContactPolygon(ContactPolygon &contactPolygon, Face &refFace, Face &incFace)
{
	// 초기 polygon: Incident Face의 4점
	memcpy(contactPolygon.points, incFace.vertices, sizeof(alglm::vec3) * incFace.verticesCount);
	contactPolygon.pointsCount = incFace.verticesCount;

	// Ref Face의 4개 엣지로 만들어지는 '4개 사이드 평면'에 대해 클리핑
	// refFace.vertices = v0,v1,v2,v3 라고 가정, CCW 형태
	alglm::vec3 *vertices = refFace.vertices;
	int32_t len = refFace.verticesCount;

	for (int32_t i = 0; i < len; i++)
	{
		alglm::vec3 start = vertices[i];
		alglm::vec3 end = vertices[(i + 1) % len];

		// edge
		alglm::vec3 edge = end - start;

		// refFace.normal과 edge의 cross => 사이드 plane normal
		alglm::vec3 sideN = alglm::cross(refFace.normal, edge);
		sideN = alglm::normalize(sideN);

		float planeDist = alglm::dot(sideN, start);
		clipPolygonAgainstPlane(contactPolygon, sideN, planeDist);

		if (contactPolygon.pointsCount == 0)
		{
			break;
		}
	}
	// 마지막으로 "Ref Face 자체" 평면에 대해서도 클리핑(뒤집힌 면 제거)
	clipPolygonAgainstPlane(contactPolygon, refFace.normal, refFace.distance);
}

void Contact::sortVerticesClockwise(alglm::vec3 *vertices, const alglm::vec3 &center, const alglm::vec3 &normal,
									int32_t verticesSize)
{
	// 1. 법선 벡터 기준으로 평면의 두 축 정의
	alglm::vec3 u = alglm::normalize(alglm::cross(normal, alglm::vec3(1.0f, 0.0f, 0.0f)));
	if (alglm::length(u) < 1e-6)
	{
		// normal이 x축과 평행한 경우 y축 사용
		u = alglm::normalize(alglm::cross(normal, alglm::vec3(0.0f, 1.0f, 0.0f)));
	}
	alglm::vec3 v = alglm::normalize(alglm::cross(normal, u)); // 법선과 u의 외적

	// 2. 각도 계산 및 정렬
	auto angleComparator = [&center, &u, &v](const alglm::vec3 &a, const alglm::vec3 &b) {
		// a와 b를 u, v 축 기준으로 투영
		alglm::vec3 da = a - center;
		alglm::vec3 db = b - center;

		float angleA = atan2(alglm::dot(da, v), alglm::dot(da, u));
		float angleB = atan2(alglm::dot(db, v), alglm::dot(db, u));

		return angleA > angleB; // 시계 방향 정렬
	};

	std::sort(vertices, vertices + verticesSize, angleComparator);
}

void Contact::setBoxFace(Face &face, const ConvexInfo &box, const alglm::vec3 &normal)
{
	// 1) box의 6개 면 중, worldNormal과 가장 유사한 면( dot > 0 ) 찾기
	// 2) 그 면의 4개 꼭지점을 구함
	// 여기서는 '법선이 박스의 +Y축과 가장 가까우면 top face' 식으로 단순화 예시
	// 실제 구현은 회전/transform 고려해야 함

	alglm::vec3 axes[6] = {-box.axes[0], -box.axes[1], -box.axes[2], box.axes[0], box.axes[1], box.axes[2]};

	float maxDotRes = -FLT_MAX;
	int32_t maxIdx = -1;
	for (int32_t i = 0; i < 6; i++)
	{
		float nowDotRes = alglm::dot(axes[i], normal);
		if (nowDotRes > maxDotRes)
		{
			maxDotRes = nowDotRes;
			maxIdx = i;
		}
	}

	alglm::vec3 axis = axes[maxIdx];
	float centerDotRes = alglm::dot(box.center, axis);
	alglm::vec3 center(0.0f);

	int32_t pointCount = box.pointsCount;
	int32_t idx = 0;
	for (int32_t i = 0; i < pointCount; ++i)
	{
		alglm::vec3 &point = box.points[i];
		if (alglm::dot(point, axis) > centerDotRes)
		{
			center += point;
			face.vertices[idx] = point;
			++idx;
		}
	}

	center = alglm::vec3((center.x / idx), (center.y / idx), (center.z / idx));
	face.verticesCount = idx;
	face.normal = axis;
	face.distance = alglm::dot(axis, face.vertices[0]);

	sortVerticesClockwise(face.vertices, center, face.normal, face.verticesCount);
}

void Contact::setCylinderFace(Face &face, const ConvexInfo &cylinder, const alglm::vec3 &normal)
{
	alglm::vec3 center;
	int32_t segments = 20;
	float length = alglm::dot(normal, cylinder.axes[0]);
	float angleStep = 2.0f * alglm::pi<float>() / static_cast<float>(segments);

	float limit = alglm::dot(cylinder.axes[0], alglm::normalize(cylinder.points[0] - cylinder.center));

	if (length > limit)
	{
		face.verticesCount = segments;
		int32_t len = segments;
		for (int32_t i = 0; i < len; ++i)
		{
			face.vertices[i] = cylinder.points[i];
		}

		center = cylinder.center + cylinder.axes[0] * cylinder.height * 0.5f;
		face.normal = cylinder.axes[0];
		face.distance = alglm::dot(cylinder.axes[0], face.vertices[0]);
	}
	else if (length < -limit)
	{
		face.verticesCount = segments;
		int32_t len = segments * 2;
		for (int32_t i = segments; i < len; ++i)
		{
			face.vertices[i - segments] = cylinder.points[i];
		}

		center = cylinder.center - cylinder.axes[0] * cylinder.height * 0.5f;
		face.normal = -cylinder.axes[0];
		face.distance = alglm::dot(-cylinder.axes[0], face.vertices[0]);
	}
	else
	{
		face.normal = normal;
		float dotResult = alglm::dot(normal, cylinder.axes[0]);
		if (dotResult != 0.0f)
		{
			face.normal = alglm::normalize(normal - dotResult * cylinder.axes[0]);
		}

		int32_t dir;
		float max = -FLT_MAX;

		for (int32_t i = 1; i <= segments; ++i)
		{
			dotResult = alglm::dot(cylinder.axes[i], face.normal);
			if (dotResult > max)
			{
				dir = i;
				max = dotResult;
			}
		}

		int32_t idx1 = dir - 1;
		int32_t idx2 = dir % segments;

		face.verticesCount = 4;
		face.vertices[0] = cylinder.points[idx1];
		face.vertices[1] = cylinder.points[idx2];
		face.vertices[2] = cylinder.points[idx1 + segments];
		face.vertices[3] = cylinder.points[idx2 + segments];

		face.distance = alglm::dot(face.normal, face.vertices[0]);
		center = (face.vertices[0] + face.vertices[1] + face.vertices[2] + face.vertices[3]) / 4.0f;
	}

	sortVerticesClockwise(face.vertices, center, face.normal, face.verticesCount);
}

void Contact::setCapsuleFace(Face &face, const ConvexInfo &capsule, const alglm::vec3 &normal)
{
	int32_t segments = 20;
	float angleStep = 2.0f * alglm::pi<float>() / static_cast<float>(segments);

	face.normal = normal;
	float dotResult = alglm::dot(normal, capsule.axes[0]);

	if (dotResult != 0.0f)
	{
		face.normal = alglm::normalize(normal - dotResult * capsule.axes[0]);
	}

	int32_t dir;
	float max = -FLT_MAX;

	for (int32_t i = 1; i <= segments; ++i)
	{
		dotResult = alglm::dot(capsule.axes[i], face.normal);
		if (dotResult > max)
		{
			dir = i;
			max = dotResult;
		}
	}

	int32_t idx1 = dir - 1;
	int32_t idx2 = dir % segments;

	face.verticesCount = 4;
	face.vertices[0] = capsule.points[idx1];
	face.vertices[1] = capsule.points[idx2];
	face.vertices[2] = capsule.points[idx1 + segments];
	face.vertices[3] = capsule.points[idx2 + segments];

	face.distance = alglm::dot(face.normal, face.vertices[0]);
	alglm::vec3 center = (face.vertices[0] + face.vertices[1] + face.vertices[2] + face.vertices[3]) / 4.0f;

	sortVerticesClockwise(face.vertices, center, face.normal, face.verticesCount);
}

bool Contact::isCollideToHemisphere(const ConvexInfo &capsule, const alglm::vec3 &dir)
{
	float dotResult = alglm::dot(capsule.axes[0], dir);
	return dotResult > 0.0001f || dotResult < -0.0001f;
}

void Contact::addFaceInFaceArray(FaceArray &faceArray, int32_t idx1, int32_t idx2, int32_t idx3)
{
	int32_t count = faceArray.count;
	int32_t faceIdx = count * 3;

	if (count >= faceArray.maxCount)
	{
		sizeUpFaceArray(faceArray, count * 2);
	}

	faceArray.faces[faceIdx] = idx1;
	faceArray.faces[faceIdx + 1] = idx2;
	faceArray.faces[faceIdx + 2] = idx3;
	++faceArray.count;
}

void Contact::mergeFaceArray(FaceArray &faceArray, FaceArray &newFaceArray)
{
	int32_t faceArrayCount = faceArray.count;
	int32_t newFaceArrayCount = newFaceArray.count;
	int32_t newCount = newFaceArrayCount + faceArrayCount;

	if (newCount > faceArray.maxCount)
	{
		int32_t newMaxCount = faceArrayCount;
		while (newMaxCount < newCount)
		{
			newMaxCount = newMaxCount * 2;
		}

		sizeUpFaceArray(faceArray, newMaxCount);
	}

	memcpy(faceArray.faces + faceArrayCount * 3, newFaceArray.faces, sizeof(int32_t) * newFaceArrayCount * 3);
	memcpy(faceArray.normals + faceArrayCount, newFaceArray.normals, sizeof(alglm::vec4) * newFaceArrayCount);

	faceArray.count = newCount;
}

void Contact::sizeUpFaceArray(FaceArray &faceArray, int32_t newMaxCount)
{
	int32_t count = faceArray.count;
	void *memory = PhysicsAllocator::m_blockAllocator.allocateBlock(3 * newMaxCount * sizeof(int32_t));
	int32_t *newFaces = static_cast<int32_t *>(memory);
	memcpy(newFaces, faceArray.faces, sizeof(int32_t) * count * 3);

	memory = PhysicsAllocator::m_blockAllocator.allocateBlock(newMaxCount * sizeof(alglm::vec4));
	alglm::vec4 *newNormals = static_cast<alglm::vec4 *>(memory);
	memcpy(newNormals, faceArray.normals, sizeof(alglm::vec4) * count);

	PhysicsAllocator::m_blockAllocator.freeBlock(faceArray.faces, sizeof(int32_t) * faceArray.maxCount * 3);
	PhysicsAllocator::m_blockAllocator.freeBlock(faceArray.normals, sizeof(alglm::vec4) * faceArray.maxCount);

	faceArray.maxCount = newMaxCount;
	faceArray.faces = newFaces;
	faceArray.normals = newNormals;
}

void Contact::freeConvexInfo(ConvexInfo &convexA, ConvexInfo &convexB)
{
	if (convexA.points != nullptr)
	{
		PhysicsAllocator::m_blockAllocator.freeBlock(convexA.points, sizeof(alglm::vec3) * convexA.pointsCount);
	}
	if (convexB.points != nullptr)
	{
		PhysicsAllocator::m_blockAllocator.freeBlock(convexB.points, sizeof(alglm::vec3) * convexB.pointsCount);
	}

	if (convexA.axes != nullptr)
	{
		PhysicsAllocator::m_blockAllocator.freeBlock(convexA.axes, sizeof(alglm::vec3) * convexA.axesCount);
	}

	if (convexB.axes != nullptr)
	{
		PhysicsAllocator::m_blockAllocator.freeBlock(convexB.axes, sizeof(alglm::vec3) * convexB.axesCount);
	}
}
} // namespace ale