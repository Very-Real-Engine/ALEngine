#include "Renderer/RenderingComponent.h"

namespace ale {


std::unique_ptr<RenderingComponent> RenderingComponent::createRenderingComponent(std::shared_ptr<Model> model) {
    std::unique_ptr<RenderingComponent> renderingComponent = std::unique_ptr<RenderingComponent>(new RenderingComponent());
    renderingComponent->initRenderingComponent(model);
    return renderingComponent;
}

void RenderingComponent::initRenderingComponent(std::shared_ptr<Model> model) {
    m_model = model;
    m_materials = m_model->getMaterials();
    m_shaderResourceManager = ShaderResourceManager::createGeometryPassShaderResourceManager(m_model.get());
    for (size_t i = 0; i < 4; i++) {
        m_shadowMapResourceManager.push_back(ShaderResourceManager::createShadowMapShaderResourceManager());
    }
    for (size_t i = 0; i < 4; i++) {
        m_shadowCubeMapResourceManager.push_back(ShaderResourceManager::createShadowCubeMapShaderResourceManager());
    }
}

void RenderingComponent::updateMaterial(std::vector<std::shared_ptr<Material>> materials) {

    for (size_t i = 0; i < m_materials.size() && i < materials.size(); i++) {
		m_materials[i] = materials[i];
	}
    m_shaderResourceManager->updateDescriptorSets(m_model.get(), materials);
}

void RenderingComponent::draw(DrawInfo& drawInfo) {
    drawInfo.shaderResourceManager = m_shaderResourceManager.get();
    drawInfo.materials = m_materials;
    m_model->draw(drawInfo);
}

void RenderingComponent::drawShadow(ShadowMapDrawInfo& drawInfo, uint32_t index) {
    drawInfo.shaderResourceManager = m_shadowMapResourceManager[index].get();
    m_model->drawShadow(drawInfo);
}

void RenderingComponent::drawShadowCubeMap(ShadowCubeMapDrawInfo& drawInfo, uint32_t index) {
    drawInfo.shaderResourceManager = m_shadowCubeMapResourceManager[index].get();
    m_model->drawShadowCubeMap(drawInfo);
}

}