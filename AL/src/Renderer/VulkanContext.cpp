#include "Renderer/VulkanContext.h"
#include "ALpch.h"

namespace ale
{
VulkanContext &VulkanContext::getContext()
{
	static VulkanContext context;
	return context;
}

void VulkanContext::initContext(GLFWwindow *window)
{
	createInstance();
	setupDebugMessenger();
	createSurface(window);
	pickPhysicalDevice();
	createLogicalDevice();
	createCommandPool();
	createDescriptorPool();
}

void VulkanContext::cleanup()
{
	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroyDevice(device, nullptr);
	if (enableValidationLayers)
	{
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}
	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);
}

void VulkanContext::createSurface(GLFWwindow *window)
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}
}

void VulkanContext::createInstance()
{
	// 디버그 모드에서 검증 레이어 적용 불가능시 예외 발생
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	// 애플리케이션 정보를 담은 구조체
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// 인스턴스 생성을 위한 정보를 담은 구조체
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	std::vector<const char *> extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	// 디버깅 메시지 객체 생성을 위한 정보 구조체
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

	if (enableValidationLayers)
	{
		// 디버그 모드시 구조체에 검증 레이어 포함
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
		// 인스턴스 생성 및 파괴시에도 검증 가능
		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT *>(&debugCreateInfo);
	}
	else
	{
		// 디버그 모드 아닐 시 검증 레이어 x
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	// 인스턴스 생성
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}
}

// 검증 레이어가 사용 가능한 레이어 목록에 있는지 확인
bool VulkanContext::checkValidationLayerSupport()
{
	// Vulkan 인스턴스에서 사용 가능한 레이어들 목록 생성
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// 필요한 검증 레이어들이 사용 가능 레이어에 포함 되어있는지 확인
	for (const char *layerName : validationLayers)
	{
		bool layerFound = false;
		for (const auto &layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				// 포함 확인!
				layerFound = true;
				break;
			}
		}
		if (!layerFound)
		{
			return false; // 필요한 레이어가 없다면 false 반환
		}
	}
	return true; // 모든 레이어가 지원되면 true 반환
}

/*
GLFW 라이브러리에서 Vulkan 인스턴스를 생성할 때 필요한 인스턴스 확장 목록을 반환
(디버깅 모드시 메시지 콜백 확장 추가)
*/
std::vector<const char *> VulkanContext::getRequiredExtensions()
{
	// 필요한 확장 목록 가져오기
	uint32_t glfwExtensionCount = 0;
	const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	// 디버깅 모드이면 VK_EXT_debug_utils 확장 추가 (메세지 콜백 확장)
	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

// 디버그 메신저 객체 생성
void VulkanContext::setupDebugMessenger()
{
	// 디버그 모드 아니면 return
	if (!enableValidationLayers)
		return;

	// VkDebugUtilsMessengerCreateInfoEXT 구조체 생성
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	// 디버그 메시지 객체 생성
	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

// vkDebugUtilsMessengerCreateInfoEXT 구조체 내부를 채워주는 함수
void VulkanContext::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
								 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
								 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
							 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
							 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

// 적절한 GPU 고르는 함수
void VulkanContext::pickPhysicalDevice()
{
	// GPU 장치 목록 불러오기
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	// 적합한 GPU 탐색
	for (const auto &device : devices)
	{
		if (isDeviceSuitable(device))
		{
			physicalDevice = device;
			msaaSamples = getMaxUsableSampleCount();
			break;
		}
	}

	// 적합한 GPU가 발견되지 않은 경우 에러 발생
	if (physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

// GPU와 소통할 인터페이스인 Logical device 생성
void VulkanContext::createLogicalDevice()
{
	// 그래픽 큐 패밀리의 인덱스를 가져옴
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	// 큐 패밀리의 인덱스들을 set으로 래핑
	std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	// 큐 생성을 위한 정보 설정
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	// 큐 우선순위 0.0f ~ 1.0f 로 표현
	float queuePriority = 1.0f;
	// 큐 패밀리 별로 정보 생성
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// 사용할 장치 기능이 포함된 구조체
	// vkGetPhysicalDeviceFeatures 함수로 디바이스에서 설정 가능한
	// 장치 기능 목록을 확인할 수 있음
	// 일단 지금은 VK_FALSE로 전부 등록함
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE; // 이방성 필터링 사용 설정
	deviceFeatures.sampleRateShading = VK_TRUE; // 디바이스에 샘플 셰이딩 기능 활성화
	deviceFeatures.multiViewport = VK_TRUE;		// 멀티 뷰포트 활성화

	// 논리적 장치 생성을 위한 정보 등록
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;

	// 확장 설정
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	// 구버전 호환을 위해 디버그 모드일 경우
	// 검증 레이어를 포함 시키지만, 현대 시스템에서는 논리적 장치의 레이어를 안 씀
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	// 논리적 장치 생성
	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	// 큐 핸들 가져오기
	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

/*
	[커맨드 풀 생성]
	커맨드 풀이란?
	1. 커맨드 버퍼들을 관리한다.
	2. 큐 패밀리당 1개의 커맨드 풀이 필요하다.
*/
void VulkanContext::createCommandPool()
{
	// 큐 패밀리 인덱스 가져오기
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // 커맨드 버퍼를 개별적으로 재설정할 수 있도록
																	  // 설정 (이게 아니면 커맨드 풀의 모든 커맨드 버퍼
																	  // 설정이 한 번에 이루어짐)
	poolInfo.queueFamilyIndex =
		queueFamilyIndices.graphicsFamily.value(); // 그래픽스 큐 인덱스 등록 (대응시킬 큐 패밀리 등록)

	// 커맨드 풀 생성
	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
}

// 그래픽, 프레젠테이션 큐 패밀리가 존재하는지, GPU와 surface가 호환하는 SwapChain이 존재하는지 검사
bool VulkanContext::isDeviceSuitable(VkPhysicalDevice device)
{
	// 큐 패밀리 확인
	QueueFamilyIndices indices = findQueueFamilies(device);

	// 스왑 체인 확장을 지원하는지 확인
	bool extensionsSupported = checkDeviceExtensionSupport(device);
	bool swapChainAdequate = false;

	// 스왑 체인 확장이 존재하는 경우
	if (extensionsSupported)
	{
		// 물리 디바이스와 surface가 호환하는 SwapChain 정보를 가져옴
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		// GPU와 surface가 지원하는 format과 presentMode가 존재하면 통과
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	// GPU 에서 이방성 필터링을 지원하는지 확인
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

// GPU와 surface가 호환하는 SwapChain 정보를 반환
SwapChainSupportDetails VulkanContext::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	// GPU와 surface가 호환할 수 있는 capability 정보 쿼리
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	// device에서 surface 객체를 지원하는 format이 존재하는지 확인
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	// device에서 surface 객체를 지원하는 presentMode가 있는지 확인
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

// GPU 에서 지원하는 최대 샘플 개수 반환
VkSampleCountFlagBits VulkanContext::getMaxUsableSampleCount()
{
	VkPhysicalDeviceProperties physicalDeviceProperties;
	vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

	// GPU가 지원하는 color 샘플링 개수와 depth 샘플링 개수의 공통 분모를 찾음
	VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
								physicalDeviceProperties.limits.framebufferDepthSampleCounts;

	// 가장 높은 샘플링 개수부터 확인하면서 반환
	if (counts & VK_SAMPLE_COUNT_64_BIT)
	{
		return VK_SAMPLE_COUNT_64_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_32_BIT)
	{
		return VK_SAMPLE_COUNT_32_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_16_BIT)
	{
		return VK_SAMPLE_COUNT_16_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_8_BIT)
	{
		return VK_SAMPLE_COUNT_8_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_4_BIT)
	{
		return VK_SAMPLE_COUNT_4_BIT;
	}
	if (counts & VK_SAMPLE_COUNT_2_BIT)
	{
		return VK_SAMPLE_COUNT_2_BIT;
	}

	return VK_SAMPLE_COUNT_1_BIT;
}

// 디바이스가 지원하는 확장 중
bool VulkanContext::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	// 스왑 체인 확장이 존재하는지 확인
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
	for (const auto &extension : availableExtensions)
	{
		// 지원 가능한 확장들 목록을 순회하며 제거
		requiredExtensions.erase(extension.extensionName);
	}

	// 만약 기존에 있던 확장이 제거되면 true
	// 기존에 있던 확장이 그대로면 지원을 안 하는 것이므로 false
	return requiredExtensions.empty();
}

/*
GPU가 지원하는 큐패밀리 인덱스 가져오기
그래픽스 큐패밀리, 프레젠테이션 큐패밀리 인덱스를 저장
해당 큐패밀리가 없으면 optional 객체에 정보가 empty 상태
*/
QueueFamilyIndices VulkanContext::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	// GPU가 지원하는 큐 패밀리 개수 가져오기
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	// GPU가 지원하는 큐 패밀리 리스트 가져오기
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	// 그래픽 큐 패밀리 검색
	int i = 0;
	for (const auto &queueFamily : queueFamilies)
	{
		// 그래픽 큐 패밀리 찾기 성공한 경우 indices에 값 생성
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		// GPU의 i 인덱스 큐 패밀리가 surface에서 프레젠테이션을 지원하는지 확인
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		// 프레젠테이션 큐 패밀리 등록
		if (presentSupport)
		{
			indices.presentFamily = i;
		}

		// 그래픽 큐 패밀리 찾은 경우 break
		if (indices.isComplete())
		{
			break;
		}

		i++;
	}
	// 그래픽 큐 패밀리를 못 찾은 경우 값이 없는 채로 반환 됨
	return indices;
}

// 디스크립터 풀 생성
void VulkanContext::createDescriptorPool()
{
	size_t MAX_OBJECTS = 1000000;

	// 디스크립터 풀의 타입별 디스크립터 개수를 설정하는 구조체
	std::array<VkDescriptorPoolSize, 5> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; // 유니폼 버퍼 설정
	poolSizes[0].descriptorCount =
		static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * MAX_OBJECTS); // 유니폼 버퍼 디스크립터 최대 개수 설정
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; // 샘플러 설정
	poolSizes[1].descriptorCount =
		static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * MAX_OBJECTS); // 샘플러 디스크립터 최대 개수 설정
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * MAX_OBJECTS);
	poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[3].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * MAX_OBJECTS);
	// image input attachment
	poolSizes[4].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	poolSizes[4].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * MAX_OBJECTS);

	// 디스크립터 풀을 생성할 때 필요한 설정 정보를 담는 구조체
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size()); // 디스크립터 poolSize 구조체 개수
	poolInfo.pPoolSizes = poolSizes.data();							  // 디스크립터 poolSize 구조체 배열
	poolInfo.maxSets =
		static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * MAX_OBJECTS); // 풀에 존재할 수 있는 총 디스크립터 셋 개수
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

	// 디스크립터 풀 생성
	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

// 디버그 메시지 콜백 함수
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanContext::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
															VkDebugUtilsMessageTypeFlagsEXT messageType,
															const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
															void *pUserData)
{
	// 메시지 내용만 출력
	std::cout << "validation layer: " << pCallbackData->pMessage << std::endl;
	// VK_TRUE 반환시 프로그램 종료됨
	return VK_FALSE;
}

uint32_t VulkanContext::getQueueFamily()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	return indices.graphicsFamily.value();
}

} // namespace ale