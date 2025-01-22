#ifndef RENDERING_COMPONENT_H
#define RENDERING_COMPONENT_H

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/Model.h"
#include "Renderer/ShaderResourceManager.h"

namespace ale {

class RenderingComponent {
public:
    static std::unique_ptr<RenderingComponent> createRenderingComponent(std::shared_ptr<Model> model);
    ~RenderingComponent() {}
    void draw(DrawInfo& drawInfo);
    void drawShadow(ShadowMapDrawInfo& drawInfo, uint32_t index);
    void updateMaterial(std::vector<std::shared_ptr<Material>> materials);

    std::vector<std::shared_ptr<Material>>& getMaterials() { return m_materials; }

private:
    RenderingComponent() = default;
    std::shared_ptr<Model> m_model;
    std::unique_ptr<ShaderResourceManager> m_shaderResourceManager;
    std::vector<std::unique_ptr<ShaderResourceManager>> m_shadowMapResourceManager;
    std::vector<std::shared_ptr<Material>> m_materials;
    void initRenderingComponent(std::shared_ptr<Model> model);
};

}

#endif