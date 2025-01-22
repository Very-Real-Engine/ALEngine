#include "Renderer/Object.h"

namespace ale
{
std::unique_ptr<Object> Object::createObject(std::string name, std::shared_ptr<Model> model, Transform transform)
{
	std::unique_ptr<Object> object = std::unique_ptr<Object>(new Object());
	object->initObject(name, model, transform);
	return object;
}

void Object::draw(DrawInfo& drawInfo)
{
	drawInfo.model = getModelMatrix();
	m_renderingComponent->draw(drawInfo);
}

void Object::drawShadow(ShadowMapDrawInfo& drawInfo, uint32_t index)
{
	drawInfo.model = getModelMatrix();
	m_renderingComponent->drawShadow(drawInfo, index);
}

glm::mat4 Object::getModelMatrix()
{
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, m_transform.position);
	model = glm::rotate(model, glm::radians(m_transform.rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, glm::radians(m_transform.rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, glm::radians(m_transform.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, m_transform.scale);
	return model;
}

void Object::initObject(std::string name, std::shared_ptr<Model> model, Transform transform)
{
	m_name = name;
	m_model = model;
	m_transform = transform;
}

void Object::createShaderResourceManager() {
	m_shaderResourceManager = ShaderResourceManager::createGeometryPassShaderResourceManager(m_model.get());
}

void Object::createRenderingComponent() {
	m_renderingComponent = RenderingComponent::createRenderingComponent(m_model);
}

void Object::updateMaterial(std::vector<std::shared_ptr<Material>> materials)
{
	m_renderingComponent->updateMaterial(materials);
}

} // namespace ale