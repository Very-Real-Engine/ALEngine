#pragma once

/**
 * @file Common.h
 * @brief 공통 헤더
 */

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <alglm/include/alglm.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

// 헤더위치가 좀 애매
#include "Renderer/Animation/Bones.h"

namespace ale
{
// 동시에 처리할 최대 프레임 수
const int MAX_FRAMES_IN_FLIGHT = 2;

// 검증 레이어 설정
const std::vector<const char *> validationLayers = {"VK_LAYER_KHRONOS_validation"};

// 스왑 체인 확장
const std::vector<const char *> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME,
													VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME};

// 디버그 모드시 검증 레이어 사용
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

/**
 * @brief 디버그 유틸 메시지 생성
 * @param instance 인스턴스
 * @param pCreateInfo 생성 정보
 * @param pAllocator 할당 콜백
 * @param pDebugMessenger 디버그 메시지
 * @return VkResult
 */
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
									  const VkAllocationCallbacks *pAllocator,
									  VkDebugUtilsMessengerEXT *pDebugMessenger);

/**
 * @brief 디버그 유틸 메시지 삭제
 * @param instance 인스턴스
 * @param debugMessenger 디버그 메시지
 * @param pAllocator 할당 콜백
 */
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
								   const VkAllocationCallbacks *pAllocator);

/**
 * @brief 큐 패밀리 인덱스 관리 구조체
 */
struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	/**
	 * @brief 완료 여부 확인
	 * @return 완료 여부
	 */
	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

/**
 * @brief GPU와 surface가 지원하는 SwapChain 지원 세부 정보 구조체
 */
struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

/**
 * @brief 정점 구조체
 */
struct Vertex
{
	alglm::vec3 pos;
	alglm::vec3 normal;
	alglm::vec2 texCoord;
	alglm::vec3 tangent;
	alglm::ivec4 boneIds;
	alglm::vec4 weights;

	/**
	 * @brief 정점 데이터가 전달되는 방법을 알려주는 구조체 반환하는 함수
	 * @return VkVertexInputBindingDescription
	 */
	static VkVertexInputBindingDescription getBindingDescription()
	{
		// 파이프라인에 정점 데이터가 전달되는 방법을 알려주는 구조체
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;				// 버텍스 바인딩 포인트 (현재 0번에 vertex 정보 바인딩)
		bindingDescription.stride = sizeof(Vertex); // 버텍스 1개 단위의 정보 크기
		bindingDescription.inputRate =
			VK_VERTEX_INPUT_RATE_VERTEX; // 정점 데이터 처리 방법
										 // 1. VK_VERTEX_INPUT_RATE_VERTEX : 정점별로 데이터 처리
										 // 2. VK_VERTEX_INPUT_RATE_INSTANCE : 인스턴스별로 데이터 처리
		return bindingDescription;
	}

	/**
	 * @brief 정점 속성별 데이터 형식과 위치를 지정하는 구조체 반환하는 함수
	 * @return std::array<VkVertexInputAttributeDescription, 6>
	 */
	static std::array<VkVertexInputAttributeDescription, 6> getAttributeDescriptions()
	{
		// 정점 속성의 데이터 형식과 위치를 지정하는 구조체
		std::array<VkVertexInputAttributeDescription, 6> attributeDescriptions{};

		// pos 속성 정보 입력
		attributeDescriptions[0].binding = 0;  // 버텍스 버퍼의 바인딩 포인트
		attributeDescriptions[0].location = 0; // 버텍스 셰이더의 어떤 location에 대응되는지 지정
		attributeDescriptions[0].format =
			VK_FORMAT_R32G32B32_SFLOAT; // 저장되는 데이터 형식 (VK_FORMAT_R32G32B32_SFLOAT = vec3)
		attributeDescriptions[0].offset = offsetof(Vertex, pos); // 버텍스 구조체에서 해당 속성이 시작되는 위치

		// normal 속성 정보 입력
		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, normal);

		// texCoord 속성 정보 입력
		attributeDescriptions[2].binding = 0;
		attributeDescriptions[2].location = 2;
		attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

		// tangent 속성 정보 입력
		attributeDescriptions[3].binding = 0;
		attributeDescriptions[3].location = 3;
		attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[3].offset = offsetof(Vertex, tangent);

		// boneIds 속성 정보 입력
		attributeDescriptions[4].binding = 0;
		attributeDescriptions[4].location = 4;
		attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SINT;
		attributeDescriptions[4].offset = offsetof(Vertex, boneIds);

		// weights 속성 정보 입력
		attributeDescriptions[5].binding = 0;
		attributeDescriptions[5].location = 5;
		attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[5].offset = offsetof(Vertex, weights);

		return attributeDescriptions;
	}
};

/**
 * @brief 유니폼 버퍼 객체
 */
struct UniformBufferObject
{
	ALIGN16 alglm::mat4 model;
	ALIGN16 alglm::mat4 view;
	ALIGN16 alglm::mat4 proj;
};

/**
 * @brief Geometry Pass 정점 유니폼 버퍼 객체
 */
struct GeometryPassVertexUniformBufferObject
{
	ALIGN16 alglm::mat4 model; // 64바이트
	ALIGN16 alglm::mat4 view;  // 64바이트
	ALIGN16 alglm::mat4 proj;  // 64바이트
	ALIGN16 alglm::mat4 finalBonesMatrices[MAX_BONES];
	ALIGN4 bool heightFlag;	  // 4바이트
	ALIGN4 float heightScale; // 4바이트
	ALIGN4 uint32_t padding1;
	ALIGN4 uint32_t padding2; // 8바이트 (패딩)
};

/**
 * @brief Geometry Pass 프래그먼트 유니폼 버퍼 객체
 */
struct GeometryPassFragmentUniformBufferObject
{
	ALIGN16 alglm::vec4 albedoValue; // 16바이트 (정렬 우선순위)
	ALIGN4 float roughnessValue;
	ALIGN4 float metallicValue;
	ALIGN4 float aoValue;

	ALIGN4 bool albedoFlag;
	ALIGN4 bool normalFlag;
	ALIGN4 bool roughnessFlag;
	ALIGN4 bool metallicFlag;
	ALIGN4 bool aoFlag;
	ALIGN4 uint32_t padding1;
	ALIGN4 uint32_t padding2; // 패딩 추가 (8바이트)
};

/**
 * @brief 광원 구조체
 */
struct Light
{
	ALIGN4 float intensity;	  // 광원의 강도
	ALIGN4 float innerCutoff; // 스포트라이트 내부 각도 (cosine 값)
	ALIGN4 float outerCutoff; // 스포트라이트 외부 각도 (cosine 값)
	ALIGN4 uint32_t type;	  // 광원 타입 (0: 점광원, 1: 스포트라이트, 2: 방향성 광원)
	ALIGN4 uint32_t onShadowMap;
	ALIGN4 uint32_t padding1;
	ALIGN4 uint32_t padding2;
	ALIGN4 uint32_t padding3;
	alglm::vec3 position;  // 광원의 위치 (점광원, 스포트라이트)
	alglm::vec3 direction; // 광원의 방향 (스포트라이트, 방향성 광원)
	alglm::vec3 color;	   // 광원의 색상
};

/**
 * @brief Lighting Pass 유니폼 버퍼 객체
 */
struct LightingPassUniformBufferObject
{
	ALIGN16 Light lights[16];	   // 최대 16개의 광원 정보
	ALIGN16 alglm::vec3 cameraPos; // 카메라 위치
	ALIGN16 alglm::mat4 view[4][6];
	ALIGN16 alglm::mat4 proj[4];
	ALIGN4 uint32_t numLights;	  // 활성화된 광원 개수
	ALIGN4 float ambientStrength; // 주변광 강도
	ALIGN4 uint32_t padding1;
	ALIGN4 uint32_t padding2;
};

/**
 * @brief Shadow Map 유니폼 버퍼 객체
 */
struct ShadowMapUniformBufferObject
{
	ALIGN16 alglm::mat4 proj;
	ALIGN16 alglm::mat4 view;
	ALIGN16 alglm::mat4 model;
};

/**
 * @brief Shadow Cube Map 유니폼 버퍼 객체
 */
struct ShadowCubeMapUniformBufferObject
{
	ALIGN16 alglm::mat4 proj;
	ALIGN16 alglm::mat4 view[6];
	ALIGN16 alglm::mat4 model;
};

/**
 * @brief Shadow Cube Map 레이어 인덱스 유니폼 버퍼 객체
 */
struct ShadowCubeMapLayerIndex
{
	ALIGN4 uint32_t layerIndex;
};

/**
 * @brief Spherical Map 유니폼 버퍼 객체
 */
struct SphericalMapUniformBufferObject
{
	ALIGN16 alglm::mat4 transform;
	ALIGN4 uint32_t layerIndex;
};

/**
 * @brief Background 유니폼 버퍼 객체
 */
struct BackgroundUniformBufferObject
{
	ALIGN16 alglm::mat4 proj;
	ALIGN16 alglm::mat4 view;
};

/**
 * @brief Collider 유니폼 버퍼 객체
 */
struct ColliderUniformBufferObject
{
	ALIGN16 alglm::mat4 model;
	ALIGN16 alglm::mat4 view;
	ALIGN16 alglm::mat4 proj;
	ALIGN16 alglm::vec3 color;
};

/**
 * @brief Shadow Map UBO
 */
struct ShadowMapUBO
{
	ALIGN16 alglm::mat4 proj;
	ALIGN16 alglm::mat4 view;
};

/**
 * @brief Shadow Cube Map UBO
 */
struct ShadowCubeMapUBO
{
	ALIGN16 alglm::mat4 proj;
	ALIGN16 alglm::mat4 view[6];
};

/**
 * @brief Shadow Map SSBO
 */
struct ShadowMapSSBO
{
	ALIGN16 alglm::mat4 model;
};

/**
 * @brief Shadow Cube Map Push Constants
 */
struct ShadowCubeMapPushConstants
{
	ALIGN4 uint32_t layerIndex;
};

} // namespace ale
