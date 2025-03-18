#pragma once

/**
 * @file RenderingComponent.h
 * @brief 렌더링 컴포넌트 클래스
 */

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/Model.h"
#include "Renderer/ShaderResourceManager.h"

namespace ale
{

struct CullSphere;

/**
 * @class RenderingComponent
 * @brief 렌더링 컴포넌트 클래스
 */
class RenderingComponent
{
  public:
	/**
	 * @brief 렌더링 컴포넌트 생성
	 * @param model 모델
	 * @return std::unique_ptr<RenderingComponent> 렌더링 컴포넌트
	 */
	static std::unique_ptr<RenderingComponent> createRenderingComponent(std::shared_ptr<Model> model);
	~RenderingComponent() = default;

	/**
	 * @brief 렌더링
	 * @param drawInfo 렌더링 정보
	 */
	void draw(DrawInfo &drawInfo);
	/**
	 * @brief 그림자 렌더링
	 * @param drawInfo 렌더링 정보
	 * @param index 인덱스
	 */
	void drawShadow(ShadowMapDrawInfo &drawInfo, uint32_t index);
	/**
	 * @brief 그림자 큐브 맵 렌더링
	 * @param drawInfo 렌더링 정보
	 * @param index 인덱스
	 */
	void drawShadowCubeMap(ShadowCubeMapDrawInfo &drawInfo, uint32_t index);
	/**
	 * @brief 재질 업데이트
	 * @param materials 재질
	 */
	void updateMaterial(std::vector<std::shared_ptr<Material>> materials);
	/**
	 * @brief 모델 업데이트
	 * @param model 모델
	 */
	void updateMaterial(std::shared_ptr<Model> model);
	/**
	 * @brief 모델 반환
	 * @return std::shared_ptr<Model> 모델
	 */
	std::shared_ptr<Model> getModel()
	{
		return m_model;
	};
	/**
	 * @brief 정리
	 */
	void cleanup();
	/**
	 * @brief 컬링 스피어 반환
	 * @return CullSphere 컬링 스피어
	 */
	CullSphere getCullSphere();
	/**
	 * @brief 재질 반환
	 * @return std::vector<std::shared_ptr<Material>> & 재질
	 */
	std::vector<std::shared_ptr<Material>> &getMaterials()
	{
		return m_materials;
	}

	/**
	 * @brief 모델 이름 반환
	 * @return std::string & 모델 이름
	 */
	std::string &getModelName()
	{
		return m_model->getName();
	}

  public:
	/**
	 * @brief 원시 타입
	 */
	enum class EPrimitiveType
	{
		BOX = 0,
		SPHERE,
		PLANE,
		MODEL,
		NONE
	};

  private:
	RenderingComponent() = default;
	std::shared_ptr<Model> m_model;
	std::unique_ptr<ShaderResourceManager> m_shaderResourceManager;
	std::vector<std::shared_ptr<Material>> m_materials;
	/**
	 * @brief 렌더링 컴포넌트 초기화
	 * @param model 모델
	 */
	void initRenderingComponent(std::shared_ptr<Model> model);
};

} // namespace ale