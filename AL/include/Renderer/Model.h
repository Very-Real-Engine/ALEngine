#pragma once

/**
 * @file Model.h
 * @brief Model 클래스
 */

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

/**
 * @brief DrawInfo 구조체
 */
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

/**
 * @brief ShadowMapDrawInfo 구조체
 */
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

/**
 * @brief ShadowCubeMapDrawInfo 구조체
 */
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

/**
 * @brief Model 클래스
 */
class Model
{
  public:
	/**
	 * @brief Model 생성
	 * @param path 모델 경로
	 * @param defaultMaterial 기본 재질
	 * @return Model
	 */
	static std::shared_ptr<Model> createModel(std::string path, std::shared_ptr<Material> &defaultMaterial);
	/**
	 * @brief 박스 모델 생성
	 * @param defaultMaterial 기본 재질
	 * @return 박스 모델
	 */
	static std::shared_ptr<Model> createBoxModel(std::shared_ptr<Material> &defaultMaterial);
	/**
	 * @brief 구 모델 생성
	 * @param defaultMaterial 기본 재질
	 * @return 구 모델
	 */
	static std::shared_ptr<Model> createSphereModel(std::shared_ptr<Material> &defaultMaterial);
	/**
	 * @brief 평면 모델 생성
	 * @param defaultMaterial 기본 재질
	 * @return 평면 모델
	 */
	static std::shared_ptr<Model> createPlaneModel(std::shared_ptr<Material> &defaultMaterial);
	/**
	 * @brief 지면 모델 생성
	 * @param defaultMaterial 기본 재질
	 * @return 지면 모델
	 */
	static std::shared_ptr<Model> createGroundModel(std::shared_ptr<Material> &defaultMaterial);
	/**
	 * @brief 캡슐 모델 생성
	 * @param defaultMaterial 기본 재질
	 * @return 캡슐 모델
	 */
	static std::shared_ptr<Model> createCapsuleModel(std::shared_ptr<Material> &defaultMaterial);
	/**
	 * @brief 원기둥 모델 생성
	 * @param defaultMaterial 기본 재질
	 * @return 원기둥 모델
	 */
	static std::shared_ptr<Model> createCylinderModel(std::shared_ptr<Material> &defaultMaterial);
	/**
	 * @brief 콜라이더 박스 모델 생성
	 * @param defaultMaterial 기본 재질
	 * @return 콜라이더 박스 모델
	 */
	static std::shared_ptr<Model> createColliderBoxModel(std::shared_ptr<Material> &defaultMaterial);

	~Model() = default;
	/**
	 * @brief Model 정리
	 */
	void cleanup();
	/**
	 * @brief 모델 그리기
	 * @param drawInfo 그리기 정보
	 */
	void draw(DrawInfo &drawInfo);
	/**
	 * @brief 그림자 맵 그리기
	 * @param drawInfo 그리기 정보
	 */
	void drawShadow(ShadowMapDrawInfo &drawInfo);
	/**
	 * @brief 그림자 큐브 맵 그리기
	 * @param drawInfo 그리기 정보
	 */
	void drawShadowCubeMap(ShadowCubeMapDrawInfo &drawInfo);
	/**
	 * @brief 컬링 스피어 초기화
	 * @return 컬링 스피어
	 */
	CullSphere initCullSphere();

	/**
	 * @brief 메시 개수 반환
	 * @return 메시 개수
	 */
	size_t getMeshCount()
	{
		return m_meshes.size();
	}
	/**
	 * @brief 메시 반환
	 * @return 메시
	 */
	std::vector<std::shared_ptr<Mesh>> &getMeshes()
	{
		return m_meshes;
	}
	/**
	 * @brief 재질 반환
	 * @return 재질
	 */
	std::vector<std::shared_ptr<Material>> &getMaterials()
	{
		return m_materials;
	}
	/**
	 * @brief 재질 업데이트
	 * @param materials 재질
	 */
	void updateMaterial(std::vector<std::shared_ptr<Material>> materials);
	/**
	 * @brief 애니메이션 업데이트
	 * @param animation 애니메이션
	 * @param timestep 타임스텝
	 * @param prevImage 이전 이미지
	 * @param currentImage 현재 이미지
	 */
	void updateAnimations(SkeletalAnimation *animation, const Timestep &timestep, uint32_t prevImage,
						  uint32_t currentImage);
	/**
	 * @brief 쉐이더 데이터 설정
	 * @param shaderData 쉐이더 데이터
	 */
	void setShaderData(const std::vector<alglm::mat4> &shaderData);
	/**
	 * @brief 애니메이션 반환
	 * @return 애니메이션
	 */
	std::shared_ptr<SkeletalAnimations> &getAnimations();
	/**
	 * @brief 스켈레톤 반환
	 * @return 스켈레톤
	 */
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

	/**
	 * @brief 모델 초기화
	 * @param path 모델 경로
	 * @param defaultMaterial 기본 재질
	 */
	void initModel(std::string path, std::shared_ptr<Material> &defaultMaterial);
	/**
	 * @brief 박스 모델 초기화
	 * @param defaultMaterial 기본 재질
	 */
	void initBoxModel(std::shared_ptr<Material> &defaultMaterial);
	/**
	 * @brief 구 모델 초기화
	 * @param defaultMaterial 기본 재질
	 */
	void initSphereModel(std::shared_ptr<Material> &defaultMaterial);
	/**
	 * @brief 평면 모델 초기화
	 * @param defaultMaterial 기본 재질
	 */
	void initPlaneModel(std::shared_ptr<Material> &defaultMaterial);
	/**
	 * @brief 지면 모델 초기화
	 * @param defaultMaterial 기본 재질
	 */
	void initGroundModel(std::shared_ptr<Material> &defaultMaterial);
	/**
	 * @brief 캡슐 모델 초기화
	 * @param defaultMaterial 기본 재질
	 */
	void initCapsuleModel(std::shared_ptr<Material> &defaultMaterial);
	/**
	 * @brief 원기둥 모델 초기화
	 * @param defaultMaterial 기본 재질
	 */
	void initCylinderModel(std::shared_ptr<Material> &defaultMaterial);
	/**
	 * @brief 콜라이더 박스 모델 초기화
	 * @param defaultMaterial 기본 재질
	 */
	void initColliderBoxModel(std::shared_ptr<Material> &defaultMaterial);

	/**
	 * @brief 모델 로드
	 * @param path 모델 경로
	 * @param defaultMaterial 기본 재질
	 */
	void loadModel(std::string path, std::shared_ptr<Material> &defaultMaterial);

	/**
	 * @brief GLTF 모델 로드
	 * @param path 모델 경로
	 * @param defaultMaterial 기본 재질
	 */
	void loadGLTFModel(std::string path, std::shared_ptr<Material> &defaultMaterial);
	/**
	 * @brief OBJ 모델 로드
	 * @param path 모델 경로
	 * @param defaultMaterial 기본 재질
	 */
	void loadOBJModel(std::string path, std::shared_ptr<Material> &defaultMaterial);

	/**
	 * @brief 재질 경로 반환
	 * @param path 모델 경로
	 * @param materialPath 재질 경로
	 * @return 재질 경로
	 */
	std::string getMaterialPath(std::string &path, std::string materialPath);
	/**
	 * @brief GLTF 재질 처리
	 * @param scene 장면
	 * @param material 재질
	 * @param defaultMaterial 기본 재질
	 */
	std::shared_ptr<Material> processGLTFMaterial(const aiScene *scene, aiMaterial *material,
												  std::shared_ptr<Material> &defaultMaterial, std::string path);
	/**
	 * @brief GLTF 노드 처리
	 * @param node 노드
	 * @param scene 장면
	 * @param materials 재질
	 */
	void processGLTFNode(aiNode *node, const aiScene *scene, std::vector<std::shared_ptr<Material>> &materials);
	/**
	 * @brief GLTF 메시 처리
	 * @param mesh 메시
	 * @param scene 장면
	 * @param material 재질
	 * @return 메시
	 */
	std::shared_ptr<Mesh> processGLTFMesh(aiMesh *mesh, const aiScene *scene, std::shared_ptr<Material> &material);
	/**
	 * @brief OBJ 재질 처리
	 * @param mtl MTL
	 * @param defaultMaterial 기본 재질
	 * @return 재질
	 */
	std::shared_ptr<Material> processOBJMaterial(MTL &mtl, std::shared_ptr<Material> &defaultMaterial);
	/**
	 * @brief 텍스처 로드
	 * @param scene 장면
	 * @param material 재질
	 * @param path 모델 경로
	 * @param texturePath 텍스처 경로
	 * @return 텍스처
	 */
	std::shared_ptr<Texture> loadMaterialTexture(const aiScene *scene, aiMaterial *material, std::string path,
												 aiString texturePath);
	/**
	 * @brief OBJ 노드 처리
	 * @param node 노드
	 * @param scene 장면
	 * @param defaultMaterial 기본 재질
	 */
	void processOBJNode(aiNode *node, const aiScene *scene, std::shared_ptr<Material> &defaultMaterial);
	/**
	 * @brief GLTF 스켈레톤 처리
	 * @param scene 장면
	 */
	void processGLTFSkeleton(const aiScene *scene);
	/**
	 * @brief 모든 본 수집
	 * @param scene 장면
	 * @param outBones 본
	 */
	void collectAllBones(const aiScene *scene, std::vector<aiBone *> &outBones);

	/**
	 * @brief 스켈레톤 본 배열 빌드
	 * @param allAiBones 본
	 */
	void buildSkeletonBoneArray(const std::vector<aiBone *> &allAiBones);
	/**
	 * @brief 본 로드
	 * @param node 노드
	 * @param parentBoneIndex 부모 본 인덱스
	 */
	void loadBone(aiNode *node, int parentBoneIndex);

	/**
	 * @brief 애니메이션 로드
	 * @param scene 장면
	 */
	void loadAnimations(const aiScene *scene);
	/**
	 * @brief 행렬 변환
	 * @param m 행렬
	 * @return 행렬
	 */
	alglm::mat4 convertMatrix(const aiMatrix4x4 &m);
};

} // namespace ale
