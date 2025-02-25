#include "Renderer/Object.h"

namespace ale
{
std::unique_ptr<Object> Object::createObject(std::string name, std::shared_ptr<Model> model,
											 std::shared_ptr<Texture> texture, alglm::vec3 position,
											 alglm::vec3 rotation, alglm::vec3 scale)
{
	std::unique_ptr<Object> object = std::unique_ptr<Object>(new Object());
	object->initObject(name, model, texture, position, rotation, scale);
	return object;
}

// void Object::draw(VkCommandBuffer commandBuffer)
// {
// 	m_model->draw(commandBuffer);
// }

alglm::mat4 Object::getModelMatrix()
{
	alglm::mat4 model = alglm::mat4(1.0f);
	model = alglm::translate(model, m_position);
	model = alglm::rotate(model, alglm::radians(m_rotation.x), alglm::vec3(1.0f, 0.0f, 0.0f));
	model = alglm::rotate(model, alglm::radians(m_rotation.y), alglm::vec3(0.0f, 1.0f, 0.0f));
	model = alglm::rotate(model, alglm::radians(m_rotation.z), alglm::vec3(0.0f, 0.0f, 1.0f));
	model = alglm::scale(model, m_scale);
	return model;
}

void Object::initObject(std::string name, std::shared_ptr<Model> model, std::shared_ptr<Texture> texture,
						alglm::vec3 position, alglm::vec3 rotation, alglm::vec3 scale)
{
	m_name = name;
	m_model = model;
	m_texture = texture;
	m_position = position;
	m_rotation = rotation;
	m_scale = scale;
}
} // namespace ale