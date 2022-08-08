#pragma once
#ifndef VK_DESCRIPTORPOOL_H
#define VK_DESCRIPTORPOOL_H
#include <vulkan/vulkan.h>
#include "VKApp.h"
#include "VKSwapChain.h"
#include "ShaderSet.h"

class DescriptorPool
{
public:
    DescriptorPool(VKApp* vkApp) :app(vkApp) {

    }
    ~DescriptorPool() {}
public:
    void create(VKShaderSet* shaderSet) {
        uint32_t frames = app->getSwapChain()->getSwapChainSize();
        shaderSet->updateDescriptorPoolSize(frames);

        VkDescriptorPoolSize poolSize{};
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = frames;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        if (shaderSet->getDescriptorPoolSizeCount() > 0)
        {
            poolInfo.poolSizeCount = static_cast<uint32_t>(shaderSet->getDescriptorPoolSizeCount());
            poolInfo.pPoolSizes = shaderSet->getDescriptorPoolSizeData();
        }
        else {
            poolInfo.poolSizeCount = 1;
            poolInfo.pPoolSizes = &poolSize;
        }
        poolInfo.maxSets = frames;

        VkDevice device = app->getDevice()->getLogicalDevice();
        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }
    VkDescriptorPool getDescriptorPool() {
        return descriptorPool;
    }
    void release() {
        if (descriptorPool) {
            vkDestroyDescriptorPool(app->getDevice()->getLogicalDevice(), descriptorPool, nullptr);
        }

    }

private:
    VKApp* app = nullptr;
    VkDescriptorPool descriptorPool = nullptr;
};

#endif // VK_DESCRIPTORPOOL_H