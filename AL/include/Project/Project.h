#pragma once

#include <filesystem>
#include <string>

namespace ale
{
/**
 * @struct ProjectConfig
 * @brief 프로젝트의 구성 정보를 저장하는 구조체.
 */
struct ProjectConfig
{
	std::string m_Name = "Untitled";

	std::filesystem::path m_ProjectPath;
	std::filesystem::path m_StartScene;

	std::filesystem::path m_AssetDirectory;
	std::filesystem::path m_ScriptModulePath;
	std::filesystem::path m_ScriptCorePath;
};

/**
 * @class Project
 * @brief 프로젝트를 관리하는 클래스.
 */
class Project
{
  public:
	/**
	 * @brief 현재 활성화된 프로젝트의 디렉터리를 반환합니다.
	 * @return const std::filesystem::path& 프로젝트 디렉터리 경로.
	 */
	static const std::filesystem::path &getProjectDirectory()
	{
		// ASSERT
		return s_ActiveProject->m_ProjectDirectory;
	}

	/**
	 * @brief 현재 활성화된 프로젝트의 에셋 디렉터리를 반환합니다.
	 * @return std::filesystem::path 에셋 디렉터리 경로.
	 */
	static std::filesystem::path getAssetDirectory()
	{
		// ASSERT
		return getProjectDirectory() / s_ActiveProject->m_Config.m_AssetDirectory;
	}

	/**
	 * @brief 주어진 상대 경로를 프로젝트의 에셋 파일 시스템 경로로 변환합니다.
	 * @param path 에셋의 상대 경로.
	 * @return std::filesystem::path 변환된 전체 파일 시스템 경로.
	 */
	static std::filesystem::path getAssetFileSystemPath(const std::filesystem::path &path)
	{
		// ASSERT
		return getAssetDirectory() / path;
	}

	/**
	 * @brief 현재 프로젝트의 구성 정보를 반환합니다.
	 * @return ProjectConfig& 프로젝트 구성 정보.
	 */
	ProjectConfig &getConfig()
	{
		return m_Config;
	}

	/**
	 * @brief 현재 활성화된 프로젝트를 반환합니다.
	 * @return std::shared_ptr<Project> 활성 프로젝트의 공유 포인터.
	 */
	static std::shared_ptr<Project> getActive()
	{
		return s_ActiveProject;
	}

	/**
	 * @brief 새로운 프로젝트를 생성합니다.
	 * @return std::shared_ptr<Project> 새롭게 생성된 프로젝트 객체.
	 */
	static std::shared_ptr<Project> create();
	
	/**
	 * @brief 주어진 경로에서 프로젝트를 로드합니다.
	 * @param path 로드할 프로젝트 파일의 경로.
	 * @return std::shared_ptr<Project> 로드된 프로젝트 객체.
	 */
	static std::shared_ptr<Project> load(const std::filesystem::path &path);
	
	/**
	 * @brief 현재 활성화된 프로젝트를 지정된 경로에 저장합니다.
	 * @param path 저장할 파일의 경로.
	 * @return true 저장 성공.
	 * @return false 저장 실패.
	 */
	static bool saveActive(const std::filesystem::path &path);

  private:
	ProjectConfig m_Config;
	std::filesystem::path m_ProjectDirectory;

	inline static std::shared_ptr<Project> s_ActiveProject;
};
} // namespace ale