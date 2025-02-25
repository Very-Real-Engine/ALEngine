#pragma once

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/Model.h"
#include "Renderer/ShaderResourceManager.h"

namespace ale
{

struct CullSphere;

class RenderingComponent
{
  public:
	static std::unique_ptr<RenderingComponent> createRenderingComponent(std::shared_ptr<Model> model);
	~RenderingComponent() = default;

	void draw(DrawInfo &drawInfo);
	void drawShadow(ShadowMapDrawInfo &drawInfo, uint32_t index);
	void drawShadowCubeMap(ShadowCubeMapDrawInfo &drawInfo, uint32_t index);
	void updateMaterial(std::vector<std::shared_ptr<Material>> materials);
	void updateMaterial(std::shared_ptr<Model> model);
	std::shared_ptr<Model> getModel() { return m_model; };

	void cleanup();

	CullSphere getCullSphere();

	std::vector<std::shared_ptr<Material>> &getMaterials()
	{
		return m_materials;
	}

  public:
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
	std::vector<std::unique_ptr<ShaderResourceManager>> m_shadowMapResourceManager;
	std::vector<std::unique_ptr<ShaderResourceManager>> m_shadowCubeMapResourceManager;
	std::vector<std::shared_ptr<Material>> m_materials;
	void initRenderingComponent(std::shared_ptr<Model> model);
};

} // namespace ale