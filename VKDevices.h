#pragma once
#ifndef _VK_DEVICES_H
#define _VK_DEVICES_H
#include <vector>
#include <memory>
#include <map>
#include <set>
#include <optional>
#include <vulkan/vulkan.h>
#include "VKApp.h"
#include "VKUtil.h"
#include "VKValidateLayer.h"
#include "VKAllocator.h"


class VKDevices {

public:

    VKDevices(VKApp* App) : app(App){
        deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        createVulkanDevice();
    }
    ~VKDevices() {
       
    }
public:
    VkPhysicalDevice getPhysicalDevice() {
        return physicalDevice;
    }
    VkDevice getLogicalDevice() {
        return this->device;
    }
    //队列
    VkQueue getGraphicsQueue() {
        return this->graphicsQueue;
    }

    VkQueue getPresentQueue() {
        return this->presentQueue;
    }
    QueueFamilyIndices getQueueFamiliesIndices() {
        return indices;
    }
    SwapChainSupportDetails getSwapChainSupportDetails() {
        return swapChainSupport;
    }
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;
        //查容量
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, app->getSurface(), &details.capabilities);

        //查格式
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, app->getSurface(), &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, app->getSurface(), &formatCount, details.formats.data());
        }

        //查展示模式
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, app->getSurface(), &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, app->getSurface(), &presentModeCount, details.presentModes.data());
        }
        return details;
    }
    VkPhysicalDeviceProperties getPhysicalDeviceProp() {
        return deviceProperties;
    }
    //find supported image format based physical device
    VkFormat findSupportImageFormat(std::vector<VkFormat> candidates,
        VkImageTiling tiling, VkFormatFeatureFlags features) {
        for (VkFormat candidate : candidates) {
            VkFormatProperties formatProp;
            vkGetPhysicalDeviceFormatProperties(physicalDevice, candidate, &formatProp);
            if (tiling == VK_IMAGE_TILING_LINEAR &&
                (formatProp.linearTilingFeatures & features) == features) {
                return candidate;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                (formatProp.optimalTilingFeatures & features) == features) {
                return candidate;
            }
            else
            {
                throw std::runtime_error("failed to find a supported format in candidates vector!");
            }
        }
        return candidates[0];
    }
    void release() {
        if (device) {
            destroyDevice();
            device = VK_NULL_HANDLE;
        }
        delete this;
    }
private:
    bool createVulkanDevice() {
        return pickPhysicalDevice() &&
            createLogicalDevice();
    }
    void destroyDevice() {
        vkDestroyDevice(device, nullptr);
        for (auto str : deviceExtensions)
            delete[]str;
    }
    //物理设备
    bool pickPhysicalDevice() {
        //检查物理设备数目
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(app->getInstance(), &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
            return false;
        }
        //获得物理设备信息
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(app->getInstance(), &deviceCount, devices.data());

        //根据显卡评分往multimap里插入候选显卡，选择评分最高的显卡
        std::multimap<int, VkPhysicalDevice> candidates;

        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                int score = rateDeviceSuitability(device);
                candidates.insert(std::make_pair(score, device));
            }
        }
        //挑选评分最高的显卡
        if (candidates.rbegin()->first > 0) {
            physicalDevice = candidates.rbegin()->second;
        }
        else {
            throw std::runtime_error("failed to find a suitable GPU!");
            return false;
        }
        
        //打印所有设备属性
        for (auto candidate : candidates) {
            vkGetPhysicalDeviceFeatures(candidate.second, &deviceFeatures);
            vkGetPhysicalDeviceProperties(candidate.second, &deviceProperties);
            std::cout << "device name:" << deviceProperties.deviceName << std::endl;
        }
        //获取选中的物理设备属性
        vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        return true;
    }
    //逻辑设备
    bool createLogicalDevice() {
        //查找已经绑定的物理设备的队列家族信息索引
        indices = findQueueFamilies(physicalDevice);
        ////标明要创建的逻辑设备的队列的信息
        //VkDeviceQueueCreateInfo queueCreateInfo{};
        //queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        //queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        //queueCreateInfo.queueCount = 1;
        ////队列优先级，哪怕只有一个队列也需要设定
        //float queuePriority = 1.0f;
        //queueCreateInfo.pQueuePriorities = &queuePriority;

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }


        //开始创建逻辑设备
        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

        createInfo.pEnabledFeatures = &deviceFeatures;//logicalFeatures

        ////不启用扩展
        //createInfo.enabledExtensionCount = 0;

        //启用VK_KHR_swpchain扩展
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        //验证层相关
        app->getValidationLayer()->adjustVkDeviceCreateInfo(createInfo);
        /*
        创建逻辑设备
        参数1：物理设备，
        参数2：队列及其用法（我们已标明），
        参数3：可选的回调函数指针，
        参数4：保存逻辑设备句柄的变量的地址
        */
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &(this->device)) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }
        /*
        * 检索每个队列家族的队列句柄并绑定到VkQueue  graphicsQueue
        * 参数1：逻辑设备，
        * 参数2：队列家族，
        * 参数3：队列索引，
        * 参数4：保存队列句柄的变量的指针
        */
        vkGetDeviceQueue((this->device), indices.graphicsFamily.value(), 0, &(this->graphicsQueue));
        vkGetDeviceQueue((this->device), indices.presentFamily.value(), 0, &(this->presentQueue));

        return true;
    }

    int  rateDeviceSuitability(VkPhysicalDevice device) {
        int score = 0;
        //物理设备基本属性
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        //物理设备功能特征
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
        //判断是否是独显，独显性能由于核显，
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        }
        // Maximum possible size of textures affects graphics quality
        score += deviceProperties.limits.maxImageDimension2D;
        // 若显卡都支持几何shader直接返回 0 相当于显卡不能用
        if (!deviceFeatures.geometryShader) {
            return 0;
        }
        return score;
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        //物理设备基本属性
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        //物理设备功能特征
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        ////显卡是否是几何shader专用显卡
        //return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;

        //判断显卡的队列家族是否有支持图形着色
        QueueFamilyIndices indices = findQueueFamilies(device);
        //判断显卡的队列家族是否有支持的扩展（交换链）
        bool extensionsSupported = checkDeviceExtensionSupport(device);
        //判断扩展是否支持交换链，交换链是否足够
        bool swapChainAdequate = false;
        //只在验证了扩展可用后，尝试查询交换链支持
        if (extensionsSupported) {
            this->swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !this->swapChainSupport.formats.empty() && !this->swapChainSupport.presentModes.empty();
        }
        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        //队列家族数目
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        //队列家族信息
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        //找到至少一个支持VK_QUEUE_GRAPHICS_BIT的队列家族。
        int i = 0;//队列号对列下标
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }
            //optional判断是否可用
            if (indices.isComplete()) {
                break;
            }
            //查询能够呈现到窗口surface的队列家族,默认不支持
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, app->getSurface(), &presentSupport);
            //判断是否有支持的队列家族，返回队列家族的索引
            if (queueFamily.queueCount > 0 && presentSupport) {
                indices.presentFamily = i;
            }
            i++;
        }
        return indices;
    }
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        //扩展数目
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        //可用的扩展
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        //所要求的扩展 交换链
        //deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        //通过set.erase判断availableExtension里是否有所需要的扩展
        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }
private:
    VKApp* app = nullptr; //external application

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceFeatures deviceFeatures{};
    VkPhysicalDeviceProperties deviceProperties;

    std::vector<const char*> deviceExtensions;
    VkDevice device = nullptr;
    VkPhysicalDeviceFeatures logicalFeatures{};

    SwapChainSupportDetails swapChainSupport;

    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue presentQueue = VK_NULL_HANDLE;

    QueueFamilyIndices indices;
};

#endif // !_VK_DEVICES_H
