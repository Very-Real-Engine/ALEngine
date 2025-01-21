#ifndef SCENE_H
#define SCENE_H

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/Object.h"
#include "Renderer/Material.h"

namespace ale
{
class AL_API Scene
{
public:
	static std::unique_ptr<Scene> createScene();
	~Scene() {}
	void cleanup();

	const std::vector< std::shared_ptr<Object> >& getObjects() { return m_objects; }
	std::vector<std::shared_ptr<Object>> &getLightObjects() { return m_lightObjects; }
	std::vector<Light> &getLights() { return m_lights; }
	glm::vec3 &getCamPos() { return m_cameraPos; }
	glm::vec3 &getCamFront() { return m_cameraFront; }
	glm::vec3 &getCamUp() { return m_cameraUp; }
	float &getCamPitch() { return m_cameraPitch; }
	float &getCamYaw() { return m_cameraYaw; }
	size_t getObjectCount() { return m_objectCount; }
	glm::mat4 getViewMatrix();
	glm::mat4 getProjMatrix(VkExtent2D swapChainExtent);
	float &getAmbientStrength() { return m_ambientStrength; }
	DefaultTextures &getDefaultTextures() { return m_defaultTextures; }

	// void updateLightPos(glm::vec3 lightPos);
	void mouseButton(int button, int action, double x, double y);
	void mouseMove(double x, double y);
	void processInput(GLFWwindow* window);




private:
	Scene() {}

	DefaultTextures m_defaultTextures;

	std::shared_ptr<Model> m_boxModel;
	std::shared_ptr<Model> m_sphereModel;
	std::shared_ptr<Model> m_planeModel;



	std::vector< std::shared_ptr<Object> > m_objects;
	
	
	
	// floor
	std::shared_ptr<Texture> m_floorDiffuseTexture;
	std::shared_ptr<Texture> m_floorNormalTexture;
	std::shared_ptr<Texture> m_floorRoughnessTexture;
	std::shared_ptr<Material> m_floorMaterial;
	std::shared_ptr<Object> m_floorObject;


	// camera object
	std::shared_ptr<Model> m_cameraModel;
	std::shared_ptr<Object> m_cameraObject;

	// alarm
	std::shared_ptr<Model> m_alarmModel;
	std::shared_ptr<Object> m_alarmObject;

	// table
	std::shared_ptr<Model> m_tableModel;
	std::shared_ptr<Object> m_tableObject;

	// sofa
	std::shared_ptr<Model> m_sofaModel;
	std::shared_ptr<Object> m_sofaObject;

	// jug
	std::shared_ptr<Model> m_jugModel;
	std::shared_ptr<Object> m_jugObject;

	// shelf
	std::shared_ptr<Model> m_shelfModel;
	std::shared_ptr<Object> m_shelfObject;

	// plant1
	std::shared_ptr<Model> m_plant1Model;
	std::shared_ptr<Object> m_plant1Object;

	// plant2
	std::shared_ptr<Model> m_plant2Model;
	std::shared_ptr<Object> m_plant2Object;


	std::shared_ptr<Material> m_defaultMaterial;


	bool m_cameraControl { false };
	glm::vec2 m_prevMousePos { 0.0f, 0.0f };

    glm::vec3 m_cameraPos { glm::vec3(0.0f, 0.0f, 5.0f) };
    glm::vec3 m_cameraFront { glm::vec3(0.0f, 0.0f, -1.0f) };
    glm::vec3 m_cameraUp { glm::vec3(0.0f, 1.0f, 0.0f) };
	float m_cameraPitch { 0.0f };
    float m_cameraYaw { 0.0f };
	size_t m_objectCount;

	// lighting
	std::vector<Light> m_lights;
	uint32_t m_numLights { 0 };
	float m_ambientStrength { 0.2f };
	std::vector<std::shared_ptr<Object>> m_lightObjects;
	

	void initScene();
};

} // namespace ale

#endif