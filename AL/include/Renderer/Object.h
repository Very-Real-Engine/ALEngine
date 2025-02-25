#ifndef OBJECT_H
#define OBJECT_H

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/Model.h"
#include "Renderer/Texture.h"

namespace ale
{
class Object
{
  public:
	static std::unique_ptr<Object> createObject(std::string name, std::shared_ptr<Model> model,
												std::shared_ptr<Texture> texture, alglm::vec3 position,
												alglm::vec3 rotation, alglm::vec3 scale);
	~Object()
	{
	}

	void draw(VkCommandBuffer commandBuffer);

	alglm::mat4 getModelMatrix();
	const std::shared_ptr<Texture> &getTexture()
	{
		return m_texture;
	}

	const std::string &getName()
	{
		return m_name;
	}
	alglm::vec3 &getPosition()
	{
		return m_position;
	}
	alglm::vec3 &getRotation()
	{
		return m_rotation;
	}
	alglm::vec3 &getScale()
	{
		return m_scale;
	}
	void setPosition(alglm::vec3 position)
	{
		m_position = position;
	}
	void setRotation(alglm::vec3 rotation)
	{
		m_rotation = rotation;
	}
	void setScale(alglm::vec3 scale)
	{
		m_scale = scale;
	}

  private:
	Object()
	{
	}

	std::shared_ptr<Model> m_model;
	std::shared_ptr<Texture> m_texture;
	alglm::vec3 m_position;
	alglm::vec3 m_rotation;
	alglm::vec3 m_scale;
	std::string m_name;

	void initObject(std::string name, std::shared_ptr<Model> model, std::shared_ptr<Texture> texture,
					alglm::vec3 position, alglm::vec3 rotation, alglm::vec3 scale);
};
} // namespace ale

#endif