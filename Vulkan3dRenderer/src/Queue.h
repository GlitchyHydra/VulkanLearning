#pragma once

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value() &&
			presentFamily.has_value();
	}
};

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	//The VkQueueFamilyProperties struct contains some details about the queue family,
	//including the type of operations that are supported and the number of queues that can be created based on that family.
	//We need to find at least one queue family that supports VK_QUEUE_GRAPHICS_BIT
	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport)
		{
			indices.presentFamily = i;
		}

		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		if (indices.isComplete())
			break;

		i++;
	}

	return indices;
}

//Querying details of swap chain support
struct SwapChainSupportDetails
{
	//Basic surface capabilities (min/max number of images in swap chain, min/max width and height of images)
	VkSurfaceCapabilitiesKHR capapibilities;
	//Surface formats (pixel format, color space)
	std::vector<VkSurfaceFormatKHR> formats; 
	//Available presentation modes
	std::vector<VkPresentModeKHR> presentModes;
};

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	SwapChainSupportDetails details;

	//querying the supported surface formats
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capapibilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	//querying the supported presentation modes
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}


	return details;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& avaliableFormats)
{
	//Each VkSurfaceFormatKHR entry contains a format and a colorSpace member.
	//The format member specifies the color channels and types
	//The colorSpace member indicates if the SRGB color space is supported or not using the VK_COLOR_SPACE_SRGB_NONLINEAR_KHR flag
	for (const auto& availableFormat : avaliableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return avaliableFormats[0];
}

VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& avaliablePresentModes)
{
	/*
	* VK_PRESENT_MODE_IMMEDIATE_KHR: Images submitted by your application
	are transferred to the screen right away, which may result in tearing.
	* 
VK_PRESENT_MODE_FIFO_KHR: The swap chain is a queue where the display takes an image from
the front of the queue when the display is refreshed and the program inserts rendered images at the back of the queue.
If the queue is full then the program has to wait. This is most similar to vertical sync as found in modern games.
The moment that the display is refreshed is known as "vertical blank".

VK_PRESENT_MODE_FIFO_RELAXED_KHR: This mode only differs from the previous one if the application
is late and the queue was empty at the last vertical blank. Instead of waiting for the next vertical blank,
the image is transferred right away when it finally arrives. This may result in visible tearing.

VK_PRESENT_MODE_MAILBOX_KHR: This is another variation of the second mode.
Instead of blocking the application when the queue is full,
the images that are already queued are simply replaced with the newer ones.
This mode can be used to render frames as fast as possible while still avoiding tearing,
resulting in fewer latency issues than standard vertical sync. This is commonly known as "triple buffering",
although the existence of three buffers alone does not necessarily mean that the framerate is unlocked.
	*/

	for (const auto& availablePresentMode : avaliablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

