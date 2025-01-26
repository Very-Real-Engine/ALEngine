#include "Renderer/Model.h"
#include "Renderer/ShaderResourceManager.h"

namespace ale
{
std::shared_ptr<Model> Model::createModel(std::string path, std::shared_ptr<Material> &defaultMaterial)
{
	std::shared_ptr<Model> model = std::shared_ptr<Model>(new Model());
	model->initModel(path, defaultMaterial);
	return model;
}

std::shared_ptr<Model> Model::createBoxModel(std::shared_ptr<Material> &defaultMaterial)
{
	std::shared_ptr<Model> model = std::shared_ptr<Model>(new Model());
	model->initBoxModel(defaultMaterial);
	return model;
}

std::shared_ptr<Model> Model::createSphereModel(std::shared_ptr<Material> &defaultMaterial)
{
	std::shared_ptr<Model> model = std::shared_ptr<Model>(new Model());
	model->initSphereModel(defaultMaterial);
	return model;
}

std::shared_ptr<Model> Model::createPlaneModel(std::shared_ptr<Material> &defaultMaterial)
{
	std::shared_ptr<Model> model = std::shared_ptr<Model>(new Model());
	model->initPlaneModel(defaultMaterial);
	return model;
}

void Model::cleanup()
{
	for (auto &mesh : m_meshes)
	{
		mesh->cleanup();
	}
}

void Model::draw(DrawInfo& drawInfo)
{
	auto& descriptorSets = drawInfo.shaderResourceManager->getDescriptorSets();
	auto& vertexUniformBuffers = drawInfo.shaderResourceManager->getVertexUniformBuffers();
	auto& fragmentUniformBuffers = drawInfo.shaderResourceManager->getFragmentUniformBuffers();
	for (uint32_t i = 0; i < m_meshes.size(); i++)
	{
		uint32_t index = MAX_FRAMES_IN_FLIGHT * i + drawInfo.currentFrame;
        vkCmdBindDescriptorSets(drawInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, drawInfo.pipelineLayout, 0, 1, &descriptorSets[index], 0, nullptr);
		GeometryPassVertexUniformBufferObject vertexUbo{};
		vertexUbo.model = drawInfo.model;
		vertexUbo.view = drawInfo.view;
		vertexUbo.proj = drawInfo.projection;
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

void Model::drawShadow(ShadowMapDrawInfo& drawInfo)
{
	auto& descriptorSets = drawInfo.shaderResourceManager->getDescriptorSets();
	auto& uniformBuffers = drawInfo.shaderResourceManager->getUniformBuffers();
	for (uint32_t i = 0; i < m_meshes.size(); i++)
	{
		uint32_t index = drawInfo.currentFrame;
		vkCmdBindDescriptorSets(drawInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, drawInfo.pipelineLayout, 0, 1, &descriptorSets[index], 0, nullptr);
		ShadowMapUniformBufferObject shadowMapUbo{};
		shadowMapUbo.model = drawInfo.model;
		shadowMapUbo.view = drawInfo.view;
		shadowMapUbo.proj = drawInfo.projection;
		uniformBuffers[index]->updateUniformBuffer(&shadowMapUbo, sizeof(shadowMapUbo));
		m_meshes[i]->draw(drawInfo.commandBuffer);
	}
}

void Model::drawShadowCubeMap(ShadowCubeMapDrawInfo& drawInfo) {
	auto& descriptorSets = drawInfo.shaderResourceManager->getDescriptorSets();
	auto& uniformBuffers = drawInfo.shaderResourceManager->getUniformBuffers();
	auto& layerIndexUniformBuffers = drawInfo.shaderResourceManager->getLayerIndexUniformBuffers();
	uint32_t currentFrame = drawInfo.currentFrame;
	ShadowCubeMapUniformBufferObject shadowCubeMapUbo{};
	shadowCubeMapUbo.model = drawInfo.model;
	shadowCubeMapUbo.proj = drawInfo.projection;
	for (uint32_t i = 0; i < 6; i++) {
		shadowCubeMapUbo.view[i] = drawInfo.view[i];
	}

	for (uint32_t i = 0; i < 6; i++) {
		uint32_t index = currentFrame * 6 + i;
		for (uint32_t j = 0; j < m_meshes.size(); j++) {
			vkCmdBindDescriptorSets(drawInfo.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, drawInfo.pipelineLayout, 0, 1, &descriptorSets[index], 0, nullptr);
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

void Model::loadModel(std::string path, std::shared_ptr<Material> &defaultMaterial)
{
	// gltf, obj 구별해서 로드하자
	if (path.find(".gltf") != std::string::npos || path.find(".glb") != std::string::npos) {
		loadGLTFModel(path, defaultMaterial);
	}
	else if (path.find(".obj") != std::string::npos) {
		loadOBJModel(path, defaultMaterial);
	}
}

void Model::loadGLTFModel(std::string path, std::shared_ptr<Material>& defaultMaterial) {
	// Assimp를 사용하여 OBJ 파일 로드
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, 
	aiProcess_Triangulate |
    aiProcess_FlipUVs |
    aiProcess_JoinIdenticalVertices |
    aiProcess_OptimizeMeshes |
    aiProcess_OptimizeGraph);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Failed to load GLTF model!" << std::endl;
        throw std::runtime_error("Failed to load GLTF model!");
    }

	// material부터 처리
	std::vector<std::shared_ptr<Material>> materials(scene->mNumMaterials);
	for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
		materials[i] = processGLTFMaterial(scene, scene->mMaterials[i], defaultMaterial, path);
	}

	processGLTFNode(scene->mRootNode, scene, materials);
}

std::string Model::getMaterialPath(std::string &path, std::string materialPath) {
	// 모델 경로에서 마지막 슬래시 위치 찾기
    size_t lastSlashPos = path.find_last_of("/\\");
    if (lastSlashPos == std::string::npos) {
        throw std::runtime_error("Invalid model path: " + path);
    }

    // 모델 디렉토리 경로 추출
    std::string modelDirectory = path.substr(0, lastSlashPos + 1);

    // 모델 디렉토리와 텍스처 상대 경로 결합
    return modelDirectory + materialPath;
}

std::shared_ptr<Material> Model::processGLTFMaterial(const aiScene* scene, aiMaterial *material, std::shared_ptr<Material> &defaultMaterial, std::string path) {

	Albedo albedo;
	NormalMap normalMap;
	Roughness roughness;
	Metallic metallic;
	AOMap ao;
	HeightMap heightMap;

	aiString texturePath;
    aiColor4D color;
    float value;
	
	if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
		albedo.albedo = glm::vec3(color.r, color.g, color.b);
	}
	else {
		albedo.albedo = defaultMaterial->getAlbedo().albedo;
	}
	if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS) {
		if (texturePath.C_Str()[0] == '*') {
			albedo.albedoTexture = Texture::createTextureFromMemory(scene->mTextures[std::stoi(texturePath.C_Str() + 1)]);
		}
		else {
			albedo.albedoTexture = Texture::createMaterialTexture(getMaterialPath(path, texturePath.C_Str()));
		}
		albedo.flag = true;
	}
	else {
		albedo.albedoTexture = defaultMaterial->getAlbedo().albedoTexture;
		albedo.flag = false;
	}

    if (material->GetTexture(aiTextureType_NORMALS, 0, &texturePath) == AI_SUCCESS) {
		// normalMap.normalTexture = Texture::createMaterialTexture(getMaterialPath(path, texturePath.C_Str()));
		normalMap.normalTexture = loadMaterialTexture(scene, material, path, texturePath);
		normalMap.flag = true;
    } else {
        normalMap.normalTexture = defaultMaterial->getNormalMap().normalTexture;
        normalMap.flag = false;
    }

	// Roughness Texture
    if (material->GetTexture(aiTextureType_UNKNOWN, 0, &texturePath) == AI_SUCCESS) {
		// roughness.roughnessTexture = Texture::createMaterialTexture(getMaterialPath(path, texturePath.C_Str()));
		roughness.roughnessTexture = loadMaterialTexture(scene, material, path, texturePath);
        roughness.flag = true;
		roughness.roughness = 0.5f;
    } else {
        roughness.roughnessTexture = defaultMaterial->getRoughness().roughnessTexture;
        roughness.flag = false;
		roughness.roughness = defaultMaterial->getRoughness().roughness;
    }

	// Metallic Texture
	if (material->GetTexture(aiTextureType_UNKNOWN, 0, &texturePath) == AI_SUCCESS) {
		// metallic.metallicTexture = Texture::createMaterialTexture(getMaterialPath(path, texturePath.C_Str()));
		metallic.metallicTexture = loadMaterialTexture(scene, material, path, texturePath);
		metallic.flag = true;
		metallic.metallic = defaultMaterial->getMetallic().metallic; // 기본 메탈릭 값 설정
	} else {
		metallic.metallicTexture = defaultMaterial->getMetallic().metallicTexture;
		metallic.flag = false;
		metallic.metallic = defaultMaterial->getMetallic().metallic; // 기본 메탈릭 값
	}

	// Ambient Occlusion Texture
	if (material->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &texturePath) == AI_SUCCESS) {
		// ao.aoTexture = Texture::createMaterialTexture(getMaterialPath(path, texturePath.C_Str()));
		ao.aoTexture = loadMaterialTexture(scene, material, path, texturePath);
		ao.flag = true;
		ao.ao = 1.0f; // 기본 AO 값 설정
	} else {
		ao.aoTexture = defaultMaterial->getAOMap().aoTexture;
		ao.flag = false;
		ao.ao = defaultMaterial->getAOMap().ao; // 기본 AO 값
	}

	// HeightMap Texture
	if (material->GetTexture(aiTextureType_HEIGHT, 0, &texturePath) == AI_SUCCESS) {
		// heightMap.heightTexture = Texture::createMaterialTexture(getMaterialPath(path, texturePath.C_Str()));
		heightMap.heightTexture = loadMaterialTexture(scene, material, path, texturePath);
		heightMap.flag = true;
		heightMap.height = 0.0f; // 기본 Height 값 설정
	} else {
		heightMap.heightTexture = defaultMaterial->getHeightMap().heightTexture;
		heightMap.flag = false;
		heightMap.height = defaultMaterial->getHeightMap().height; // 기본 Height 값
	}

	return Material::createMaterial(albedo, normalMap, roughness, metallic, ao, heightMap);
}

void Model::processGLTFNode(aiNode *node, const aiScene *scene, std::vector<std::shared_ptr<Material>> &materials) {
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

std::shared_ptr<Mesh> Model::processGLTFMesh(aiMesh *mesh, const aiScene *scene, std::shared_ptr<Material> &material) {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

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
		vertices.push_back(vertex);
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

void Model::loadOBJModel(std::string path, std::shared_ptr<Material>& defaultMaterial) {
    // Assimp를 사용하여 OBJ 파일 로드
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, 
	aiProcess_Triangulate |
    aiProcess_FlipUVs |
    aiProcess_JoinIdenticalVertices |
    aiProcess_OptimizeMeshes |
    aiProcess_OptimizeGraph);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "Failed to load OBJ model!" << std::endl;
        throw std::runtime_error("Failed to load OBJ model!");
    }

    processOBJNode(scene->mRootNode, scene, defaultMaterial);
}

void Model::processOBJNode(aiNode *node, const aiScene *scene, std::shared_ptr<Material> &defaultMaterial)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
		m_meshes.push_back(std::move(processOBJMesh(mesh, scene, defaultMaterial)));
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processOBJNode(node->mChildren[i], scene, defaultMaterial);
	}
}

std::shared_ptr<Mesh> Model::processOBJMesh(aiMesh *mesh, const aiScene *scene, std::shared_ptr<Material> &defaultMaterial)
{
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

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
		vertices.push_back(vertex);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	m_materials.push_back(defaultMaterial);
	return Mesh::createMesh(vertices, indices);
}

void Model::updateMaterial(std::vector<std::shared_ptr<Material>> materials)
{
	for (size_t i = 0; i < m_materials.size() && i < materials.size(); i++) {
		m_materials[i] = materials[i];
	}
}

std::shared_ptr<Texture> Model::loadMaterialTexture(const aiScene* scene, aiMaterial *material, std::string path, aiString texturePath) {
	if (texturePath.C_Str()[0] == '*') {
		int textureIndex = std::stoi(texturePath.C_Str() + 1); // "*0" → 0
		const aiTexture* embeddedTexture = scene->mTextures[textureIndex];
		if (!embeddedTexture || !embeddedTexture->pcData) {
			throw std::runtime_error("Invalid embedded texture!");
		}
		return Texture::createTextureFromMemory(embeddedTexture);
	}
	else {
		return Texture::createMaterialTexture(getMaterialPath(path, texturePath.C_Str()));
	}
}

} // namespace ale