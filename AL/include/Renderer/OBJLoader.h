#pragma once

/**
 * @file OBJLoader.h
 * @brief OBJ 파일 로더
 */

#include "Core/Base.h"
#include "Renderer/Common.h"
#include <map>
#include <unordered_map>

namespace ale
{
/**
 * @struct SubMesh
 * @brief 서브 메시
 */
struct SubMesh
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};

/**
 * @struct MTL
 * @brief MTL 파일
 */
struct MTL
{
	alglm::vec3 Ka = alglm::vec3(0.0f);
	alglm::vec3 Kd = alglm::vec3(0.0f);
	alglm::vec3 Ks = alglm::vec3(0.0f);
	float Ns = 0.0f;
	float Ni = 0.0f;
	float d = 0.0f;
	int32_t illum = 0;
	std::string map_Kd = "";
	std::string map_Ks = "";
	std::string map_Ns = "";
	std::string map_Bump = "";
	std::string map_d = "";
	std::string disp = "";
	std::string map_Ao = "";
};

/**
 * @class OBJLoader
 * @brief OBJ 파일 로더
 */
class OBJLoader
{
  public:
	/**
	 * @brief OBJ 파일 로더
	 * @param path 파일 경로
	 * @return std::unique_ptr<OBJLoader> OBJ 파일 로더
	 */
	static std::unique_ptr<OBJLoader> ReadFile(const std::string &path);
	/**
	 * @brief 서브 메시 반환
	 * @return std::map<std::string, SubMesh> & 서브 메시
	 */
	std::map<std::string, SubMesh> &getSubMesh()
	{
		return subMeshMap;
	}
	/**
	 * @brief 플래그 반환
	 * @return bool 플래그
	 */
	bool getFlag()
	{
		return flag;
	}
	/**
	 * @brief MTL 맵 반환
	 * @return std::unordered_map<std::string, MTL> & MTL 맵
	 */
	std::unordered_map<std::string, MTL> &getMtlMap()
	{
		return mtlMap;
	}

  private:
	bool flag = false;

	std::unordered_map<std::string, int32_t> vertexCache;
	std::vector<alglm::vec3> globalPosition;
	std::vector<alglm::vec3> globalNormal;
	std::vector<alglm::vec2> globalTexCoord;
	std::map<std::string, SubMesh> subMeshMap;
	std::unordered_map<std::string, MTL> mtlMap;

	/**
	 * @brief 파싱
	 * @param path 파일 경로
	 */
	void parse(const std::string &path);
	/**
	 * @brief MTL 파싱
	 * @param path 파일 경로
	 */
	void parseMTL(const std::string &path);
	/**
	 * @brief MTL 경로 반환
	 * @param path 파일 경로
	 * @param mtlPath MTL 경로
	 * @return std::string MTL 경로
	 */
	std::string getMTLPath(std::string path, std::string mtlPath);
};

} // namespace ale
