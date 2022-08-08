#pragma once
#ifndef _VMA_EXPORTER
#define _VMA_EXPORTER
#define VMA_IMPLEMENTATION
#define VMA_VULKAN_VERSION 1000000 // Vulkan 1.3
#include "vk_mem_alloc.h"
VmaAllocator getVMAlocator(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device) {
	VmaAllocatorCreateInfo allocatorCreateInfo = {};
	allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_0;
	allocatorCreateInfo.physicalDevice = physicalDevice;
	allocatorCreateInfo.device = device;
	allocatorCreateInfo.instance = instance;
	//allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

	VmaAllocator allocator;
	vmaCreateAllocator(&allocatorCreateInfo, &allocator);
	return allocator;
}
#endif // !_VMA_EXPORTER
