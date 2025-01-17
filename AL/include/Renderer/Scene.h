#ifndef SCENE_H
#define SCENE_H

#include "Core/Base.h"
#include "Renderer/CameraController.h"
#include "Renderer/Common.h"
#include "Renderer/Object.h"

namespace ale
{
class Scene
{
  public:
	// static std::unique_ptr<Scene> createScene();
	~Scene()
	{
	}
	void cleanup();

	const std::vector<std::shared_ptr<Object>> &getObjects()
	{
		return m_objects;
	}
	glm::vec3 getLightPos()
	{
		return m_lightPos;
	}
	glm::vec3 getCamPos()
	{
		return m_cameraPos;
	}
	glm::vec3 getCamFront()
	{
		return m_cameraFront;
	}
	glm::vec3 getCamUp()
	{
		return m_cameraUp;
	}
	float getCamPitch()
	{
		return m_cameraPitch;
	}
	float getCamYaw()
	{
		return m_cameraYaw;
	}
	size_t getObjectCount()
	{
		return m_objectCount;
	}
	glm::mat4 getViewMatrix();
	glm::mat4 getProjMatrix(VkExtent2D swapChainExtent);
	std::shared_ptr<Object> getLightObject()
	{
		return m_lightObject;
	}

	void updateLightPos(glm::vec3 lightPos);

	void mouseButton(int button, int action, double x, double y);
	void mouseMove(double x, double y);
	void processInput(GLFWwindow *window);

  private:
	Scene()
	{
	}
	std::shared_ptr<Model> m_boxModel;
	std::shared_ptr<Model> m_sphereModel;
	std::shared_ptr<Model> m_planeModel;
	std::shared_ptr<Model> m_vikingModel;
	std::shared_ptr<Model> m_catModel;

	std::shared_ptr<Texture> m_vikingTexture;
	std::shared_ptr<Texture> m_sampleTexture;
	std::shared_ptr<Texture> m_catTexture;
	std::shared_ptr<Texture> m_karinaTexture;

	std::vector<std::shared_ptr<Object>> m_objects;
	std::shared_ptr<Object> m_lightObject;

	bool m_cameraControl{false};
	glm::vec2 m_prevMousePos{0.0f, 0.0f};

	float m_cameraPitch{0.0f};
	float m_cameraYaw{0.0f};
	glm::vec3 m_cameraFront{glm::vec3(0.0f, 0.0f, -1.0f)};
	glm::vec3 m_cameraPos{glm::vec3(0.0f, 0.0f, 5.0f)};
	glm::vec3 m_cameraUp{glm::vec3(0.0f, 1.0f, 0.0f)};
	glm::vec3 m_lightPos{0.0f, 1.0f, 0.0f};
	size_t m_objectCount;

	void initScene();
};

} // namespace ale

#endif