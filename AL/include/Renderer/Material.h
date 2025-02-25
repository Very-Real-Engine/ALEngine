#pragma once

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/Texture.h"

namespace ale
{
struct Albedo
{
	alglm::vec3 albedo = alglm::vec3(1.0f);
	std::shared_ptr<Texture> albedoTexture = nullptr;
	bool flag = false;
};

struct NormalMap
{
	std::shared_ptr<Texture> normalTexture = nullptr;
	bool flag = false;
};

struct Roughness
{
	float roughness = 0.5f;
	std::shared_ptr<Texture> roughnessTexture = nullptr;
	bool flag = false;
};

struct Metallic
{
	float metallic = 0.0f;
	std::shared_ptr<Texture> metallicTexture = nullptr;
	bool flag = false;
};

struct AOMap
{
	float ao = 1.0f;
	std::shared_ptr<Texture> aoTexture = nullptr;
	bool flag = false;
};

struct HeightMap
{
	float height = 0.0f;
	std::shared_ptr<Texture> heightTexture = nullptr;
	bool flag = false;
};

class Material
{
  public:
	static std::shared_ptr<Material> createMaterial(Albedo albedo, NormalMap normalMap, Roughness roughness,
													Metallic metallic, AOMap aoMap, HeightMap heightMap);

	~Material() = default;

	void cleanup();

	Albedo &getAlbedo()
	{
		return m_albedo;
	}
	NormalMap &getNormalMap()
	{
		return m_normalMap;
	}
	Roughness &getRoughness()
	{
		return m_roughness;
	}
	Metallic &getMetallic()
	{
		return m_metallic;
	}
	AOMap &getAOMap()
	{
		return m_aoMap;
	}
	HeightMap &getHeightMap()
	{
		return m_heightMap;
	}

	void setAlbedo(Albedo albedo)
	{
		m_albedo = albedo;
	}
	void setNormalMap(NormalMap normalMap)
	{
		m_normalMap = normalMap;
	}
	void setRoughness(Roughness roughness)
	{
		m_roughness = roughness;
	}
	void setMetallic(Metallic metallic)
	{
		m_metallic = metallic;
	}
	void setAOMap(AOMap aoMap)
	{
		m_aoMap = aoMap;
	}
	void setHeightMap(HeightMap heightMap)
	{
		m_heightMap = heightMap;
	}

  private:
	Material() = default;

	Albedo m_albedo;
	NormalMap m_normalMap;
	Roughness m_roughness;
	Metallic m_metallic;
	AOMap m_aoMap;
	HeightMap m_heightMap;

	void initMaterial(Albedo albedo, NormalMap normalMap, Roughness roughness, Metallic metallic, AOMap aoMap,
					  HeightMap heightMap);
};
} // namespace ale