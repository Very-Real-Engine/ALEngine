#ifndef MODEL_H
#define MODEL_H

#include "Core/Base.h"

#include "Renderer/Animation/SkeletalAnimations.h"
#include "Renderer/Common.h"
#include "Renderer/Material.h"
#include "Renderer/Mesh.h"
#include "Renderer/OBJLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace ale
{
class ShaderResourceManager;

struct DrawInfo
{
	alglm::mat4 model;
	alglm::mat4 view;
	alglm::mat4 projection;
	alglm::mat4 finalBonesMatrices[MAX_BONES];
	ShaderResourceManager *shaderResourceManager;
	VkCommandBuffer commandBuffer;
	VkPipelineLayout pipelineLayout;
	std::vector<std::shared_ptr<Material>> materials;
	uint32_t currentFrame;
};

struct ShadowMapDrawInfo
{
	alglm::mat4 model;
	alglm::mat4 view;
	alglm::mat4 projection;
	ShaderResourceManager *shaderResourceManager;
	VkCommandBuffer commandBuffer;
	VkPipelineLayout pipelineLayout;
	uint32_t currentFrame;
};

struct ShadowCubeMapDrawInfo
{
	alglm::mat4 model;
	alglm::mat4 view[6];
	alglm::mat4 projection;
	ShaderResourceManager *shaderResourceManager;
	VkCommandBuffer commandBuffer;
	VkPipelineLayout pipelineLayout;
	uint32_t currentFrame;
};

struct CullSphere;

class Model
{
  public:
	static std::shared_ptr<Model> createModel(std::string path, std::shared_ptr<Material> &defaultMaterial);
	static std::shared_ptr<Model> createBoxModel(std::shared_ptr<Material> &defaultMaterial);
	static std::shared_ptr<Model> createSphereModel(std::shared_ptr<Material> &defaultMaterial);
	static std::shared_ptr<Model> createPlaneModel(std::shared_ptr<Material> &defaultMaterial);
	static std::shared_ptr<Model> createGroundModel(std::shared_ptr<Material> &defaultMaterial);
	static std::shared_ptr<Model> createCapsuleModel(std::shared_ptr<Material> &defaultMaterial);
	static std::shared_ptr<Model> createCylinderModel(std::shared_ptr<Material> &defaultMaterial);
	static std::shared_ptr<Model> createColliderBoxModel(std::shared_ptr<Material> &defaultMaterial);

	~Model() = default;
	void cleanup();

	void draw(DrawInfo &drawInfo);
	void drawShadow(ShadowMapDrawInfo &drawInfo);
	void drawShadowCubeMap(ShadowCubeMapDrawInfo &drawInfo);
	CullSphere initCullSphere();

	size_t getMeshCount()
	{
		return m_meshes.size();
	}
	std::vector<std::shared_ptr<Mesh>> &getMeshes()
	{
		return m_meshes;
	}
	std::vector<std::shared_ptr<Material>> &getMaterials()
	{
		return m_materials;
	}
	void updateMaterial(std::vector<std::shared_ptr<Material>> materials);
	void updateAnimations(SkeletalAnimation *animation, const Timestep &timestep, uint32_t prevImage,
						  uint32_t currentImage);
	void setShaderData(const std::vector<alglm::mat4> &shaderData);
	std::shared_ptr<SkeletalAnimations> &getAnimations();
	std::shared_ptr<Armature::Skeleton> &getSkeleton();
	bool m_SkeletalAnimations;

  private:
	Model() = default;

	std::vector<std::shared_ptr<Mesh>> m_meshes;
	std::vector<std::shared_ptr<Material>> m_materials;

	// animation
	std::shared_ptr<SkeletalAnimations> m_Animations;
	std::shared_ptr<Armature::Skeleton> m_Skeleton;
	Armature::ShaderData m_ShaderData;

	struct VertexBoneData
	{
		std::vector<std::pair<int, float>> bones;
	};

	void initModel(std::string path, std::shared_ptr<Material> &defaultMaterial);
	void initBoxModel(std::shared_ptr<Material> &defaultMaterial);
	void initSphereModel(std::shared_ptr<Material> &defaultMaterial);
	void initPlaneModel(std::shared_ptr<Material> &defaultMaterial);
	void initGroundModel(std::shared_ptr<Material> &defaultMaterial);
	void initCapsuleModel(std::shared_ptr<Material> &defaultMaterial);
	void initCylinderModel(std::shared_ptr<Material> &defaultMaterial);
	void initColliderBoxModel(std::shared_ptr<Material> &defaultMaterial);

	void loadModel(std::string path, std::shared_ptr<Material> &defaultMaterial);
	void loadGLTFModel(std::string path, std::shared_ptr<Material> &defaultMaterial);
	void loadOBJModel(std::string path, std::shared_ptr<Material> &defaultMaterial);

	std::string getMaterialPath(std::string &path, std::string materialPath);
	std::shared_ptr<Material> processGLTFMaterial(const aiScene *scene, aiMaterial *material,
												  std::shared_ptr<Material> &defaultMaterial, std::string path);
	void processGLTFNode(aiNode *node, const aiScene *scene, std::vector<std::shared_ptr<Material>> &materials);
	std::shared_ptr<Mesh> processGLTFMesh(aiMesh *mesh, const aiScene *scene, std::shared_ptr<Material> &material);

	std::shared_ptr<Material> processOBJMaterial(MTL &mtl, std::shared_ptr<Material> &defaultMaterial);

	std::shared_ptr<Texture> loadMaterialTexture(const aiScene *scene, aiMaterial *material, std::string path,
												 aiString texturePath);
	void processOBJNode(aiNode *node, const aiScene *scene, std::shared_ptr<Material> &defaultMaterial);
	void processGLTFSkeleton(const aiScene *scene);
	void collectAllBones(const aiScene *scene, std::vector<aiBone *> &outBones);
	void buildSkeletonBoneArray(const std::vector<aiBone *> &allAiBones);
	void loadBone(aiNode *node, int parentBoneIndex);
	void loadAnimations(const aiScene *scene);
	alglm::mat4 convertMatrix(const aiMatrix4x4 &m);
};

} // namespace ale

#endif