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
    //����
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
        //������
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, app->getSurface(), &details.capabilities);

        //���ʽ
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, app->getSurface(), &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, app->getSurface(), &formatCount, details.formats.data());
        }

        //��չʾģʽ
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
    //�����豸
    bool pickPhysicalDevice() {
        //��������豸��Ŀ
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(app->getInstance(), &deviceCount, nullptr);
        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
            return false;
        }
        //��������豸��Ϣ
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(app->getInstance(), &deviceCount, devices.data());

        //�����Կ�������multimap������ѡ�Կ���ѡ��������ߵ��Կ�
        std::multimap<int, VkPhysicalDevice> candidates;

        for (const auto& device : devices) {
            if (isDeviceSuitable(device)) {
                int score = rateDeviceSuitability(device);
                candidates.insert(std::make_pair(score, device));
            }
        }
        //��ѡ������ߵ��Կ�
        if (candidates.rbegin()->first > 0) {
            physicalDevice = candidates.rbegin()->second;
        }
        else {
            throw std::runtime_error("failed to find a suitable GPU!");
            return false;
        }
        
        //��ӡ�����豸����
        for (auto candidate : candidates) {
            vkGetPhysicalDeviceFeatures(candidate.second, &deviceFeatures);
            vkGetPhysicalDeviceProperties(candidate.second, &deviceProperties);
            std::cout << "device name:" << deviceProperties.deviceName << std::endl;
        }
        //��ȡѡ�е������豸����
        vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        return true;
    }
    //�߼��豸
    bool createLogicalDevice() {
        //�����Ѿ��󶨵������豸�Ķ��м�����Ϣ����
        indices = findQueueFamilies(physicalDevice);
        ////����Ҫ�������߼��豸�Ķ��е���Ϣ
        //VkDeviceQueueCreateInfo queueCreateInfo{};
        //queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        //queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        //queueCreateInfo.queueCount = 1;
        ////�������ȼ�������ֻ��һ������Ҳ��Ҫ�趨
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


        //��ʼ�����߼��豸
        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

        createInfo.pEnabledFeatures = &deviceFeatures;//logicalFeatures

        ////��������չ
        //createInfo.enabledExtensionCount = 0;

        //����VK_KHR_swpchain��չ
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        //��֤�����
        app->getValidationLayer()->adjustVkDeviceCreateInfo(createInfo);
        /*
        �����߼��豸
        ����1�������豸��
        ����2�����м����÷��������ѱ�������
        ����3����ѡ�Ļص�����ָ�룬
        ����4�������߼��豸����ı����ĵ�ַ
        */
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &(this->device)) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }
        /*
        * ����ÿ�����м���Ķ��о�����󶨵�VkQueue  graphicsQueue
        * ����1���߼��豸��
        * ����2�����м��壬
        * ����3������������
        * ����4��������о���ı�����ָ��
        */
        vkGetDeviceQueue((this->device), indices.graphicsFamily.value(), 0, &(this->graphicsQueue));
        vkGetDeviceQueue((this->device), indices.presentFamily.value(), 0, &(this->presentQueue));

        return true;
    }

    int  rateDeviceSuitability(VkPhysicalDevice device) {
        int score = 0;
        //�����豸��������
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        //�����豸��������
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
        //�ж��Ƿ��Ƕ��ԣ������������ں��ԣ�
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        }
        // Maximum possible size of textures affects graphics quality
        score += deviceProperties.limits.maxImageDimension2D;
        // ���Կ���֧�ּ���shaderֱ�ӷ��� 0 �൱���Կ�������
        if (!deviceFeatures.geometryShader) {
            return 0;
        }
        return score;
    }

    bool isDeviceSuitable(VkPhysicalDevice device) {
        //�����豸��������
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        //�����豸��������
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        ////�Կ��Ƿ��Ǽ���shaderר���Կ�
        //return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceFeatures.geometryShader;

        //�ж��Կ��Ķ��м����Ƿ���֧��ͼ����ɫ
        QueueFamilyIndices indices = findQueueFamilies(device);
        //�ж��Կ��Ķ��м����Ƿ���֧�ֵ���չ����������
        bool extensionsSupported = checkDeviceExtensionSupport(device);
        //�ж���չ�Ƿ�֧�ֽ��������������Ƿ��㹻
        bool swapChainAdequate = false;
        //ֻ����֤����չ���ú󣬳��Բ�ѯ������֧��
        if (extensionsSupported) {
            this->swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !this->swapChainSupport.formats.empty() && !this->swapChainSupport.presentModes.empty();
        }
        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        //���м�����Ŀ
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        //���м�����Ϣ
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        //�ҵ�����һ��֧��VK_QUEUE_GRAPHICS_BIT�Ķ��м��塣
        int i = 0;//���кŶ����±�
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }
            //optional�ж��Ƿ����
            if (indices.isComplete()) {
                break;
            }
            //��ѯ�ܹ����ֵ�����surface�Ķ��м���,Ĭ�ϲ�֧��
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, app->getSurface(), &presentSupport);
            //�ж��Ƿ���֧�ֵĶ��м��壬���ض��м��������
            if (queueFamily.queueCount > 0 && presentSupport) {
                indices.presentFamily = i;
            }
            i++;
        }
        return indices;
    }
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        //��չ��Ŀ
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
        //���õ���չ
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        //��Ҫ�����չ ������
        //deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        //ͨ��set.erase�ж�availableExtension���Ƿ�������Ҫ����չ
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
