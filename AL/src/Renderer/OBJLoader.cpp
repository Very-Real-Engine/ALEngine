#include "Renderer/OBJLoader.h"
#include <fstream>
#include <iostream>
#include <sstream>

namespace ale
{

std::unique_ptr<OBJLoader> OBJLoader::ReadFile(const std::string &path)
{
	std::unique_ptr<OBJLoader> objLoader = std::unique_ptr<OBJLoader>(new OBJLoader());
	objLoader->parse(path);
	return objLoader;
}

void OBJLoader::parse(const std::string &path)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << path << std::endl;
		return;
	}

	std::string currentMaterial = "";
	int32_t currentSmoothingGroup = 0;
	std::string line;

	auto getVertexIndex = [&](std::string &vertex, int32_t &vIndex, int32_t &tIndex, int32_t &nIndex) -> bool {
		std::stringstream vss(vertex);
		std::string vIdx, tIdx, nIdx;
		std::getline(vss, vIdx, '/');
		std::getline(vss, tIdx, '/');
		std::getline(vss, nIdx, '/');

		for (auto &c : vIdx)
		{
			if (!std::isdigit(c))
				return false;
		}
		for (auto &c : tIdx)
		{
			if (!std::isdigit(c))
				return false;
		}
		for (auto &c : nIdx)
		{
			if (!std::isdigit(c))
				return false;
		}
		vIndex = std::stoi(vIdx) - 1;
		tIndex = tIdx.empty() ? -1 : std::stoi(tIdx) - 1;
		nIndex = nIdx.empty() ? -1 : std::stoi(nIdx) - 1;

		if (vIndex >= static_cast<int32_t>(globalPosition.size()) ||
			tIndex >= static_cast<int32_t>(globalTexCoord.size()) ||
			nIndex >= static_cast<int32_t>(globalNormal.size()))
		{
			return false;
		}
		return true;
	};

	auto makeKey = [&](int32_t vIndex, int32_t tIndex, int32_t nIndex, int32_t smoothingGroup) -> std::string {
		return std::to_string(vIndex) + "/" + std::to_string(tIndex) + "/" + std::to_string(nIndex) + "/" +
			   std::to_string(smoothingGroup);
	};

	auto getVertex = [&](int32_t vIndex, int32_t tIndex, int32_t nIndex, int32_t smoothingGroup) -> Vertex {
		Vertex tmp;
		if (vIndex >= 0 && vIndex < static_cast<int32_t>(globalPosition.size()))
		{
			tmp.pos = globalPosition[vIndex];
		}
		else
		{
			tmp.pos = glm::vec3(0.0f);
		}

		if (nIndex >= 0 && nIndex < static_cast<int32_t>(globalNormal.size()))
		{
			tmp.normal = globalNormal[nIndex];
		}
		else
		{
			tmp.normal = glm::vec3(0.0f);
		}

		if (tIndex >= 0 && tIndex < static_cast<int32_t>(globalTexCoord.size()))
		{
			tmp.texCoord = globalTexCoord[tIndex];
		}
		else
		{
			tmp.texCoord = glm::vec2(0.0f);
		}

		return tmp;
	};

	while (std::getline(file, line))
	{
		std::stringstream ss(line);
		std::string type;
		ss >> type;
		if (type == "f")
		{
			if (subMeshMap.find(currentMaterial) == subMeshMap.end())
			{
				subMeshMap[currentMaterial] = SubMesh();
			}
			SubMesh &subMesh = subMeshMap[currentMaterial];

			if (currentSmoothingGroup != 0)
			{
				std::string vertex;
				ss >> vertex;
				int32_t vIndex, tIndex, nIndex;
				if (!getVertexIndex(vertex, vIndex, tIndex, nIndex))
				{
					std::cerr << "Failed to get vertex index" << std::endl;
					return;
				}
				std::string key = makeKey(vIndex, tIndex, nIndex, currentSmoothingGroup);
				int32_t firstVertexIndex;
				if (vertexCache.find(key) == vertexCache.end())
				{
					vertexCache[key] = subMesh.vertices.size();
					firstVertexIndex = vertexCache[key];
					subMesh.vertices.push_back(getVertex(vIndex, tIndex, nIndex, currentSmoothingGroup));
					subMesh.indices.push_back(firstVertexIndex);
				}
				else
				{
					firstVertexIndex = vertexCache[key];
					subMesh.indices.push_back(firstVertexIndex);
				}

				ss >> vertex;
				if (!getVertexIndex(vertex, vIndex, tIndex, nIndex))
				{
					std::cerr << "Failed to get vertex index" << std::endl;
					return;
				}
				key = makeKey(vIndex, tIndex, nIndex, currentSmoothingGroup);
				if (vertexCache.find(key) == vertexCache.end())
				{
					vertexCache[key] = subMesh.vertices.size();
					subMesh.vertices.push_back(getVertex(vIndex, tIndex, nIndex, currentSmoothingGroup));
					subMesh.indices.push_back(subMesh.vertices.size() - 1);
				}
				else
				{
					subMesh.indices.push_back(vertexCache[key]);
				}

				ss >> vertex;
				if (!getVertexIndex(vertex, vIndex, tIndex, nIndex))
				{
					std::cerr << "Failed to get vertex index" << std::endl;
					return;
				}
				key = makeKey(vIndex, tIndex, nIndex, currentSmoothingGroup);
				int32_t pastVertexIndex;
				if (vertexCache.find(key) == vertexCache.end())
				{
					vertexCache[key] = subMesh.vertices.size();
					pastVertexIndex = vertexCache[key];
					subMesh.vertices.push_back(getVertex(vIndex, tIndex, nIndex, currentSmoothingGroup));
					subMesh.indices.push_back(pastVertexIndex);
				}
				else
				{
					pastVertexIndex = vertexCache[key];
					subMesh.indices.push_back(pastVertexIndex);
				}
				int32_t currentVertexIndex;
				while (ss >> vertex)
				{
					if (!getVertexIndex(vertex, vIndex, tIndex, nIndex))
					{
						std::cerr << "Failed to get vertex index" << std::endl;
						return;
					}
					key = makeKey(vIndex, tIndex, nIndex, currentSmoothingGroup);
					if (vertexCache.find(key) == vertexCache.end())
					{
						vertexCache[key] = subMesh.vertices.size();
					}
					currentVertexIndex = vertexCache[key];
					subMesh.vertices.push_back(getVertex(vIndex, tIndex, nIndex, currentSmoothingGroup));
					subMesh.indices.push_back(firstVertexIndex);
					subMesh.indices.push_back(pastVertexIndex);
					subMesh.indices.push_back(currentVertexIndex);
					pastVertexIndex = currentVertexIndex;
				}
			}
			else
			{
				std::string vertex;
				ss >> vertex;
				int32_t vIndex, tIndex, nIndex;
				if (!getVertexIndex(vertex, vIndex, tIndex, nIndex))
				{
					std::cerr << "Failed to get vertex index" << std::endl;
					return;
				}
				Vertex firstVertex = getVertex(vIndex, tIndex, nIndex, currentSmoothingGroup);
				subMesh.vertices.push_back(firstVertex);
				subMesh.indices.push_back(subMesh.vertices.size() - 1);

				ss >> vertex;
				if (!getVertexIndex(vertex, vIndex, tIndex, nIndex))
				{
					std::cerr << "Failed to get vertex index" << std::endl;
					return;
				}
				Vertex secondVertex = getVertex(vIndex, tIndex, nIndex, currentSmoothingGroup);
				subMesh.vertices.push_back(secondVertex);
				subMesh.indices.push_back(subMesh.vertices.size() - 1);

				ss >> vertex;
				if (!getVertexIndex(vertex, vIndex, tIndex, nIndex))
				{
					std::cerr << "Failed to get vertex index" << std::endl;
					return;
				}
				Vertex pastVertex = getVertex(vIndex, tIndex, nIndex, currentSmoothingGroup);
				subMesh.vertices.push_back(pastVertex);
				subMesh.indices.push_back(subMesh.vertices.size() - 1);

				Vertex currentVertex;
				while (ss >> vertex)
				{
					if (!getVertexIndex(vertex, vIndex, tIndex, nIndex))
					{
						std::cerr << "Failed to get vertex index" << std::endl;
						return;
					}
					currentVertex = getVertex(vIndex, tIndex, nIndex, currentSmoothingGroup);
					subMesh.vertices.push_back(firstVertex);
					subMesh.vertices.push_back(pastVertex);
					subMesh.vertices.push_back(currentVertex);
					subMesh.indices.push_back(subMesh.vertices.size() - 1);
					subMesh.indices.push_back(subMesh.vertices.size() - 2);
					subMesh.indices.push_back(subMesh.vertices.size() - 3);
					pastVertex = currentVertex;
				}
			}
		}
		else
		{
			if (type == "usemtl")
			{
				ss >> currentMaterial;
			}
			else if (type == "s")
			{
				std::string smoothingGroup;
				ss >> smoothingGroup;
				if (smoothingGroup == "off")
				{
					currentSmoothingGroup = 0;
				}
				else
				{
					for (auto &c : smoothingGroup)
					{
						if (!std::isdigit(c))
							return;
					}
					currentSmoothingGroup = std::stoi(smoothingGroup);
				}
			}
			else if (type == "v")
			{
				glm::vec3 position;
				if (!(ss >> position.x >> position.y >> position.z))
				{
					std::cerr << "Failed to read position" << std::endl;
					return;
				}
				globalPosition.push_back(position);
			}
			else if (type == "vn")
			{
				glm::vec3 normal;
				if (!(ss >> normal.x >> normal.y >> normal.z))
				{
					std::cerr << "Failed to read normal" << std::endl;
					return;
				}
				globalNormal.push_back(normal);
			}
			else if (type == "vt")
			{
				glm::vec2 texCoord;
				if (!(ss >> texCoord.x >> texCoord.y))
				{
					std::cerr << "Failed to read texCoord" << std::endl;
					return;
				}
				globalTexCoord.push_back(texCoord);
			}
			else if (type == "mtllib")
			{
				std::string mtlFile;
				ss >> mtlFile;
				mtlFile = getMTLPath(path, mtlFile);
				parseMTL(mtlFile);
			}
		}
	}
	file.close();
	flag = true;
}

void OBJLoader::parseMTL(const std::string &path)
{
	std::ifstream file(path);
	if (!file.is_open())
	{
		std::cerr << "Failed to open file: " << path << std::endl;
		return;
	}

	std::string line;
	std::string currentMtlName;
	while (std::getline(file, line))
	{
		std::stringstream ss(line);
		std::string type;
		ss >> type;
		if (type == "newmtl")
		{
			ss >> currentMtlName;
			mtlMap[currentMtlName] = MTL();
		}
		else if (type == "Ka")
		{
			glm::vec3 Ka;
			if (!(ss >> Ka.x >> Ka.y >> Ka.z))
			{
				std::cerr << "Failed to read Ka" << std::endl;
				return;
			}
			mtlMap[currentMtlName].Ka = Ka;
		}
		else if (type == "Kd")
		{
			glm::vec3 Kd;
			if (!(ss >> Kd.x >> Kd.y >> Kd.z))
			{
				std::cerr << "Failed to read Kd" << std::endl;
				return;
			}
			mtlMap[currentMtlName].Kd = Kd;
		}
		else if (type == "Ks")
		{
			glm::vec3 Ks;
			if (!(ss >> Ks.x >> Ks.y >> Ks.z))
			{
				std::cerr << "Failed to read Ks" << std::endl;
				return;
			}
			mtlMap[currentMtlName].Ks = Ks;
		}
		else if (type == "Ns")
		{
			float Ns;
			if (!(ss >> Ns))
			{
				std::cerr << "Failed to read Ns" << std::endl;
				return;
			}
			mtlMap[currentMtlName].Ns = Ns;
		}
		else if (type == "Ni")
		{
			float Ni;
			if (!(ss >> Ni))
			{
				std::cerr << "Failed to read Ni" << std::endl;
				return;
			}
			mtlMap[currentMtlName].Ni = Ni;
		}
		else if (type == "d")
		{
			float d;
			if (!(ss >> d))
			{
				std::cerr << "Failed to read d" << std::endl;
				return;
			}
			mtlMap[currentMtlName].d = d;
		}
		else if (type == "illum")
		{
			int32_t illum;
			if (!(ss >> illum))
			{
				std::cerr << "Failed to read illum" << std::endl;
				return;
			}
			mtlMap[currentMtlName].illum = illum;
		}
		else if (type == "map_Kd")
		{
			std::string map_Kd;
			ss >> map_Kd;
			map_Kd = getMTLPath(path, map_Kd);
			mtlMap[currentMtlName].map_Kd = map_Kd;
		}
		else if (type == "map_Ks")
		{
			std::string map_Ks;
			ss >> map_Ks;
			map_Ks = getMTLPath(path, map_Ks);
			mtlMap[currentMtlName].map_Ks = map_Ks;
		}
		else if (type == "map_Ns")
		{
			std::string map_Ns;
			ss >> map_Ns;
			map_Ns = getMTLPath(path, map_Ns);
			mtlMap[currentMtlName].map_Ns = map_Ns;
		}
		else if (type == "map_Bump" || type == "bump")
		{
			std::string map_Bump;
			ss >> map_Bump;
			map_Bump = getMTLPath(path, map_Bump);
			mtlMap[currentMtlName].map_Bump = map_Bump;
		}
		else if (type == "map_d")
		{
			std::string map_d;
			ss >> map_d;
			map_d = getMTLPath(path, map_d);
			mtlMap[currentMtlName].map_d = map_d;
		}
		else if (type == "disp")
		{
			std::string disp;
			ss >> disp;
			disp = getMTLPath(path, disp);
			mtlMap[currentMtlName].disp = disp;
		}
		else if (type == "map_Ao")
		{
			std::string map_Ao;
			ss >> map_Ao;
			map_Ao = getMTLPath(path, map_Ao);
			mtlMap[currentMtlName].map_Ao = map_Ao;
		}
	}
}

std::string OBJLoader::getMTLPath(std::string path, std::string mtlPath)
{
	size_t lastSlashPos = path.find_last_of('/');
	size_t lastBlackSlashPos = path.find_last_of('\\');
	size_t lastPos = std::max(lastSlashPos, lastBlackSlashPos);

	return path.substr(0, lastPos + 1) + mtlPath;
}

} // namespace ale