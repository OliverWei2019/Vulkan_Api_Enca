#pragma once
#ifndef VK_DESCRIPTORSETLAYOUT_H
#define VK_DESCRIPTORSETLAYOUT_H
#include <vulkan/vulkan.h>
#include "VKApp.h"
#include "ShaderSet.h"


class DescriptorSetLayout
{
public:
    DescriptorSetLayout() = delete;
    DescriptorSetLayout(VKApp* vkApp, VKShaderSet* shaderSet) :app(vkApp){
        VkDevice device = app->getDevice()->getLogicalDevice();

        VkDescriptorSetLayoutCreateInfo createLayoutInfo{};
        createLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createLayoutInfo.bindingCount = static_cast<uint32_t>(shaderSet->getDescriptorSetLayoutBindingCount());
        createLayoutInfo.pBindings = shaderSet->getDescriptorSetLayoutBindingData();

        if (vkCreateDescriptorSetLayout(device, &createLayoutInfo, nullptr, &descriptorSetLayout)
            != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptorset layout!");
        }
    }
    ~DescriptorSetLayout() {

    }
public:
    VkDescriptorSetLayout getDescriptorSetLayout() {
        return descriptorSetLayout;
    }
    void release() {
        if (descriptorSetLayout)
        {
            vkDestroyDescriptorSetLayout(app->getDevice()->getLogicalDevice(), descriptorSetLayout, nullptr);
        }
        descriptorSetLayout = nullptr;
        delete this;
    }
private:
    VKApp* app = nullptr;
    VkDescriptorSetLayout descriptorSetLayout = nullptr;
};

#endif // VK_DESCRIPTORSETLAYOUT_H