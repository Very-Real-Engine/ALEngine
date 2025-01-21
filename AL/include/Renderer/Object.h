#ifndef OBJECT_H
#define OBJECT_H

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/Model.h"
#include "Renderer/ShaderResourceManager.h"
#include "Renderer/RenderingComponent.h"

namespace ale
{
class AL_API Object
{
public:
	static std::unique_ptr<Object> createObject(std::string name, std::shared_ptr<Model> model, 
	Transform transform);

	~Object() {}

	void draw(DrawInfo& drawInfo);
	void drawShadow(ShadowMapDrawInfo& drawInfo);

	glm::mat4 getModelMatrix();

	const std::string& getName() { return m_name; }
	glm::vec3& getPosition() { return m_transform.position; }
	glm::vec3& getRotation() { return m_transform.rotation; }
	glm::vec3& getScale() { return m_transform.scale; }
	void setPosition(glm::vec3 position) { m_transform.position = position; }
	void setRotation(glm::vec3 rotation) { m_transform.rotation = rotation; }
	void setScale(glm::vec3 scale) { m_transform.scale = scale; }

	void createShaderResourceManager();
	void createRenderingComponent();
	std::vector<std::shared_ptr<Material>>& getMaterials() { return m_renderingComponent->getMaterials(); }

	void updateMaterial(std::vector<std::shared_ptr<Material>> materials);

private:
	Object() = default;

	std::unique_ptr<RenderingComponent> m_renderingComponent;
	std::shared_ptr<Model> m_model;
	Transform m_transform;
	std::string m_name;

	std::unique_ptr<ShaderResourceManager> m_shaderResourceManager;

	void initObject(std::string name, std::shared_ptr<Model> model, 
	Transform transform);


};
} // namespace ale

#endif