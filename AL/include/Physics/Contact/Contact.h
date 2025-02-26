#pragma once

#include "Physics/Fixture.h"
#include "Physics/PhysicsAllocator.h"

#include <cmath>

namespace ale
{
/**
 * @file
 * @brief 물리 충돌(Contact) 관련 구조체 및 클래스 정의.
 */

/**
 * @struct ContactPolygon
 * @brief 충돌 지점을 저장하는 다각형 구조체.
 */
struct ContactPolygon
{
	alglm::vec3 points[MAX_MANIFOLD_COUNT]; /**< 충돌 지점 배열 */
	alglm::vec3 buffer[MAX_MANIFOLD_COUNT]; /**< 버퍼 */
	int32_t pointsCount;					/**< 충돌 지점 개수 */
};

/**
 * @struct Face
 * @brief 충돌 면 정보를 저장하는 구조체.
 */
struct Face
{
	alglm::vec3 normal;						  /**< 면의 법선 벡터 */
	float distance;							  /**< 면과 원점 사이 거리 */
	alglm::vec3 vertices[MAX_MANIFOLD_COUNT]; /**< 면을 구성하는 꼭짓점 */
	int32_t verticesCount;					  /**< 꼭짓점 개수 */
};

/**
 * @struct CollisionInfo
 * @brief 충돌 정보를 저장하는 구조체.
 */
struct CollisionInfo
{
	alglm::vec3 normal[MAX_MANIFOLD_COUNT]; /**< 충돌 법선 벡터 */
	alglm::vec3 pointA[MAX_MANIFOLD_COUNT]; /**< 첫 번째 충돌 지점 */
	alglm::vec3 pointB[MAX_MANIFOLD_COUNT]; /**< 두 번째 충돌 지점 */
	float seperation[MAX_MANIFOLD_COUNT];	/**< 충돌 분리 거리 */
	int32_t size;							/**< 충돌 정보 개수 */
};

const int32_t MAX_SIMPLEX_COUNT = 100;

/**
 * @struct FaceArray
 * @brief 충돌 검출 시 사용되는 면(Face) 배열.
 */

struct FaceArray
{
	int32_t count;
	int32_t maxCount;
	int32_t *faces;
	alglm::vec4 *normals;

	/**
	 * @brief FaceArray 생성자.
	 * @details 메모리를 할당하고 기본값을 설정합니다.
	 */
	FaceArray()
	{
		count = 0;
		maxCount = 64;

		void *memory = PhysicsAllocator::m_blockAllocator.allocateBlock(3 * maxCount * sizeof(int32_t));
		faces = static_cast<int32_t *>(memory);

		memory = PhysicsAllocator::m_blockAllocator.allocateBlock(maxCount * sizeof(alglm::vec4));
		normals = static_cast<alglm::vec4 *>(memory);
	}
};

/**
 * @struct UniqueEdges
 * @brief 유일한(중복되지 않은) 엣지를 저장하는 구조체.
 */
struct UniqueEdges
{
	std::pair<int32_t, int32_t> *edges;
	int32_t size;
};

/**
 * @struct Simplex
 * @brief 심플렉스(Simplex) 데이터를 저장하는 구조체.
 */
struct Simplex
{
	alglm::vec3 diff;
	alglm::vec3 a;
	alglm::vec3 b;
};

/**
 * @struct SimplexArray
 * @brief GJK 및 EPA 알고리즘에서 사용되는 심플렉스 배열.
 */
struct SimplexArray
{
	Simplex simplices[MAX_SIMPLEX_COUNT];
	int32_t simplexCount;
};

/**
 * @struct ConvexInfo
 * @brief 볼록 다면체(Convex Shape) 정보를 저장하는 구조체.
 */
struct ConvexInfo
{
	alglm::vec3 *points{nullptr};
	alglm::vec3 *axes{nullptr};
	int32_t pointsCount;
	int32_t axesCount;
	alglm::vec3 halfSize;
	alglm::vec3 center;
	float radius;
	float height;
};

/**
 * @struct EpaInfo
 * @brief Expanding Polytope Algorithm(EPA) 결과를 저장하는 구조체.
 */
struct EpaInfo
{
	alglm::vec3 normal;
	float distance;
};

class Contact;
struct Manifold;

using contactMemberFunction = Contact *(*)(Fixture *, Fixture *, int32_t, int32_t);

/**
 * @struct ContactLink
 * @brief 두 개의 Rigidbody 간 충돌을 연결하는 구조체.
 * @details ContactLink는 특정 Rigidbody가 충돌한 다른 Rigidbody 및
 *         해당 충돌(Contact)에 대한 정보를 저장합니다.
 */
struct ContactLink
{
	Rigidbody *other;  // 연결된 반대쪽 Body
	Contact *contact;  // 두 Body 간의 Contact 정보
	ContactLink *prev; // 이전 충돌 정보
	ContactLink *next; // 다음 충돌 정보

	/**
	 * @brief 기본 생성자.
	 * @details 포인터 멤버 변수를 nullptr로 초기화합니다.
	 */
	ContactLink() : other(nullptr), contact(nullptr), prev(nullptr), next(nullptr) {};
};

/**
 * @enum EContactFlag
 * @brief 충돌 플래그 열거형.
 */
enum class EContactFlag
{
	ISLAND = (1 << 0),
	TOUCHING = (1 << 2),
};

/**
 * @brief & 비트 연산자 오버로딩.
 */
int32_t operator&(int32_t val, EContactFlag flag);

/**
 * @brief | 비트 연산자 오버로딩.
 */
int32_t operator|(int32_t val, EContactFlag flag);

/**
 * @brief ~ 비트 연산자 오버로딩.
 */
int32_t operator~(EContactFlag flag);

/**
 * @brief == 연산자 오버로딩.
 */
bool operator==(int32_t val, EContactFlag flag);

/**
 * @class Contact
 * @brief 물리 엔진 내에서 두 개체 간의 충돌을 처리하는 클래스.
 */
class Contact
{
  public:
	/**
	 * @brief Contact 객체를 생성하는 정적 함수.
	 * @param fixtureA 첫 번째 개체의 Fixture.
	 * @param fixtureB 두 번째 개체의 Fixture.
	 * @param indexA 첫 번째 개체의 인덱스.
	 * @param indexB 두 번째 개체의 인덱스.
	 * @return 생성된 Contact 객체의 포인터.
	 */
	static Contact *create(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);

	/**
	 * @brief Contact 생성자.
	 * @param fixtureA 첫 번째 개체의 Fixture.
	 * @param fixtureB 두 번째 개체의 Fixture.
	 * @param indexA 첫 번째 개체의 인덱스.
	 * @param indexB 두 번째 개체의 인덱스.
	 */
	Contact(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);

	/**
	 * @brief 충돌 정보를 업데이트합니다.
	 */
	void update();

	/**
	 * @brief 충돌 평가를 수행합니다.
	 * @param manifold 충돌 정보를 저장할 Manifold 객체.
	 * @param transformA 첫 번째 개체의 변환 정보.
	 * @param transformB 두 번째 개체의 변환 정보.
	 * @param isSensor 센서 여부.
	 */
	void evaluate(Manifold &manifold, const Transform &transformA, const Transform &transformB, bool isSensor);

	/**
	 * @brief 충돌 포인트를 생성합니다.
	 * @param collisionInfo 충돌 정보를 저장할 구조체.
	 * @param manifold 충돌 정보를 저장할 Manifold 객체.
	 * @param m_fixtureA 첫 번째 개체의 Fixture.
	 * @param m_fixtureB 두 번째 개체의 Fixture.
	 */
	void generateManifolds(CollisionInfo &collisionInfo, Manifold &manifold, Fixture *m_fixtureA, Fixture *m_fixtureB);

	/**
	 * @brief 마찰 계수를 반환합니다.
	 * @return 마찰 계수.
	 */
	float getFriction() const;

	/**
	 * @brief 반발 계수를 반환합니다.
	 * @return 반발 계수.
	 */
	float getRestitution() const;

	/**
	 * @brief 첫 번째 개체의 인덱스를 반환합니다.
	 * @return 첫 번째 개체의 인덱스.
	 */
	int32_t getChildIndexA() const;

	/**
	 * @brief 두 번째 개체의 인덱스를 반환합니다.
	 * @return 두 번째 개체의 인덱스.
	 */
	int32_t getChildIndexB() const;

	/**
	 * @brief 심플렉스를 이용해 충돌면 법선을 계산합니다.
	 * @param simplexArray 심플렉스 배열.
	 * @param faceArray 충돌면 정보를 저장할 배열.
	 * @return 계산된 면의 개수.
	 */
	int32_t getFaceNormals(SimplexArray &simplexArray, FaceArray &faceArray);

	/** @brief 다음 Contact 포인터를 반환합니다. */
	Contact *getNext();

	/**
	 * @brief 주어진 방향에서 충돌 지점(Support Point)을 찾습니다.
	 * @param convexA 첫 번째 Convex 객체.
	 * @param convexB 두 번째 Convex 객체.
	 * @param dir 검색할 방향 벡터.
	 * @return Support Point 정보.
	 */
	Simplex getSupportPoint(const ConvexInfo &convexA, const ConvexInfo &convexB, alglm::vec3 &dir);

	/**
	 * @brief EPA 알고리즘을 이용하여 충돌 깊이와 법선을 계산합니다.
	 * @param convexA 첫 번째 Convex 객체.
	 * @param convexB 두 번째 Convex 객체.
	 * @param simplexArray 심플렉스 배열.
	 * @return 계산된 EpaInfo 구조체.
	 */
	EpaInfo getEpaResult(const ConvexInfo &convexA, const ConvexInfo &convexB, SimplexArray &simplexArray);

	/** @brief 첫 번째 개체의 Fixture를 반환합니다. */
	Fixture *getFixtureA() const;

	/** @brief 두 번째 개체의 Fixture를 반환합니다. */
	Fixture *getFixtureB() const;

	/** @brief 첫 번째 개체의 ContactLink를 반환합니다. */
	ContactLink *getNodeA();

	/** @brief 두 번째 개체의 ContactLink를 반환합니다. */
	ContactLink *getNodeB();

	/**
	 * @brief 충돌 정보를 저장하는 Manifold 객체를 반환합니다.
	 * @return 충돌 정보를 저장하는 Manifold 객체.
	 */
	Manifold &getManifold();

	/** @brief 이전 Contact 포인터를 설정합니다. */
	void setPrev(Contact *contact);

	/** @brief 다음 Contact 포인터를 설정합니다. */
	void setNext(Contact *contact);

	/**
	 * @brief 충돌 플래그를 설정합니다.
	 * @param flag 설정할 플래그.
	 */
	void setFlag(EContactFlag flag);

	/**
	 * @brief 충돌 플래그가 설정되어 있는지 확인합니다.
	 * @param flag 확인할 플래그.
	 * @return 설정되어 있으면 true, 그렇지 않으면 false.
	 */
	bool hasFlag(EContactFlag flag);

	/** @brief 충돌 플래그를 해제합니다. */
	void unsetFlag(EContactFlag flag);

  protected:
	static contactMemberFunction createContactFunctions[32];

	/** @brief 라인 심플렉스를 처리합니다. */
	bool handleLineSimplex(SimplexArray &simplexArray, alglm::vec3 &dir);

	/** @brief 삼각형 심플렉스를 처리합니다. */
	bool handleTriangleSimplex(SimplexArray &simplexArray, alglm::vec3 &dir);

	/** @brief 사면체 심플렉스를 처리합니다. */
	bool handleTetrahedronSimplex(SimplexArray &simplexArray, alglm::vec3 &dir);

	/** @brief 심플렉스를 처리하여 충돌을 해결합니다. */
	bool handleSimplex(SimplexArray &simplexArray, alglm::vec3 &dir);

	/** @brief GJK 알고리즘을 사용하여 충돌 여부를 판별합니다. */
	bool getGjkResult(const ConvexInfo &convexA, const ConvexInfo &convexB, SimplexArray &simplexArray);

	/** @brief 두 개의 구(Sphere) 충돌을 판별합니다. */
	bool checkSphereToSphereCollide(const ConvexInfo &convexA, const ConvexInfo &convexB);

	/** @brief 특정 지점이 기존 심플렉스에 중복되어 있는지 확인합니다. */
	bool isDuplicatedPoint(const SimplexArray &simplexArray, const alglm::vec3 &supportPoint);

	/** @brief 두 벡터가 같은 방향인지 확인합니다. */
	bool isSameDirection(alglm::vec3 v1, alglm::vec3 v2);

	/** @brief 두 벡터가 유사한 방향인지 확인합니다. */
	bool isSimilarDirection(alglm::vec3 v1, alglm::vec3 v2);

	/** @brief 유니크한 엣지만 추가합니다. */
	void addIfUniqueEdge(UniqueEdges &uniqueEdges, const int32_t *faces, int32_t p1, int32_t p2);

	/**
	 * @brief 충돌체 A의 Support Function을 계산하는 가상 함수.
	 * @param convexA 충돌체 A의 ConvexInfo.
	 * @param dir 검색할 방향 벡터.
	 * @return 해당 방향에서의 Support 점 (alglm::vec3).
	 */
	virtual alglm::vec3 supportA(const ConvexInfo &convexA, alglm::vec3 dir) = 0;

	/**
	 * @brief 충돌체 B의 Support Function을 계산하는 가상 함수.
	 * @param convexB 충돌체 B의 ConvexInfo.
	 * @param dir 검색할 방향 벡터.
	 * @return 해당 방향에서의 Support 점 (alglm::vec3).
	 */
	virtual alglm::vec3 supportB(const ConvexInfo &convexB, alglm::vec3 dir) = 0;

	/**
	 * @brief 두 개체 간의 충돌 포인트를 계산하는 가상 함수.
	 * @param convexA 첫 번째 개체의 ConvexInfo.
	 * @param convexB 두 번째 개체의 ConvexInfo.
	 * @param collisionInfo 충돌 정보를 저장할 구조체.
	 * @param epaInfo EPA 정보를 저장할 구조체.
	 * @param simplexArray 심플렉스 배열.
	 */
	virtual void findCollisionPoints(const ConvexInfo &convexA, const ConvexInfo &convexB, CollisionInfo &collisionInfo,
									 EpaInfo &epaInfo, SimplexArray &simplexArray) = 0;

	/**
	 * @brief 충돌 폴리곤을 계산합니다.
	 * @param contactPolygon 생성된 충돌 폴리곤을 저장할 구조체.
	 * @param refFace 기준이 되는 면(Face).
	 * @param incFace 충돌하는 면(Face).
	 */
	void computeContactPolygon(ContactPolygon &contactPolygon, Face &refFace, Face &incFace);

	/**
	 * @brief 평면을 기준으로 폴리곤을 클리핑(잘라내기)합니다.
	 * @param contactPolygon 클리핑할 폴리곤.
	 * @param planeNormal 기준 평면의 법선 벡터.
	 * @param planeDist 기준 평면의 거리.
	 */
	void clipPolygonAgainstPlane(ContactPolygon &contactPolygon, const alglm::vec3 &planeNormal, float planeDist);

	/**
	 * @brief 폴리곤을 기반으로 충돌 Manifold를 생성합니다.
	 * @param collisionInfo 충돌 정보를 저장할 구조체.
	 * @param refFace 기준이 되는 면(Face).
	 * @param incFace 충돌하는 면(Face).
	 * @param contactPolygon 충돌 폴리곤.
	 * @param epaInfo EPA 알고리즘을 통해 계산된 충돌 정보.
	 */
	void buildManifoldFromPolygon(CollisionInfo &collisionInfo, const Face &refFace, const Face &incFace,
								  ContactPolygon &contactPolygon, EpaInfo &epaInfo);

	/**
	 * @brief 주어진 법선 방향을 기준으로 정점(Vertex)들을 시계 방향으로 정렬합니다.
	 * @param vertices 정렬할 정점 배열.
	 * @param center 정점들이 속한 중심점.
	 * @param normal 기준이 되는 법선 벡터.
	 * @param verticesSize 정점 개수.
	 */
	void sortVerticesClockwise(alglm::vec3 *vertices, const alglm::vec3 &center, const alglm::vec3 &normal,
							   int32_t verticesSize);

	/**
	 * @brief 박스(상자) 형태의 Face를 설정합니다.
	 * @param face 설정할 Face.
	 * @param box 기준이 되는 ConvexInfo(Box 형태).
	 * @param normal 기준이 되는 법선 벡터.
	 */
	void setBoxFace(Face &face, const ConvexInfo &box, const alglm::vec3 &normal);

	/**
	 * @brief 실린더 형태의 Face를 설정합니다.
	 * @param face 설정할 Face.
	 * @param cylinder 기준이 되는 ConvexInfo(실린더 형태).
	 * @param normal 기준이 되는 법선 벡터.
	 */
	void setCylinderFace(Face &face, const ConvexInfo &cylinder, const alglm::vec3 &normal);

	/**
	 * @brief 캡슐 형태의 Face를 설정합니다.
	 * @param face 설정할 Face.
	 * @param capsule 기준이 되는 ConvexInfo(캡슐 형태).
	 * @param normal 기준이 되는 법선 벡터.
	 */
	void setCapsuleFace(Face &face, const ConvexInfo &capsule, const alglm::vec3 &normal);

	/**
	 * @brief 캡슐과 반구가 충돌하는지 검사합니다.
	 * @param capsule 검사할 캡슐 ConvexInfo.
	 * @param dir 검사할 방향 벡터.
	 * @return 충돌하면 true, 그렇지 않으면 false.
	 */
	bool isCollideToHemisphere(const ConvexInfo &capsule, const alglm::vec3 &dir);

	/**
	 * @brief FaceArray에 새로운 Face를 추가합니다.
	 * @param faceArray 추가할 FaceArray.
	 * @param idx1 첫 번째 정점의 인덱스.
	 * @param idx2 두 번째 정점의 인덱스.
	 * @param idx3 세 번째 정점의 인덱스.
	 */
	void addFaceInFaceArray(FaceArray &faceArray, int32_t idx1, int32_t idx2, int32_t idx3);

	/**
	 * @brief 두 개의 FaceArray를 병합합니다.
	 * @param faceArray 병합될 FaceArray.
	 * @param newFaceArray 새롭게 추가될 FaceArray.
	 */
	void mergeFaceArray(FaceArray &faceArray, FaceArray &newFaceArray);

	/**
	 * @brief FaceArray의 크기를 증가시킵니다.
	 * @param faceArray 크기를 늘릴 FaceArray.
	 * @param newMaxCount 새로운 최대 크기.
	 */
	void sizeUpFaceArray(FaceArray &faceArray, int32_t newMaxCount);

	/**
	 * @brief ConvexInfo에 할당된 메모리를 해제합니다.
	 * @param convexA 해제할 첫 번째 ConvexInfo.
	 * @param convexB 해제할 두 번째 ConvexInfo.
	 */
	void freeConvexInfo(ConvexInfo &convexA, ConvexInfo &convexB);

	bool m_wasTouched;
	float m_friction;
	float m_restitution;
	int32_t m_flags;
	Contact *m_prev;
	Contact *m_next;
	ContactLink m_nodeA;
	ContactLink m_nodeB;
	Fixture *m_fixtureA;
	Fixture *m_fixtureB;
	int32_t m_indexA;
	int32_t m_indexB;
	Manifold m_manifold;
};
} // namespace ale