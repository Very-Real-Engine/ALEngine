#include "Renderer/Scene.h"
#include <random>

namespace ale
{
std::unique_ptr<Scene> Scene::createScene() {
    std::unique_ptr<Scene> scene = std::unique_ptr<Scene>(new Scene());
    scene->initScene();
    return scene;
}


// object는 cleanup 해줄 필요 없지만 model과 texture는 해줘야함
void Scene::cleanup() {
    m_defaultTextures.albedo->cleanup();
    m_defaultTextures.normal->cleanup();
    m_defaultTextures.roughness->cleanup();
    m_defaultTextures.metallic->cleanup();
    m_defaultTextures.ao->cleanup();
    m_defaultTextures.height->cleanup();

    m_boxModel->cleanup();
    m_sphereModel->cleanup();
    m_planeModel->cleanup();

}


glm::mat4 Scene::getViewMatrix() {
    m_cameraFront =
        glm::rotate(glm::mat4(1.0f),
        glm::radians(m_cameraYaw), glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f),
        glm::radians(m_cameraPitch), glm::vec3(1.0f, 0.0f, 0.0f)) *
        glm::vec4(0.0f, 0.0f, -1.0f, 0.0f);
    return glm::lookAt(m_cameraPos, m_cameraPos + m_cameraFront, m_cameraUp);
}


glm::mat4 Scene::getProjMatrix(VkExtent2D swapChainExtent) {
    return glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 100.0f);
}


// void Scene::updateLightPos(glm::vec3 lightPos) {
//     m_lightInfo.lightPos = lightPos;
//     m_lightInfo.lightDirection = glm::normalize(-lightPos);
//     m_lightObject->setPosition(lightPos);
// }


void Scene::initScene() {

    m_defaultTextures.albedo = Texture::createDefaultTexture(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    m_defaultTextures.normal = Texture::createDefaultTexture(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    m_defaultTextures.roughness = Texture::createDefaultTexture(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
    m_defaultTextures.metallic = Texture::createDefaultTexture(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    m_defaultTextures.ao = Texture::createDefaultTexture(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    m_defaultTextures.height = Texture::createDefaultTexture(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

    m_defaultMaterial = Material::createMaterial(
        {glm::vec3(1.0f, 1.0f, 1.0f), m_defaultTextures.albedo, false},
        {m_defaultTextures.normal, false},
        {0.5f, m_defaultTextures.roughness, false},
        {0.0f, m_defaultTextures.metallic, false},
        {1.0f, m_defaultTextures.ao, false},
        {0.0f, m_defaultTextures.height, false}
    );

    m_cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
    m_cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    m_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    m_cameraPitch = 0.0f;
    m_cameraYaw = 0.0f;

    glm::mat4 view[6];
    for (int i = 0; i < 6; i++) {
        view[i] = glm::mat4(1.0f);
    }
    glm::mat4 proj = glm::mat4(1.0f);
    Light light1 {
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        1.0f,
        glm::cos(glm::radians(12.5f)),
        glm::cos(glm::radians(17.5f)),
        1,
        1,
        0,
        glm::vec2(0.0f, 0.0f)
    };
    m_lights.push_back(light1);
    m_numLights = m_lights.size();

    std::random_device rd;                              // 시드 생성
    std::mt19937 gen(rd());                             // 난수 생성기
    std::uniform_real_distribution<float> posDist(-3.0f, 3.0f); // 위치는 -5 ~ 5 사이
    std::uniform_real_distribution<float> colorDist(0.0f, 1.0f); // 색상은 0 ~ 1 사이
    std::uniform_real_distribution<float> dirDist(-1.0f, 1.0f); // 방향은 -1 ~ 1 사이

    for (size_t i = 0; i < 4; ++i) {
        Light light {
            glm::vec3(posDist(gen), 0.0f, posDist(gen)), // 랜덤 위치
            glm::vec3(dirDist(gen), -1.0f, dirDist(gen)),                      // 방향은 고정
            glm::vec3(colorDist(gen), colorDist(gen), colorDist(gen)), // 랜덤 색상
            1.0f,                                              // 강도 (기본값)
            glm::cos(glm::radians(12.5f)),
            glm::cos(glm::radians(17.5f)),
            1,                                                 // 점광원 (type = 0)
            1,
            0,
            glm::vec2(0.0f, 0.0f)
        };
        m_lights.push_back(light);
    }
    m_numLights = m_lights.size(); // 광원 개수 설정


    m_boxModel = Model::createBoxModel(m_defaultMaterial);
    m_sphereModel = Model::createSphereModel(m_defaultMaterial);
    m_planeModel = Model::createPlaneModel(m_defaultMaterial);

    for (auto& light : m_lights) {
        m_lightObjects.push_back(Object::createObject("light", m_sphereModel, 
        Transform{light.position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.1f, 0.1f, 0.1f)}));
    }
    

    m_floorDiffuseTexture = Texture::createTexture("textures/laminate_floor_02_diff_2k.jpg");
    m_floorNormalTexture = Texture::createMaterialTexture("textures/laminate_floor_02_nor_gl_2k.jpg");
    m_floorRoughnessTexture = Texture::createMaterialTexture("textures/laminate_floor_02_rough_2k.jpg");
    

    m_floorMaterial = Material::createMaterial(
        {glm::vec3(1.0f, 1.0f, 1.0f), m_floorDiffuseTexture, true},
        {m_floorNormalTexture, true},
        {0.5f, m_floorRoughnessTexture, true},
        {0.0f, m_defaultTextures.metallic, false},
        {1.0f, m_defaultTextures.ao, false},
        {0.0f, m_defaultTextures.height, false}
    );

    m_floorObject = Object::createObject("floor", m_planeModel, 
    Transform{glm::vec3(0.0f, -1.7f, 0.0f), glm::vec3(-90.0f, 0.0f, 0.0f), glm::vec3(5.0f, 5.0f, 5.0f)});
    m_objects.push_back(m_floorObject);


    m_tableModel = Model::createModel("Models/coffee_table_round_01_4k.gltf/coffee_table_round_01_4k.gltf", m_defaultMaterial);
    m_tableObject = Object::createObject("table", m_tableModel, 
    Transform{glm::vec3(0.0f, -1.7f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)});
    m_objects.push_back(m_tableObject);

    m_cameraModel = Model::createModel("Models/Camera_01_2k.gltf/Camera_01_2k.gltf", m_defaultMaterial);
    m_cameraObject = Object::createObject("camera", m_cameraModel, 
    Transform{glm::vec3(0.1f, -1.2f, 0.1f), glm::vec3(0.0f, -50.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)});
    m_objects.push_back(m_cameraObject);

    m_sofaModel = Model::createModel("Models/Sofa_01_4k.gltf/Sofa_01_4k.gltf", m_defaultMaterial);
    m_sofaObject = Object::createObject("sofa", m_sofaModel, 
    Transform{glm::vec3(0.0f, -1.7f, -1.2f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)});
    m_objects.push_back(m_sofaObject);

    m_jugModel = Model::createModel("Models/jug_01_4k.gltf/jug_01_4k.gltf", m_defaultMaterial);
    m_jugObject = Object::createObject("jug", m_jugModel, 
    Transform{glm::vec3(-0.1f, -1.2f, -0.1f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)});
    m_objects.push_back(m_jugObject);


    m_shelfModel = Model::createModel("Models/steel_frame_shelves_02_4k.gltf/steel_frame_shelves_02_4k.gltf", m_defaultMaterial);
    m_shelfObject = Object::createObject("shelf", m_shelfModel, 
    Transform{glm::vec3(-1.6f, -1.7f, -1.25f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)});
    m_objects.push_back(m_shelfObject);


    m_plant1Model = Model::createModel("Models/potted_plant_01_4k.gltf/potted_plant_01_4k.gltf", m_defaultMaterial);
    m_plant1Object = Object::createObject("plant1", m_plant1Model, 
    Transform{glm::vec3(1.5f, -1.7f, -0.8f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)});
    m_objects.push_back(m_plant1Object);


    m_plant2Model = Model::createModel("Models/potted_plant_04_4k.gltf/potted_plant_04_4k.gltf", m_defaultMaterial);
    m_plant2Object = Object::createObject("plant2", m_plant2Model, 
    Transform{glm::vec3(-1.6f, -0.56f, -1.25f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f)});
    m_objects.push_back(m_plant2Object);

    m_objectCount = m_objects.size();

    for (auto& object : m_objects) {
        object->createRenderingComponent();
    }

    for (auto& lightObject : m_lightObjects) {
        lightObject->createRenderingComponent();
    }

    m_floorObject->updateMaterial({m_floorMaterial});

}


void Scene::processInput(GLFWwindow* window) {
    if (!m_cameraControl)
        return;
    const float cameraSpeed = 0.01f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        m_cameraPos += cameraSpeed * m_cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        m_cameraPos -= cameraSpeed * m_cameraFront;

    auto cameraRight = glm::normalize(glm::cross(m_cameraUp, -m_cameraFront));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        m_cameraPos += cameraSpeed * cameraRight;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        m_cameraPos -= cameraSpeed * cameraRight;    

    auto cameraUp = glm::normalize(glm::cross(-m_cameraFront, cameraRight));
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        m_cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        m_cameraPos -= cameraSpeed * cameraUp;
}


void Scene::mouseMove(double x, double y) {
    if (!m_cameraControl)
        return;
    auto pos = glm::vec2((float)x, (float)y);
    auto deltaPos = pos - m_prevMousePos;

    const float cameraRotSpeed = 0.8f;
    m_cameraYaw -= deltaPos.x * cameraRotSpeed;
    m_cameraPitch -= deltaPos.y * cameraRotSpeed;

    if (m_cameraYaw < 0.0f)   m_cameraYaw += 360.0f;
    if (m_cameraYaw > 360.0f) m_cameraYaw -= 360.0f;

    if (m_cameraPitch > 89.0f)  m_cameraPitch = 89.0f;
    if (m_cameraPitch < -89.0f) m_cameraPitch = -89.0f;

    m_prevMousePos = pos;
}

void Scene::mouseButton(int button, int action, double x, double y) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            m_prevMousePos = glm::vec2((float)x, (float)y);
            m_cameraControl = true;
        }
        else if (action == GLFW_RELEASE) {
            m_cameraControl = false;
        }
    }
}


} // namespace ale