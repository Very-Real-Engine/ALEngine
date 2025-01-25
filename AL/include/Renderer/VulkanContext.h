#ifndef VULKANCONTEXT_H
#define VULKANCONTEXT_H

#include "Core/Base.h"
#include "Renderer/Common.h"

namespace ale
{
class AL_API VulkanContext
{
public:
	static VulkanContext& getContext();
	~VulkanContext() {}
	void cleanup();
	void initContext(GLFWwindow* window);
	void createSurface(GLFWwindow* window);

	VkInstance getInstance() { return instance; }
	VkDebugUtilsMessengerEXT getDebugMessenger() { return debugMessenger; }
	VkPhysicalDevice getPhysicalDevice() { return physicalDevice; }
	VkDevice getDevice() { return device; }
	VkQueue getGraphicsQueue() { return graphicsQueue; }
	VkQueue getPresentQueue() { return presentQueue; }
	VkCommandPool getCommandPool() { return commandPool; }
	VkSurfaceKHR getSurface() { return surface; }
	VkSampleCountFlagBits getMsaaSamples() { return msaaSamples; }
	VkDescriptorPool getDescriptorPool() { return descriptorPool; }
	uint32_t getQueueFamily();
	VkDescriptorSetLayout getGeometryPassDescriptorSetLayout() { return geometryPassDescriptorSetLayout; }
	VkDescriptorSetLayout getShadowMapDescriptorSetLayout() { return shadowMapDescriptorSetLayout; }
	VkDescriptorSetLayout getShadowCubeMapDescriptorSetLayout() { return shadowCubeMapDescriptorSetLayout; }

	void setGeometryPassDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) { geometryPassDescriptorSetLayout = descriptorSetLayout; }
	void setShadowMapDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) { shadowMapDescriptorSetLayout = descriptorSetLayout; }
	void setShadowCubeMapDescriptorSetLayout(VkDescriptorSetLayout descriptorSetLayout) { shadowCubeMapDescriptorSetLayout = descriptorSetLayout; }
private:
	VulkanContext() { }
	VulkanContext(VulkanContext const&) = delete;
	void operator=(VulkanContext const&) = delete;

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	VkDevice device;
	VkCommandPool commandPool;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout geometryPassDescriptorSetLayout;
	VkDescriptorSetLayout shadowMapDescriptorSetLayout;
	VkDescriptorSetLayout shadowCubeMapDescriptorSetLayout;

	void createInstance();
	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();
	void setupDebugMessenger();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createCommandPool();
	bool isDeviceSuitable(VkPhysicalDevice device);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkSampleCountFlagBits getMaxUsableSampleCount();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	void createDescriptorPool();

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
														VkDebugUtilsMessageTypeFlagsEXT messageType,
														const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
														void* pUserData);
};
} // namespace ale

#endif