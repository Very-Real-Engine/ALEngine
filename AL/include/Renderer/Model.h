#ifndef MODEL_H
#define MODEL_H

#include "Core/Base.h"
#include "Renderer/Common.h"
#include "Renderer/Mesh.h"
#include "Renderer/Material.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace ale
{
class ShaderResourceManager;

struct DrawInfo {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
	ShaderResourceManager* shaderResourceManager;
	VkCommandBuffer commandBuffer;
	VkPipelineLayout pipelineLayout;
	std::vector<std::shared_ptr<Material>> materials;
	uint32_t currentFrame;
};

struct ShadowMapDrawInfo {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;
	ShaderResourceManager* shaderResourceManager;
	VkCommandBuffer commandBuffer;
	VkPipelineLayout pipelineLayout;
	uint32_t currentFrame;
};

struct ShadowCubeMapDrawInfo {
	glm::mat4 model;
	glm::mat4 view[6];
	glm::mat4 projection;
	ShaderResourceManager* shaderResourceManager;
	VkCommandBuffer commandBuffer;
	VkPipelineLayout pipelineLayout;
	uint32_t currentFrame;
};

class AL_API Model
{
  public:
	static std::shared_ptr<Model> createModel(std::string path, std::shared_ptr<Material> &defaultMaterial);
	static std::shared_ptr<Model> createBoxModel(std::shared_ptr<Material> &defaultMaterial);
	static std::shared_ptr<Model> createSphereModel(std::shared_ptr<Material> &defaultMaterial);
	static std::shared_ptr<Model> createPlaneModel(std::shared_ptr<Material> &defaultMaterial);
	~Model() = default;
	void cleanup();

	void draw(DrawInfo& drawInfo);
	void drawShadow(ShadowMapDrawInfo& drawInfo);
	void drawShadowCubeMap(ShadowCubeMapDrawInfo& drawInfo);
	size_t getMeshCount() {return m_meshes.size();}
	std::vector<std::shared_ptr<Mesh>>& getMeshes() {return m_meshes;}
	std::vector<std::shared_ptr<Material>>& getMaterials() {return m_materials;}
	void updateMaterial(std::vector<std::shared_ptr<Material>> materials);
	
  private:
	Model() = default;

	std::vector<std::shared_ptr<Mesh>> m_meshes;
	std::vector<std::shared_ptr<Material>> m_materials;

	void initModel(std::string path, std::shared_ptr<Material> &defaultMaterial);
	void initBoxModel(std::shared_ptr<Material> &defaultMaterial);
	void initSphereModel(std::shared_ptr<Material> &defaultMaterial);
	void initPlaneModel(std::shared_ptr<Material> &defaultMaterial);
	void loadModel(std::string path, std::shared_ptr<Material> &defaultMaterial);
	void loadGLTFModel(std::string path, std::shared_ptr<Material> &defaultMaterial);
	void loadOBJModel(std::string path, std::shared_ptr<Material> &defaultMaterial);
	
	std::string getMaterialPath(std::string &path, std::string materialPath);
	std::shared_ptr<Material> processGLTFMaterial(const aiScene* scene, aiMaterial *material, std::shared_ptr<Material> &defaultMaterial, std::string path);
	void processGLTFNode(aiNode *node, const aiScene *scene, std::vector<std::shared_ptr<Material>> &materials);
	std::shared_ptr<Mesh> processGLTFMesh(aiMesh *mesh, const aiScene *scene, std::shared_ptr<Material> &material);

	void processOBJNode(aiNode *node, const aiScene *scene, std::shared_ptr<Material> &defaultMaterial);
	std::shared_ptr<Mesh> processOBJMesh(aiMesh *mesh, const aiScene *scene, std::shared_ptr<Material> &defaultMaterial);

	std::shared_ptr<Texture> loadMaterialTexture(const aiScene* scene, aiMaterial *material, std::string path, aiString texturePath);
};

} // namespace ale

#endif