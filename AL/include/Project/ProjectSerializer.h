#pragma once

#include "Project/Project.h"

namespace ale
{
/**
 * @class ProjectSerializer
 * @brief 프로젝트 데이터를 파일로 저장하고 불러오는 직렬화 클래스.
 */
class ProjectSerializer
{
  public:
	/**
	 * @brief ProjectSerializer 생성자.
	 * @param project 직렬화할 프로젝트 객체의 공유 포인터.
	 */
	ProjectSerializer(std::shared_ptr<Project> project);

	/**
	 * @brief 프로젝트 데이터를 지정된 경로에 저장(직렬화)합니다.
	 * @param filepath 저장할 파일의 경로.
	 * @return true 직렬화 성공.
	 * @return false 직렬화 실패.
	 */
	bool serialize(const std::filesystem::path &filepath);
	
	/**
	 * @brief 지정된 경로에서 프로젝트 데이터를 불러옴(역직렬화).
	 * @param filepath 불러올 파일의 경로.
	 * @return true 역직렬화 성공.
	 * @return false 역직렬화 실패.
	 */
	bool deserialize(const std::filesystem::path &filepath);

  private:
	std::shared_ptr<Project> m_Project;
};
} // namespace ale