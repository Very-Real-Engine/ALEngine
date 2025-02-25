#include "Renderer/Model.h"
#include "Core/App.h"
#include "Renderer/ShaderResourceManager.h"
#include "Scene/CullTree.h"

#include <glm/gtx/string_cast.hpp>

namespace ale
{
std::shared_ptr<Model> Model::createModel(std::string path, std::shared_ptr<Material> &defaultMaterial)
{
	auto &renderer = App::get().getRenderer();
	auto &modelsMap = renderer.getModelsMap();
	if (modelsMap.find(path) != modelsMap.end())
	{
		return modelsMap[path];
	}
	std::shared_ptr<Model> model = std::shared_ptr<Model>(new Model());
	model->initModel(path, defaultMaterial);
	modelsMap[path] = model;
	return model;
}

std::shared_ptr<Model> Model::createBoxModel(std::shared_ptr<Material> &defaultMaterial)
{
	auto &renderer = App::get().getRenderer();
	auto &modelsMap = renderer.getModelsMap();
	if (modelsMap.find("box") != modelsMap.end())
	{
		return modelsMap["box"];
	}
	std::shared_ptr<Model> model = std::shared_ptr<Model>(new Model());
	model->initBoxModel(defaultMaterial);
	modelsMap["box"] = model;
	return model;
}

std::shared_ptr<Model> Model::createSphereModel(std::shared_ptr<Material> &defaultMaterial)
{
	auto &renderer = App::get().getRenderer();
	auto &modelsMap = renderer.getModelsMap();
	if (modelsMap.find("sphere") != modelsMap.end())
	{
		return modelsMap["sphere"];
	}
	std::shared_ptr<Model> model = std::shared_ptr<Model>(new Model());
	model->initSphereModel(defaultMaterial);
	modelsMap["sphere"] = model;
	return model;
}

std::shared_ptr<Model> Model::createPlaneModel(std::shared_ptr<Material> &defaultMaterial)
{
	auto &renderer = App::get().getRenderer();
	auto &modelsMap = renderer.getModelsMap();
	if (modelsMap.find("plane") != modelsMap.end())
	{
		return modelsMap["plane"];
	}
	std::shared_ptr<Model> model = std::shared_ptr<Model>(new Model());
	model->initPlaneModel(defaultMaterial);
	modelsMap["plane"] = model;
	return model;
}

std::shared_ptr<Model> Model::createGroundModel(std::shared_ptr<Material> &defaultMaterial)
{
	auto &renderer = App::get().getRenderer();
	auto &modelsMap = renderer.getModelsMap();
	if (modelsMap.find("ground") != modelsMap.end())
	{
		return modelsMap["ground"];
	}
	std::shared_ptr<Model> model = std::shared_ptr<Model>(new Model());
	model->initGroundModel(defaultMaterial);
	modelsMap["ground"] = model;
	return model;
}

std::shared_ptr<Model> Model::createCapsuleModel(std::shared_ptr<Material> &defaultMaterial)
{
	auto &renderer = App::get().getRenderer();
	auto &modelsMap = renderer.getModelsMap();
	if (modelsMap.find("capsule") != modelsMap.end())
	{
		return modelsMap["capsule"];
	}
	std::shared_ptr<Model> model = std::shared_ptr<Model>(new Model());
	model->initCapsuleModel(defaultMaterial);
	modelsMap["capsule"] = model;
	return model;
}
std::shared_ptr<Model> Model::createCylinderModel(std::shared_ptr<Material> &defaultMaterial)
{
	auto &renderer = App::get().getRenderer();
	auto &modelsMap = renderer.getModelsMap();
	if (modelsMap.find("cylinder") != modelsMap.end())
	{
		return modelsMap["cylinder"];
	}
	std::shared_ptr<Model> model = std::shared_ptr<Model>(new Model());
	model->initCylinderModel(defaultMaterial);
	modelsMap["cylinder"] = model;
	return model;
}

void Model::cleanup()
{
	for (auto &mesh : m_meshes)
	{
		mesh->cleanup();
	}
	for (auto &material : m_materials)
	{
		material->cleanup();
	}
}

void Model::draw(DrawInfo &drawInfo)
{
	auto &descriptorSets = drawInfo.shaderResourceManager->getDescriptorSets();
	auto &vertexUniformBuffers = drawInfo.shaderResourceManager->getVertexUniformBuffers();
	auto &fragmentUniformBuffers = drawInfo.shaderResourceManager->getFragmentUniformBuffers();
	for (uint32_t i = 0; i < m_meshes.size(); i++)
	{
		uint32_t index = MAX_FRAMES_IN_FLIGHT * i + drawInfo.currentFrame;
		vkCmdBindDescriptorSets(drawInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, drawInfo.pipelineLayout, 0, 1,
								&descriptorSets[index], 0, nullptr);
		GeometryPassVertexUniformBufferObject vertexUbo{};
		vertexUbo.model = drawInfo.model;
		vertexUbo.view = drawInfo.view;
		vertexUbo.proj = drawInfo.projection;
		for (size_t i = 0; i < MAX_BONES; ++i)
			vertexUbo.finalBonesMatrices[i] = drawInfo.finalBonesMatrices[i];
		vertexUbo.heightFlag = drawInfo.materials[i]->getHeightMap().flag;
		vertexUbo.heightScale = 0.1;
		vertexUbo.padding = glm::vec2(0.0f);
		vertexUniformBuffers[index]->updateUniformBuffer(&vertexUbo, sizeof(vertexUbo));

		GeometryPassFragmentUniformBufferObject fragmentUbo{};
		fragmentUbo.albedoValue = glm::vec4(drawInfo.materials[i]->getAlbedo().albedo, 1.0f);
		fragmentUbo.roughnessValue = drawInfo.materials[i]->getRoughness().roughness;
		fragmentUbo.metallicValue = drawInfo.materials[i]->getMetallic().metallic;
		fragmentUbo.aoValue = drawInfo.materials[i]->getAOMap().ao;
		fragmentUbo.albedoFlag = drawInfo.materials[i]->getAlbedo().flag;
		fragmentUbo.normalFlag = drawInfo.materials[i]->getNormalMap().flag;
		fragmentUbo.roughnessFlag = drawInfo.materials[i]->getRoughness().flag;
		fragmentUbo.metallicFlag = drawInfo.materials[i]->getMetallic().flag;
		fragmentUbo.aoFlag = drawInfo.materials[i]->getAOMap().flag;
		fragmentUbo.padding = glm::vec2(0.0f);
		fragmentUniformBuffers[index]->updateUniformBuffer(&fragmentUbo, sizeof(fragmentUbo));

		m_meshes[i]->draw(drawInfo.commandBuffer);
	}
}

void Model::drawShadow(ShadowMapDrawInfo &drawInfo)
{
	auto &descriptorSets = drawInfo.shaderResourceManager->getDescriptorSets();
	auto &uniformBuffers = drawInfo.shaderResourceManager->getUniformBuffers();
	for (uint32_t i = 0; i < m_meshes.size(); i++)
	{
		uint32_t index = drawInfo.currentFrame;
		vkCmdBindDescriptorSets(drawInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, drawInfo.pipelineLayout, 0, 1,
								&descriptorSets[index], 0, nullptr);
		ShadowMapUniformBufferObject shadowMapUbo{};
		shadowMapUbo.model = drawInfo.model;
		shadowMapUbo.view = drawInfo.view;
		shadowMapUbo.proj = drawInfo.projection;
		uniformBuffers[index]->updateUniformBuffer(&shadowMapUbo, sizeof(shadowMapUbo));
		m_meshes[i]->draw(drawInfo.commandBuffer);
	}
}

void Model::drawShadowCubeMap(ShadowCubeMapDrawInfo &drawInfo)
{
	auto &descriptorSets = drawInfo.shaderResourceManager->getDescriptorSets();
	auto &uniformBuffers = drawInfo.shaderResourceManager->getUniformBuffers();
	auto &layerIndexUniformBuffers = drawInfo.shaderResourceManager->getLayerIndexUniformBuffers();
	uint32_t currentFrame = drawInfo.currentFrame;
	ShadowCubeMapUniformBufferObject shadowCubeMapUbo{};
	shadowCubeMapUbo.model = drawInfo.model;
	shadowCubeMapUbo.proj = drawInfo.projection;
	for (uint32_t i = 0; i < 6; i++)
	{
		shadowCubeMapUbo.view[i] = drawInfo.view[i];
	}

	for (uint32_t i = 0; i < 6; i++)
	{
		uint32_t index = currentFrame * 6 + i;
		for (uint32_t j = 0; j < m_meshes.size(); j++)
		{
			vkCmdBindDescriptorSets(drawInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, drawInfo.pipelineLayout, 0,
									1, &descriptorSets[index], 0, nullptr);
			uniformBuffers[currentFrame]->updateUniformBuffer(&shadowCubeMapUbo, sizeof(shadowCubeMapUbo));
			ShadowCubeMapLayerIndex layerIndexUbo{};
			layerIndexUbo.layerIndex = i;
			layerIndexUniformBuffers[index]->updateUniformBuffer(&layerIndexUbo, sizeof(layerIndexUbo));
			m_meshes[j]->draw(drawInfo.commandBuffer);
		}
	}
}

void Model::initModel(std::string path, std::shared_ptr<Material> &defaultMaterial)
{
	loadModel(path, defaultMaterial);
}

void Model::initBoxModel(std::shared_ptr<Material> &defaultMaterial)
{
	m_materials.push_back(defaultMaterial);
	m_meshes.push_back(Mesh::createBox());
}

void Model::initSphereModel(std::shared_ptr<Material> &defaultMaterial)
{
	m_materials.push_back(defaultMaterial);
	m_meshes.push_back(Mesh::createSphere());
}

void Model::initPlaneModel(std::shared_ptr<Material> &defaultMaterial)
{
	m_materials.push_back(defaultMaterial);
	m_meshes.push_back(Mesh::createPlane());
}

void Model::initGroundModel(std::shared_ptr<Material> &defaultMaterial)
{
	m_materials.push_back(defaultMaterial);
	m_meshes.push_back(Mesh::createGround());
}

void Model::initCapsuleModel(std::shared_ptr<Material> &defaultMaterial)
{
	m_materials.push_back(defaultMaterial);
	m_meshes.push_back(Mesh::createCapsule());
}

void Model::initCylinderModel(std::shared_ptr<Material> &defaultMaterial)
{
	m_materials.push_back(defaultMaterial);
	m_meshes.push_back(Mesh::createCylinder());
}

void Model::loadModel(std::string path, std::shared_ptr<Material> &defaultMaterial)
{
	// gltf, obj 구별해서 로드하자
	if (path.find(".gltf") != std::string::npos || path.find(".glb") != std::string::npos)
	{
		loadGLTFModel(path, defaultMaterial);
	}
	else if (path.find(".obj") != std::string::npos)
	{
		loadOBJModel(path, defaultMaterial);
	}
}

void Model::loadGLTFModel(std::string path, std::shared_ptr<Material> &defaultMaterial)
{
	Assimp::Importer importer;
	const aiScene *scene =
		importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices |
									aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << "Failed to load GLTF model!" << std::endl;
		// box모델 로드
		m_meshes.push_back(Mesh::createBox());
		m_materials.push_back(defaultMaterial);
		return;
		// throw std::runtime_error("Failed to load GLTF model!");
	}

	// material부터 처리
	std::vector<std::shared_ptr<Material>> materials(scene->mNumMaterials);
	for (unsigned int i = 0; i < scene->mNumMaterials; i++)
	{
		materials[i] = processGLTFMaterial(scene, scene->mMaterials[i], defaultMaterial, path);
	}

	processGLTFSkeleton(scene);
	processGLTFNode(scene->mRootNode, scene, materials);
}

std::shared_ptr<Material> Model::processOBJMaterial(MTL &mtl, std::shared_ptr<Material> &defaultMaterial)
{
	Albedo albedo;
	NormalMap normalMap;
	Roughness roughness;
	Metallic metallic;
	AOMap ao;
	HeightMap heightMap;

	if (mtl.illum >= 1) // albedo, normal, ao, heightmap
	{
		albedo.albedo = glm::vec3(mtl.Ka.x, mtl.Ka.y, mtl.Ka.z);
		if (mtl.map_Kd != "")
		{
			albedo.albedoTexture = Texture::createTexture(mtl.map_Kd);
			albedo.flag = true;
		}
		else
		{
			albedo.albedoTexture = defaultMaterial->getAlbedo().albedoTexture;
			albedo.flag = false;
		}

		if (mtl.map_Bump != "")
		{
			normalMap.normalTexture = Texture::createMaterialTexture(mtl.map_Bump);
			normalMap.flag = true;
		}
		else
		{
			normalMap.normalTexture = defaultMaterial->getNormalMap().normalTexture;
			normalMap.flag = false;
		}

		ao.ao = defaultMaterial->getAOMap().ao;
		if (mtl.map_Ao != "")
		{
			ao.aoTexture = Texture::createMaterialTexture(mtl.map_Ao);
			ao.flag = true;
		}
		else
		{
			ao.aoTexture = defaultMaterial->getAOMap().aoTexture;
			ao.flag = false;
		}

		heightMap.height = defaultMaterial->getHeightMap().height;
		if (mtl.disp != "")
		{
			heightMap.heightTexture = Texture::createMaterialTexture(mtl.disp);
			heightMap.flag = true;
		}
		else
		{
			heightMap.heightTexture = defaultMaterial->getHeightMap().heightTexture;
			heightMap.flag = false;
		}
	}
	else
	{
		albedo.albedo = defaultMaterial->getAlbedo().albedo;
		albedo.albedoTexture = defaultMaterial->getAlbedo().albedoTexture;
		albedo.flag = false;

		normalMap.normalTexture = defaultMaterial->getNormalMap().normalTexture;
		normalMap.flag = false;

		ao.ao = defaultMaterial->getAOMap().ao;
		ao.aoTexture = defaultMaterial->getAOMap().aoTexture;
		ao.flag = false;

		heightMap.height = defaultMaterial->getHeightMap().height;
		heightMap.heightTexture = defaultMaterial->getHeightMap().heightTexture;
		heightMap.flag = false;
	}

	if (mtl.illum >= 2) // roughness, metallic
	{
		roughness.roughness = 1.0f - (mtl.Ns / 1000.0f);
		if (mtl.map_Ns != "")
		{
			roughness.roughnessTexture = Texture::createMaterialTexture(mtl.map_Ns);
			roughness.flag = true;
		}
		else
		{
			roughness.roughnessTexture = defaultMaterial->getRoughness().roughnessTexture;
			roughness.flag = false;
		}

		metallic.metallic = (mtl.Ks.x + mtl.Ks.y + mtl.Ks.z) / 3.0f;
		if (mtl.map_Ks != "")
		{
			metallic.metallicTexture = Texture::createMaterialTexture(mtl.map_Ks);
			metallic.flag = true;
		}
		else
		{
			metallic.metallicTexture = defaultMaterial->getMetallic().metallicTexture;
			metallic.flag = false;
		}
	}
	else
	{
		roughness.roughness = defaultMaterial->getRoughness().roughness;
		roughness.roughnessTexture = defaultMaterial->getRoughness().roughnessTexture;
		roughness.flag = false;

		metallic.metallic = defaultMaterial->getMetallic().metallic;
		metallic.metallicTexture = defaultMaterial->getMetallic().metallicTexture;
		metallic.flag = false;
	}

	return Material::createMaterial(albedo, normalMap, roughness, metallic, ao, heightMap);
}

std::string Model::getMaterialPath(std::string &path, std::string materialPath)
{
	// 모델 경로에서 마지막 슬래시 위치 찾기
	size_t lastSlashPos = path.find_last_of("/\\");
	size_t lastBlackSlachPos = path.find_last_of("\\");
	size_t lastPos = std::max(lastSlashPos, lastBlackSlachPos);
	if (lastPos == std::string::npos)
	{
		throw std::runtime_error("Invalid model path: " + path);
	}

	// 모델 디렉토리 경로 추출
	std::string modelDirectory = path.substr(0, lastPos + 1);

	// 모델 디렉토리와 텍스처 상대 경로 결합
	return modelDirectory + materialPath;
}

std::shared_ptr<Material> Model::processGLTFMaterial(const aiScene *scene, aiMaterial *material,
													 std::shared_ptr<Material> &defaultMaterial, std::string path)
{
	Albedo albedo;
	NormalMap normalMap;
	Roughness roughness;
	Metallic metallic;
	AOMap ao;
	HeightMap heightMap;
	aiString texturePath;
	aiColor4D color;
	float value;

	if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
	{
		albedo.albedo = glm::vec3(color.r, color.g, color.b);
	}
	else
	{
		albedo.albedo = defaultMaterial->getAlbedo().albedo;
	}
	if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
	{
		if (texturePath.C_Str()[0] == '*')
		{
			albedo.albedoTexture =
				Texture::createTextureFromMemory(scene->mTextures[std::stoi(texturePath.C_Str() + 1)]);
		}
		else
		{
			albedo.albedoTexture = Texture::createMaterialTexture(getMaterialPath(path, texturePath.C_Str()));
		}
		albedo.flag = true;
	}
	else
	{
		albedo.albedoTexture = defaultMaterial->getAlbedo().albedoTexture;
		albedo.flag = false;
	}

	if (material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS)
	{
		// normalMap.normalTexture = Texture::createMaterialTexture(getMaterialPath(path, texturePath.C_Str()));
		normalMap.normalTexture = loadMaterialTexture(scene, material, path, texturePath);
		normalMap.flag = true;
	}
	else
	{
		normalMap.normalTexture = defaultMaterial->getNormalMap().normalTexture;
		normalMap.flag = false;
	}

	// Roughness Texture
	if (material->GetTexture(aiTextureType_UNKNOWN, 0, &texturePath) == AI_SUCCESS)
	{
		// roughness.roughnessTexture = Texture::createMaterialTexture(getMaterialPath(path, texturePath.C_Str()));
		roughness.roughnessTexture = loadMaterialTexture(scene, material, path, texturePath);
		roughness.flag = true;
		roughness.roughness = 0.5f;
	}
	else
	{
		roughness.roughnessTexture = defaultMaterial->getRoughness().roughnessTexture;
		roughness.flag = false;
		roughness.roughness = defaultMaterial->getRoughness().roughness;
	}

	// Metallic Texture
	if (material->GetTexture(aiTextureType_UNKNOWN, 0, &texturePath) == AI_SUCCESS)
	{
		// metallic.metallicTexture = Texture::createMaterialTexture(getMaterialPath(path, texturePath.C_Str()));
		metallic.metallicTexture = loadMaterialTexture(scene, material, path, texturePath);
		metallic.flag = true;
		metallic.metallic = defaultMaterial->getMetallic().metallic; // 기본 메탈릭 값 설정
	}
	else
	{
		metallic.metallicTexture = defaultMaterial->getMetallic().metallicTexture;
		metallic.flag = false;
		metallic.metallic = defaultMaterial->getMetallic().metallic; // 기본 메탈릭 값
	}

	// Ambient Occlusion Texture
	if (material->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &texturePath) == AI_SUCCESS)
	{
		// ao.aoTexture = Texture::createMaterialTexture(getMaterialPath(path, texturePath.C_Str()));
		ao.aoTexture = loadMaterialTexture(scene, material, path, texturePath);
		ao.flag = true;
		ao.ao = 1.0f; // 기본 AO 값 설정
	}
	else
	{
		ao.aoTexture = defaultMaterial->getAOMap().aoTexture;
		ao.flag = false;
		ao.ao = defaultMaterial->getAOMap().ao; // 기본 AO 값
	}

	// HeightMap Texture
	if (material->GetTexture(aiTextureType_HEIGHT, 0, &texturePath) == AI_SUCCESS)
	{
		// heightMap.heightTexture = Texture::createMaterialTexture(getMaterialPath(path, texturePath.C_Str()));
		heightMap.heightTexture = loadMaterialTexture(scene, material, path, texturePath);
		heightMap.flag = true;
		heightMap.height = 0.0f; // 기본 Height 값 설정
	}
	else
	{
		heightMap.heightTexture = defaultMaterial->getHeightMap().heightTexture;
		heightMap.flag = false;
		heightMap.height = defaultMaterial->getHeightMap().height; // 기본 Height 값
	}

	return Material::createMaterial(albedo, normalMap, roughness, metallic, ao, heightMap);
}

void Model::processGLTFNode(aiNode *node, const aiScene *scene, std::vector<std::shared_ptr<Material>> &materials)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		uint32_t materialIndex = mesh->mMaterialIndex;
		m_meshes.push_back(std::move(processGLTFMesh(mesh, scene, materials[materialIndex])));
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processGLTFNode(node->mChildren[i], scene, materials);
	}
}

std::shared_ptr<Mesh> Model::processGLTFMesh(aiMesh *mesh, const aiScene *scene, std::shared_ptr<Material> &material)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	vertices.reserve(mesh->mNumVertices);

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex{};
		vertex.pos = {mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z};
		if (mesh->mNormals)
		{
			vertex.normal = {mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z};
		}
		else
		{
			vertex.normal = {0.0f, 0.0f, 0.0f};
		}
		if (mesh->mTextureCoords[0])
		{
			vertex.texCoord = {mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y};
		}
		else
		{
			vertex.texCoord = {0.0f, 0.0f};
		}

		vertex.boneIds = glm::ivec4(-1, -1, -1, -1);
		vertex.weights = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);

		vertices.push_back(vertex);
	}

	if (mesh->mNumBones > 0)
	{
		std::vector<VertexBoneData> vertexBoneData(mesh->mNumVertices);

		// 영향을 미치는 모든 본 정보 처리
		for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
		{
			std::string boneName(mesh->mBones[boneIndex]->mName.C_Str());

			auto it = m_Skeleton->m_NodeNameToBoneIndex.find(boneName);
			if (it == m_Skeleton->m_NodeNameToBoneIndex.end())
			{
				AL_INFO("Model::processGLTFMesh: bone not found: boneName: {0}", boneName);
				continue;
			}

			int targetBoneIndex = it->second;

			for (unsigned int i = 0; i < mesh->mBones[boneIndex]->mNumWeights; ++i)
			{
				unsigned int vertexID = mesh->mBones[boneIndex]->mWeights[i].mVertexId;
				float weight = mesh->mBones[boneIndex]->mWeights[i].mWeight;

				vertexBoneData[vertexID].bones.emplace_back(targetBoneIndex, weight);
			}
		}

		// 정점에 본ids, 가중치 할당
		for (unsigned int vertexIndex = 0; vertexIndex < mesh->mNumVertices; ++vertexIndex)
		{
			auto &bones = vertexBoneData[vertexIndex].bones;

			std::sort(bones.begin(), bones.end(),
					  [](const std::pair<int, float> &a, const std::pair<int, float> &b) -> bool {
						  return a.second > b.second;
					  });

			int numBones = std::min((int)bones.size(), 4);
			float totalWeight = 0.0f;

			for (unsigned int boneIndex = 0; boneIndex < numBones; ++boneIndex)
			{
				vertices[vertexIndex].boneIds[boneIndex] = bones[boneIndex].first;
				vertices[vertexIndex].weights[boneIndex] = bones[boneIndex].second;
				totalWeight += bones[boneIndex].second;
			}

			if (totalWeight > 0.0f)
			{
				for (unsigned int boneIndex = 0; boneIndex < numBones; ++boneIndex)
					vertices[vertexIndex].weights[boneIndex] /= totalWeight;
			}
		}
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	m_materials.push_back(material);
	return Mesh::createMesh(vertices, indices);
}

void Model::processGLTFSkeleton(const aiScene *scene)
{
	if (!scene || !scene->HasAnimations())
	{
		m_SkeletalAnimations = false;
		AL_CORE_INFO("Model::processGLTFSkeleton(): No animations or invalid scene.");
		return;
	}

	m_SkeletalAnimations = true;
	m_Animations = std::make_shared<SkeletalAnimations>();
	m_Skeleton = std::make_shared<Armature::Skeleton>();

	// 1) 본 정보 수집
	std::vector<aiBone *> allAiBones;
	collectAllBones(scene, allAiBones);

	// unique 처리
	buildSkeletonBoneArray(allAiBones);

	// 2) 노드 트리를 통해 본 계층 관계 구성
	loadBone(scene->mRootNode, Armature::NO_PARENT);

	loadAnimations(scene);
}

void Model::collectAllBones(const aiScene *scene, std::vector<aiBone *> &outBones)
{
	for (size_t meshIndex = 0; meshIndex < scene->mNumMeshes; ++meshIndex)
	{
		aiMesh *mesh = scene->mMeshes[meshIndex];
		for (size_t boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
		{
			aiBone *bone = mesh->mBones[boneIndex];
			outBones.push_back(bone);
		}
	}
}

void Model::buildSkeletonBoneArray(const std::vector<aiBone *> &allAiBones)
{
	std::unordered_map<std::string, int> boneNameToIndex;

	for (aiBone *bone : allAiBones)
	{
		std::string boneName = bone->mName.C_Str();

		// 중복 체크
		if (boneNameToIndex.find(boneName) == boneNameToIndex.end())
		{
			Armature::Bone newBone;
			newBone.m_Name = boneName;
			newBone.m_InverseBindMatrix = convertMatrix(bone->mOffsetMatrix);

			m_Skeleton->m_Bones.emplace_back(newBone);
			boneNameToIndex[boneName] = static_cast<int>(m_Skeleton->m_Bones.size() - 1);
		}
	}

	m_Skeleton->m_ShaderData.m_FinalBonesMatrices.resize(m_Skeleton->m_Bones.size());
	m_Skeleton->m_NodeNameToBoneIndex = boneNameToIndex;
}

void Model::loadBone(aiNode *node, int parentBoneIndex)
{
	if (!node)
		return;

	std::string nodeName = node->mName.C_Str();

	// 해당 노드가 본 목록(m_Skeleton->m_NodeNameToBoneIndex) 에 있다면,
	// 해당 본의 부모/자식 연결
	auto it = m_Skeleton->m_NodeNameToBoneIndex.find(nodeName);
	int currentBoneIndex = -1;

	if (it != m_Skeleton->m_NodeNameToBoneIndex.end())
	{
		currentBoneIndex = it->second;
    
		auto& bone = m_Skeleton->m_Bones[currentBoneIndex];

		bone.m_ParentBone = parentBoneIndex;

		if (parentBoneIndex >= 0 && parentBoneIndex < static_cast<int>(m_Skeleton->m_Bones.size()))
			m_Skeleton->m_Bones[parentBoneIndex].m_Children.emplace_back(currentBoneIndex);
	}

	for (size_t childIndex = 0; childIndex < node->mNumChildren; ++childIndex)
		loadBone(node->mChildren[childIndex], currentBoneIndex);
}

void Model::loadAnimations(const aiScene *scene)
{
	size_t numberOfAnimations = scene->mNumAnimations;
	for (size_t animationIndex = 0; animationIndex < numberOfAnimations; ++animationIndex)
	{
		aiAnimation *aiAnim = scene->mAnimations[animationIndex];
		std::string animationName =
			(aiAnim->mName.length > 0 ? std::string(aiAnim->mName.C_Str()) : ("Anim" + std::to_string(animationIndex)));

		std::shared_ptr<SkeletalAnimation> animation = std::make_shared<SkeletalAnimation>(animationName);

		double ticksPerSecond = (aiAnim->mTicksPerSecond != 0.0) ? aiAnim->mTicksPerSecond : 30.0;
		double durationTicks = aiAnim->mDuration;

		float durationSeconds = static_cast<float>(durationTicks / ticksPerSecond);
		size_t numberOfChannels = aiAnim->mNumChannels;
		for (size_t channelIndex = 0; channelIndex < numberOfChannels; ++channelIndex)
		{
			aiNodeAnim *nodeAnim = aiAnim->mChannels[channelIndex];
			std::string nodeName = nodeAnim->mNodeName.C_Str();

			// (A) Translation
			if (nodeAnim->mNumPositionKeys > 0)
			{
				size_t numberOfKeys = nodeAnim->mNumPositionKeys;
				SkeletalAnimation::Sampler samplerPos;

				samplerPos.m_Timestamps.resize(numberOfKeys);
				samplerPos.m_TRSoutputValuesToBeInterpolated.resize(numberOfKeys);

				if (numberOfKeys > 0)
				{
					switch (nodeAnim->mPositionKeys[0].mInterpolation)
					{
					case 0:
						samplerPos.m_Interpolation = SkeletalAnimation::EInterpolationMethod::STEP;
						break;
					case 1:
						samplerPos.m_Interpolation = SkeletalAnimation::EInterpolationMethod::LINEAR;
						break;
					case 3:
						samplerPos.m_Interpolation = SkeletalAnimation::EInterpolationMethod::CUBICSPLINE;
					default:
						AL_ERROR("Model::loadAnimations: No Support Interpolate");
						return;
					}
				}

				for (size_t keyIndex = 0; keyIndex < numberOfKeys; ++keyIndex)
				{
					float timeSec = static_cast<float>(nodeAnim->mPositionKeys[keyIndex].mTime / ticksPerSecond);
					aiVector3D aiPos = nodeAnim->mPositionKeys[keyIndex].mValue;
					glm::vec3 pos(aiPos.x, aiPos.y, aiPos.z);

					samplerPos.m_Timestamps[keyIndex] = timeSec;
					samplerPos.m_TRSoutputValuesToBeInterpolated[keyIndex] = glm::vec4(pos, 0.0f);
				}

				size_t samplerIndexPos = animation->m_Samplers.size();
				animation->m_Samplers.push_back(samplerPos);

				SkeletalAnimation::Channel channel;
				channel.m_samplerIndex = samplerIndexPos;
				channel.m_NodeName = nodeName;
				channel.m_Path = SkeletalAnimation::EPath::TRANSLATION;

				animation->m_Channels.push_back(channel);
			}
			// (B) Rotation
			if (nodeAnim->mNumRotationKeys > 0)
			{
				size_t numberOfKeys = nodeAnim->mNumRotationKeys;
				SkeletalAnimation::Sampler samplerRot;

				samplerRot.m_Timestamps.resize(numberOfKeys);
				samplerRot.m_TRSoutputValuesToBeInterpolated.resize(numberOfKeys);

				if (numberOfKeys > 0)
				{
					switch (nodeAnim->mRotationKeys[0].mInterpolation)
					{
					case 0:
						samplerRot.m_Interpolation = SkeletalAnimation::EInterpolationMethod::STEP;
						break;
					case 1:
						samplerRot.m_Interpolation = SkeletalAnimation::EInterpolationMethod::LINEAR;
						break;
					case 3:
						samplerRot.m_Interpolation = SkeletalAnimation::EInterpolationMethod::CUBICSPLINE;
					default:
						AL_ERROR("Model::loadAnimations: No Support Interpolate");
						return;
					}
				}

				for (size_t keyIndex = 0; keyIndex < numberOfKeys; ++keyIndex)
				{
					float timeSec = static_cast<float>(nodeAnim->mRotationKeys[keyIndex].mTime / ticksPerSecond);
					aiQuaternion aiQuat = nodeAnim->mRotationKeys[keyIndex].mValue;
					glm::quat rot(aiQuat.w, aiQuat.x, aiQuat.y, aiQuat.z);

					samplerRot.m_Timestamps[keyIndex] = timeSec;
					samplerRot.m_TRSoutputValuesToBeInterpolated[keyIndex] = glm::vec4(rot.x, rot.y, rot.z, rot.w);
				}

				size_t samplerIndexRot = animation->m_Samplers.size();
				animation->m_Samplers.push_back(samplerRot);

				SkeletalAnimation::Channel channel;
				channel.m_samplerIndex = samplerIndexRot;
				channel.m_NodeName = nodeName;
				channel.m_Path = SkeletalAnimation::EPath::ROTATION;

				animation->m_Channels.emplace_back(channel);
			}
			// (C) Scale
			if (nodeAnim->mNumScalingKeys > 0)
			{
				size_t numberOfKeys = nodeAnim->mNumScalingKeys;
				SkeletalAnimation::Sampler samplerScl;

				samplerScl.m_Timestamps.resize(numberOfKeys);
				samplerScl.m_TRSoutputValuesToBeInterpolated.resize(numberOfKeys);

				if (numberOfKeys > 0)
				{
					switch (nodeAnim->mScalingKeys[0].mInterpolation)
					{
					case 0:
						samplerScl.m_Interpolation = SkeletalAnimation::EInterpolationMethod::STEP;
						break;
					case 1:
						samplerScl.m_Interpolation = SkeletalAnimation::EInterpolationMethod::LINEAR;
						break;
					case 3:
						samplerScl.m_Interpolation = SkeletalAnimation::EInterpolationMethod::CUBICSPLINE;
					default:
						AL_ERROR("Model::loadAnimations: No Support Interpolate");
						return;
					}
				}

				for (size_t keyIndex = 0; keyIndex < numberOfKeys; ++keyIndex)
				{
					float timeSec = static_cast<float>(nodeAnim->mScalingKeys[keyIndex].mTime / ticksPerSecond);
					aiVector3D aiScale = nodeAnim->mScalingKeys[keyIndex].mValue;
					glm::vec3 scl(aiScale.x, aiScale.y, aiScale.z);

					samplerScl.m_Timestamps[keyIndex] = timeSec;
					samplerScl.m_TRSoutputValuesToBeInterpolated[keyIndex] = glm::vec4(scl, 0.0f);
				}

				size_t samplerIndexScl = animation->m_Samplers.size();
				animation->m_Samplers.push_back(samplerScl);

				SkeletalAnimation::Channel channel;
				channel.m_samplerIndex = samplerIndexScl;
				channel.m_NodeName = nodeName;
				channel.m_Path = SkeletalAnimation::EPath::SCALE;

				animation->m_Channels.emplace_back(channel);
			}
		}

		if (animation->m_Samplers.size() > 2)
		{
			auto &sampler = animation->m_Samplers[0];
			// set duration
			animation->setFirstKeyFrameTime(sampler.m_Timestamps[0]);
			animation->setLastKeyFrameTime(sampler.m_Timestamps.back());
		}

		m_Animations->push(animation);
	}
}

glm::mat4 Model::convertMatrix(const aiMatrix4x4 &m)
{
	return glm::mat4(m.a1, m.b1, m.c1, m.d1, m.a2, m.b2, m.c2, m.d2, m.a3, m.b3, m.c3, m.d3, m.a4, m.b4, m.c4, m.d4);
}

void Model::setShaderData(const std::vector<glm::mat4> &shaderData)
{
	m_ShaderData.m_FinalBonesMatrices = shaderData;
}

std::shared_ptr<SkeletalAnimations>& Model::getAnimations()  { return m_Animations; }
std::shared_ptr<Armature::Skeleton>& Model::getSkeleton() { return m_Skeleton; }

void Model::loadOBJModel(std::string path, std::shared_ptr<Material> &defaultMaterial)
{
	auto objLoader = OBJLoader::ReadFile(path);
	if (!objLoader->getFlag())
	{
		std::cerr << "Failed to load OBJ model!" << std::endl;
		// box모델 로드
		m_meshes.push_back(Mesh::createBox());
		m_materials.push_back(defaultMaterial);
		return;
		// throw std::runtime_error("Failed to load OBJ model!");
	}

	auto &subMeshMap = objLoader->getSubMesh();
	auto &mtlMap = objLoader->getMtlMap();
	for (auto &map : subMeshMap)
	{
		auto &subMesh = map.second;
		auto &vertices = subMesh.vertices;
		auto &indices = subMesh.indices;

		if (mtlMap.find(map.first) != mtlMap.end())
		{
			MTL &mtl = mtlMap[map.first];
			m_materials.push_back(processOBJMaterial(mtl, defaultMaterial));
		}
		else
		{
			m_materials.push_back(defaultMaterial);
		}
		m_meshes.push_back(Mesh::createMesh(vertices, indices));
	}
}

void Model::updateMaterial(std::vector<std::shared_ptr<Material>> materials)
{
	for (size_t i = 0; i < m_materials.size() && i < materials.size(); i++)
	{
		m_materials[i] = materials[i];
	}
}

std::shared_ptr<Texture> Model::loadMaterialTexture(const aiScene *scene, aiMaterial *material, std::string path,
													aiString texturePath)
{
	if (texturePath.C_Str()[0] == '*')
	{
		int textureIndex = std::stoi(texturePath.C_Str() + 1); // "*0" → 0
		const aiTexture *embeddedTexture = scene->mTextures[textureIndex];
		if (!embeddedTexture || !embeddedTexture->pcData)
		{
			throw std::runtime_error("Invalid embedded texture!");
		}
		return Texture::createTextureFromMemory(embeddedTexture);
	}
	else
	{
		return Texture::createMaterialTexture(getMaterialPath(path, texturePath.C_Str()));
	}
}

CullSphere Model::initCullSphere()
{
	glm::vec3 maxPos(-FLT_MAX);
	glm::vec3 minPos(FLT_MIN);
	for (auto &mesh : m_meshes)
	{
		maxPos = glm::max(maxPos, mesh->getMaxPos());
		minPos = glm::min(minPos, mesh->getMinPos());
	}

	return CullSphere((maxPos + minPos) * 0.5f, glm::length(maxPos - minPos) * 0.5f);
}

} // namespace ale