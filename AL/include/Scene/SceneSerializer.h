#pragma once

#include "Scene/Scene.h"
#include <unordered_map>

namespace ale
{
/**
 * @struct RelationshipData
 * @brief 개체(Entity) 간의 부모-자식 관계 정보를 저장하는 구조체.
 */
struct RelationshipData
{
	uint64_t entityUUID;
	uint64_t parentUUID;
	std::vector<uint64_t> childrenUUIDs;
};

/**
 * @class SceneSerializer
 * @brief 씬(Scene) 데이터를 파일로 저장(직렬화) 및 불러오기(역직렬화)하는 클래스.
 */
class SceneSerializer
{
  public:
	/**
	 * @brief SceneSerializer 생성자.
	 * @param scene 직렬화할 씬 객체의 공유 포인터.
	 */
	SceneSerializer(const std::shared_ptr<Scene> &scene);

	/**
	 * @brief 씬을 지정된 파일 경로에 직렬화하여 저장합니다.
	 * @param filepath 저장할 파일의 경로.
	 */
	void serialize(const std::string &filepath);

	/**
	 * @brief 씬을 런타임 형식으로 직렬화하여 저장합니다.
	 * @param filepath 저장할 파일의 경로.
	 */
	void serializeRuntime(const std::string &filepath);

	/**
	 * @brief 지정된 파일 경로에서 씬 데이터를 역직렬화(로드)합니다.
	 * @param filepath 로드할 파일의 경로.
	 * @return true 역직렬화 성공.
	 * @return false 역직렬화 실패.
	 */
	bool deserialize(const std::string &filepath);

	/**
	 * @brief 런타임 형식의 씬 데이터를 역직렬화(로드)합니다.
	 * @param filepath 로드할 파일의 경로.
	 * @return true 역직렬화 성공.
	 * @return false 역직렬화 실패.
	 */
	bool deserializeRuntime(const std::string &filepath);

  private:
	std::shared_ptr<Scene> m_Scene;
	std::unordered_map<uint64_t, RelationshipData> relationshipMap;
};

} // namespace ale