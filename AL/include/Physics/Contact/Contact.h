#pragma once

#include "Physics/Fixture.h"
#include "Physics/PhysicsAllocator.h"

#include <cmath>

namespace ale
{
struct ContactPolygon
{
	alglm::vec3 points[MAX_MANIFOLD_COUNT];
	alglm::vec3 buffer[MAX_MANIFOLD_COUNT];
	int32_t pointsCount;
};

struct Face
{
	alglm::vec3 normal;
	float distance;
	alglm::vec3 vertices[MAX_MANIFOLD_COUNT];
	int32_t verticesCount;
};

struct CollisionInfo
{
	alglm::vec3 normal[MAX_MANIFOLD_COUNT];
	alglm::vec3 pointA[MAX_MANIFOLD_COUNT];
	alglm::vec3 pointB[MAX_MANIFOLD_COUNT];
	float seperation[MAX_MANIFOLD_COUNT];
	int32_t size;
};

const int32_t MAX_SIMPLEX_COUNT = 100;

struct FaceArray
{
	int32_t count;
	int32_t maxCount;
	int32_t *faces;
	alglm::vec4 *normals;

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

struct UniqueEdges
{
	std::pair<int32_t, int32_t> *edges;
	int32_t size;
};

struct Simplex
{
	alglm::vec3 diff;
	alglm::vec3 a;
	alglm::vec3 b;
};

struct SimplexArray
{
	Simplex simplices[MAX_SIMPLEX_COUNT];
	int32_t simplexCount;
};

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

struct EpaInfo
{
	alglm::vec3 normal;
	float distance;
};

class Contact;
struct Manifold;

using contactMemberFunction = Contact *(*)(Fixture *, Fixture *, int32_t, int32_t);

struct ContactLink
{
	Rigidbody *other;  // 연결된 반대쪽 Body
	Contact *contact;  // 두 Body 간의 Contact 정보
	ContactLink *prev; // 이전 충돌 정보
	ContactLink *next; // 다음 충돌 정보

	ContactLink() : other(nullptr), contact(nullptr), prev(nullptr), next(nullptr) {};
};

enum class EContactFlag
{
	ISLAND = (1 << 0),
	TOUCHING = (1 << 2),
};

int32_t operator&(int32_t val, EContactFlag flag);
int32_t operator|(int32_t val, EContactFlag flag);
int32_t operator~(EContactFlag flag);
bool operator==(int32_t val, EContactFlag flag);

class Contact
{
  public:
	static Contact *create(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);

	Contact(Fixture *fixtureA, Fixture *fixtureB, int32_t indexA, int32_t indexB);
	void update();
	void evaluate(Manifold &manifold, const Transform &transformA, const Transform &transformB);

	void generateManifolds(CollisionInfo &collisionInfo, Manifold &manifold, Fixture *m_fixtureA, Fixture *m_fixtureB);
	float getFriction() const;
	float getRestitution() const;
	int32_t getChildIndexA() const;
	int32_t getChildIndexB() const;
	int32_t getFaceNormals(SimplexArray &simplexArray, FaceArray &faceArray);
	Contact *getNext();
	Simplex getSupportPoint(const ConvexInfo &convexA, const ConvexInfo &convexB, alglm::vec3 &dir);
	EpaInfo getEpaResult(const ConvexInfo &convexA, const ConvexInfo &convexB, SimplexArray &simplexArray);
	Fixture *getFixtureA() const;
	Fixture *getFixtureB() const;
	ContactLink *getNodeA();
	ContactLink *getNodeB();
	Manifold &getManifold();

	void setPrev(Contact *contact);
	void setNext(Contact *contact);
	void setFlag(EContactFlag flag);
	bool hasFlag(EContactFlag flag);
	void unsetFlag(EContactFlag flag);

  protected:
	static contactMemberFunction createContactFunctions[32];

	bool handleLineSimplex(SimplexArray &simplexArray, alglm::vec3 &dir);
	bool handleTriangleSimplex(SimplexArray &simplexArray, alglm::vec3 &dir);
	bool handleTetrahedronSimplex(SimplexArray &simplexArray, alglm::vec3 &dir);
	bool handleSimplex(SimplexArray &simplexArray, alglm::vec3 &dir);
	bool getGjkResult(const ConvexInfo &convexA, const ConvexInfo &convexB, SimplexArray &simplexArray);
	bool checkSphereToSphereCollide(const ConvexInfo &convexA, const ConvexInfo &convexB);
	bool isDuplicatedPoint(const SimplexArray &simplexArray, const alglm::vec3 &supportPoint);
	bool isSameDirection(alglm::vec3 v1, alglm::vec3 v2);
	bool isSimilarDirection(alglm::vec3 v1, alglm::vec3 v2);
	void addIfUniqueEdge(UniqueEdges &uniqueEdges, const int32_t *faces, int32_t p1, int32_t p2);

	virtual alglm::vec3 supportA(const ConvexInfo &convexA, alglm::vec3 dir) = 0;
	virtual alglm::vec3 supportB(const ConvexInfo &convexB, alglm::vec3 dir) = 0;
	virtual void findCollisionPoints(const ConvexInfo &convexA, const ConvexInfo &convexB, CollisionInfo &collisionInfo,
									 EpaInfo &epaInfo, SimplexArray &simplexArray) = 0;

	void computeContactPolygon(ContactPolygon &contactPolygon, Face &refFace, Face &incFace);
	void clipPolygonAgainstPlane(ContactPolygon &contactPolygon, const alglm::vec3 &planeNormal, float planeDist);

	void buildManifoldFromPolygon(CollisionInfo &collisionInfo, const Face &refFace, const Face &incFace,
								  ContactPolygon &contactPolygon, EpaInfo &epaInfo);
	void sortVerticesClockwise(alglm::vec3 *vertices, const alglm::vec3 &center, const alglm::vec3 &normal,
							   int32_t verticesSize);

	void setBoxFace(Face &face, const ConvexInfo &box, const alglm::vec3 &normal);
	void setCylinderFace(Face &face, const ConvexInfo &cylinder, const alglm::vec3 &normal);
	void setCapsuleFace(Face &face, const ConvexInfo &capsule, const alglm::vec3 &normal);

	bool isCollideToHemisphere(const ConvexInfo &capsule, const alglm::vec3 &dir);

	void addFaceInFaceArray(FaceArray &faceArray, int32_t idx1, int32_t idx2, int32_t idx3);
	void mergeFaceArray(FaceArray &faceArray, FaceArray &newFaceArray);
	void sizeUpFaceArray(FaceArray &faceArray, int32_t newMaxCount);
	void freeConvexInfo(ConvexInfo &convexA, ConvexInfo &convexB);

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