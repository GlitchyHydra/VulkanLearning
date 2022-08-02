#pragma once

//TODO TRANSFER TO LUA PREMAKE SCRIPT
#define WIN_32

#ifdef WIN_32
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#else 
#endif

#include <vulkan/vulkan.h>

#include "Queue.h"

#include <vector>

struct GLFWwindow;

class Application
{
	GLFWwindow* window;
	
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;

	VkSurfaceKHR surface;

	VkSwapchainKHR swapchain;

	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	VkRenderPass renderPass;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;
	VkCommandPool commandPool;
	VkCommandBuffer commandBuffer;

	VkSemaphore imageAvaliableSemaphore;
	VkSemaphore renderFinishedSemaphore;
	VkFence inFlightFence;

	std::vector<VkFramebuffer> swapChainFramebuffers;

	const std::vector<const char*> validationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> deviceExtensions =
	{
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;

	void setupDebugMessenger();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createWin32Surface();
	void createSwapChain();
	void createImageViews();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createRenderPass();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createCommandPool();
	void createCommandBuffer();
	void createSyncObjects();

	void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	bool checkDeviceExtensionsSupport(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	void initVulkan();
	void createInstance();
	void cleanup();

	bool checkValidationLayerSupport();

	void drawFrame();

public:
	void run();
};