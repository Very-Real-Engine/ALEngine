#ifndef OBJLOADER_H
#define OBJLOADER_H

#include "Core/Base.h"
#include "Renderer/Common.h"
#include <map>
#include <unordered_map>

namespace ale
{

struct SubMesh
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};

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

class OBJLoader
{
  public:
	static std::unique_ptr<OBJLoader> ReadFile(const std::string &path);
	std::map<std::string, SubMesh> &getSubMesh()
	{
		return subMeshMap;
	}
	bool getFlag()
	{
		return flag;
	}
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

	void parse(const std::string &path);
	void parseMTL(const std::string &path);
	std::string getMTLPath(std::string path, std::string mtlPath);
};

} // namespace ale

#endif