#pragma once
#ifndef _VK_SWAPCHAIN_H
#define _VK_SWAPCHAIN_H
#include <vector>
#include <memory>
#include <map>
#include <set>
#include<optional>
#include <vulkan/vulkan.h>
#include "VKApp.h"
#include "VKUtil.h"
#include "VKValidateLayer.h"

#include "VKAllocator.h"
#include "VKImageView.h"

class VKSwapChain {
public:
	VKSwapChain(VKApp* App) :app(App) {
        create();
	}
    ~VKSwapChain() {
        if (swapChain) {
            clearup();
        }
        
    }
public:
    bool create() {
        return createSwapChain() && createSwapChainImageViews();
    }
    void clearup() {
        if (swapChain)
        {
            DestroySwapChainImageViews();
            DestroySwapChain();
        }
        //delete this;
    }
    VkSwapchainKHR getSwapChain() {
        return swapChain;
    }
    VkImage getSwapChainImage(uint32_t index) {
        if (swapChainImages.empty()) {
            std::cerr << "swap chain image vector was empty!" << std::endl;
            return VK_NULL_HANDLE;
        }
        if (index >= swapChainImages.size()) {
            std::cerr << "input index over swap chain images vector size!" << std::endl;
            return VK_NULL_HANDLE;
        }
        return swapChainImages[index];
    }
    std::vector<VkImage> getSwapChainImages() {
        return swapChainImages;
    }
    VkFormat getSwapChainImageFormat() {
        return swapChainImageFormat;
    }
    VkExtent2D getSwapChainExtent() {
        return swapChainExtent;
    }
    VkSurfaceKHR getSwapChainSurface() {
        return surface;
    }
    uint32_t getSwapChainSize() {
        return static_cast<uint32_t>(swapChainImageViews.size());
    }
    VKImageView* getSwapChainImageView(uint32_t index) {
        if (swapChainImageViews.empty()) {
            std::cerr << " swapChainImageViews vector is empty!" << std::endl;
            return nullptr;
        }
        if (index >= swapChainImageViews.size()) {
            std::cerr << "index over swapChainImageViews vector size!" << std::endl;
            return nullptr;
        }
        return swapChainImageViews[index];
    }


private:
    bool createSwapChain() {
        SwapChainSupportDetails swapChainSupport = app->getDevice()->getSwapChainSupportDetails();
        
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        //决定在交换链中要有多少个image,一般取最小值+1但不能超过最大值
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }
        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = app->getSurface(); //绑定surface
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | 
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT | 
            VK_IMAGE_USAGE_SAMPLED_BIT;

        //查询图形队列和展示队列
        QueueFamilyIndices indices = app->getDevice()->getQueueFamiliesIndices();
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        //image只能在一个队列中使用
        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0; // Optional
            createInfo.pQueueFamilyIndices = nullptr; // Optional
        }
        //交换链支持某种变换 旋转90度，水平翻转等
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;

        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        //是否引用旧交换链
        createInfo.oldSwapchain = VK_NULL_HANDLE;
        //创建交换链
        if (vkCreateSwapchainKHR(app->getDevice()->getLogicalDevice(), 
            &createInfo, 
            nullptr,
            &swapChain) != VK_SUCCESS) {
            std::cerr << "failed to create swap chain!" << std::endl;
            return false;
            //throw std::runtime_error("failed to create swap chain!");
        }
        if (vkGetSwapchainImagesKHR(app->getDevice()->getLogicalDevice(),
            swapChain,
            &imageCount,
            nullptr) != VK_SUCCESS) {
            throw std::runtime_error("failed to get swap chain images!");
        }
        swapChainImages.resize(imageCount);
        if (vkGetSwapchainImagesKHR(app->getDevice()->getLogicalDevice(),
            swapChain,
            &imageCount,
            swapChainImages.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to get swap chain images!");
        }
        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
        return true;
    }
    void DestroySwapChain() {
        if (swapChain) {
            vkDestroySwapchainKHR(app->getDevice()->getLogicalDevice(), 
                swapChain, 
                nullptr);
            swapChain = NULL;
        }
    }

    bool createSwapChainImageViews()
    {
        swapChainImageViews.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++) {
            
            VKImageView* view = new VKImageView(app);
            bool RES = view->createImageView(swapChainImages[i], 
                (uint32_t)1, 
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                swapChainImageFormat, 
                VK_IMAGE_ASPECT_COLOR_BIT);

            if (!RES) {
                std::cerr << "failed to create image views" << std::endl;
                return false;
            }

            swapChainImageViews[i] = view;
        }
        return true;
    }
    void DestroySwapChainImageViews() {
        if (!swapChainImageViews.empty()) {
            for (auto view : swapChainImageViews) {
                delete view;
            }
            swapChainImageViews.clear();
        }
    }

private:
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        /*
        * format:VK_FORMAT_B8G8R8A8_UNORM
        * colorSpace:VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        *查找满足的格式
        */
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        //返回第一个格式，找不到满足的格式也返回
        return availableFormats[0];
    }
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
        //默认使用FIFO
        VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;
        //找到更好的MailBOX，其次 IMMEDIATE
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
            else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
                bestMode = availablePresentMode;
            }
        }

        return bestMode;
    }
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            int width, height;
            glfwGetFramebufferSize(app->getWindow(), &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };
            //VkExtent2D actualExtent = { WIDTH, HEIGHT };

            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

private:
	VKApp* app = nullptr;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;//交换链隐式销毁
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    VkSurfaceKHR surface;

    std::vector<VKImageView*> swapChainImageViews;
    //VkSurfaceFormatKHR  surfaceFormat;
    //VkPresentModeKHR presentMode;
    //VkExtent2D extent;
}; 
#endif //_VK_SWAPCHAIN_H