#pragma once
#ifndef VK_VALIDATELAYER_H
#define VK_VALIDATELAYER_H
#include <vector>
#include <vulkan/vulkan.h>
#include "VKApp.h"
//vkCreateDebugUiltMessgaesEXT函数的代理函数，自动加载
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto  func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

// 销毁VkDebugUtilsMessengerEXT对象的代理函数
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks * pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

class VKValidationLayer 
{
public:
    //回调函数 VKAPI_CALL 实际上是 _stdcall，_stdcall是win32回调函数的修饰符，
    //参数从右到左, 被调用者负责堆栈销毁
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
        (void)pUserData;
        if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
            std::cerr << "message type:" << messageType << " validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }
    const std::vector<const char*> validationlayerSets = { "VK_LAYER_KHRONOS_validation", "VK_LAYER_LUNARG_standard_validation" };

public:

    VKValidationLayer(VKApp* vk_App, bool debug = false) 
        :vkApp(vk_App),vkDebug(debug) {
       
    }
    ~VKValidationLayer() {

    }

    void release() {
        delete this;
    }
public:
    void adjustVkInstanceCreateInfo(VkInstanceCreateInfo& createInfo) {
        if (vkDebug) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
            populateDebugMessengerCreateInfo(vkDebugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT* )&vkDebugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
            createInfo.pNext = nullptr;
        }
    }
    void adjustVkDeviceCreateInfo(VkDeviceCreateInfo& createInfo) {
        if (vkDebug) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else {
            createInfo.enabledLayerCount = 0;
        }
    }
    bool appendValidationLayerSupport() {
        if (!vkDebug)
            return true;

        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        //检查validationLayers是否全部在availableLayers中
        for (const auto& layerProperties : availableLayers) {
            if (isValidationLayer(layerProperties.layerName)) {
                char* ptr = new char[100];
                memset(ptr, 0, 100);
                memcpy(ptr, layerProperties.layerName, strlen(layerProperties.layerName));
                validationLayers.push_back(ptr);
                return true;
            }
        }
        /*for (const char* layerName : validationLayers) {
            bool layerFound = false;
            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }
            if (!layerFound) {
                return false;
            }
        }*/
        return true;
    }
    bool setupDebugMessenger(VkInstance instance) {
        //验证层没有启用直接返回
        if (!vkDebug)
            return true;
        //填写DebugUiltsMessengerEXT的实例debugMessenger的创建信息，必须在创建之前填写
        VkDebugUtilsMessengerCreateInfoEXT createInfo = {};

        populateDebugMessengerCreateInfo(createInfo);
        //createInfo.pUserData = nullptr; // Optional

        //调用代理函数创建debugMessenger的实例，第三个参数分配器设为nullptr
        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
        return true;
    }

    void cleanup(VkInstance instance) {
        if (vkDebug) {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
            for (auto str : validationLayers)
                delete[]str;
        }
    }
private:
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | 
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    bool isValidationLayer(const char* name) {
        auto itr = validationlayerSets.begin();
        while (itr != validationlayerSets.end()) {
            if (strcmp(*itr, name) == 0)
                return true;
            itr++;
        }
        return false;
    }
private:
    bool vkDebug = false;
    VKApp* vkApp = nullptr;
    VkDebugUtilsMessengerCreateInfoEXT vkDebugCreateInfo{};
    std::vector<const char*> validationLayers;
    VkDebugUtilsMessengerEXT debugMessenger = 0;
};

#endif // VK_VALIDATELAYER_H