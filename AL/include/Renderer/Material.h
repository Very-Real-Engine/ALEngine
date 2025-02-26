#pragma once

/**
 * @file Material.h
 * @brief Material 클래스
 */

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/Texture.h"

namespace ale
{
/**
 * @brief Albedo 구조체
 */
struct Albedo
{
	alglm::vec3 albedo = alglm::vec3(1.0f);
	std::shared_ptr<Texture> albedoTexture = nullptr;
	bool flag = false;
};

/**
 * @brief NormalMap 구조체
 */
struct NormalMap
{
	std::shared_ptr<Texture> normalTexture = nullptr;
	bool flag = false;
};

/**
 * @brief Roughness 구조체
 */
struct Roughness
{
	float roughness = 0.5f;
	std::shared_ptr<Texture> roughnessTexture = nullptr;
	bool flag = false;
};

/**
 * @brief Metallic 구조체
 */
struct Metallic
{
	float metallic = 0.0f;
	std::shared_ptr<Texture> metallicTexture = nullptr;
	bool flag = false;
};

/**
 * @brief AOMap 구조체
 */
struct AOMap
{
	float ao = 1.0f;
	std::shared_ptr<Texture> aoTexture = nullptr;
	bool flag = false;
};

/**
 * @brief HeightMap 구조체
 */
struct HeightMap
{
	float height = 0.0f;
	std::shared_ptr<Texture> heightTexture = nullptr;
	bool flag = false;
};

/**
 * @brief Material 클래스
 */
class Material
{
  public:
	/**
	 * @brief Material 생성
	 * @param albedo Albedo
	 * @param normalMap NormalMap
	 * @param roughness Roughness
	 * @param metallic Metallic
	 * @param aoMap AOMap
	 * @param heightMap HeightMap
	 * @return Material
	 */
	static std::shared_ptr<Material> createMaterial(Albedo albedo, NormalMap normalMap, Roughness roughness,
													Metallic metallic, AOMap aoMap, HeightMap heightMap);

	~Material() = default;

	/**
	 * @brief Material 정리
	 */
	void cleanup();

	/**
	 * @brief Albedo 반환
	 * @return Albedo
	 */
	Albedo &getAlbedo()
	{
		return m_albedo;
	}
	/**
	 * @brief NormalMap 반환
	 * @return NormalMap
	 */
	NormalMap &getNormalMap()
	{
		return m_normalMap;
	}
	/**
	 * @brief Roughness 반환
	 */
	Roughness &getRoughness()
	{
		return m_roughness;
	}
	/**
	 * @brief Metallic 반환
	 * @return Metallic
	 */
	Metallic &getMetallic()
	{
		return m_metallic;
	}
	/**
	 * @brief AOMap 반환
	 * @return AOMap
	 */
	AOMap &getAOMap()
	{
		return m_aoMap;
	}
	/**
	 * @brief HeightMap 반환
	 * @return HeightMap
	 */
	HeightMap &getHeightMap()
	{
		return m_heightMap;
	}

	/**
	 * @brief Albedo 설정
	 * @param albedo Albedo
	 */
	void setAlbedo(Albedo albedo)
	{
		m_albedo = albedo;
	}
	/**
	 * @brief NormalMap 설정
	 * @param normalMap NormalMap
	 */
	void setNormalMap(NormalMap normalMap)
	{
		m_normalMap = normalMap;
	}
	/**
	 * @brief Roughness 설정
	 * @param roughness Roughness
	 */
	void setRoughness(Roughness roughness)
	{
		m_roughness = roughness;
	}
	/**
	 * @brief Metallic 설정
	 * @param metallic Metallic
	 */
	void setMetallic(Metallic metallic)
	{
		m_metallic = metallic;
	}
	/**
	 * @brief AOMap 설정
	 * @param aoMap AOMap
	 */
	void setAOMap(AOMap aoMap)
	{
		m_aoMap = aoMap;
	}
	/**
	 * @brief HeightMap 설정
	 * @param heightMap HeightMap
	 */
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

	/**
	 * @brief Material 초기화
	 * @param albedo Albedo
	 * @param normalMap NormalMap
	 * @param roughness Roughness
	 * @param metallic Metallic
	 * @param aoMap AOMap
	 * @param heightMap HeightMap
	 */
	void initMaterial(Albedo albedo, NormalMap normalMap, Roughness roughness, Metallic metallic, AOMap aoMap,
					  HeightMap heightMap);
};
} // namespace ale