#pragma once
#ifndef PUSHDESCRIPTOR_H
#define PUSHDESCRIPTOR_H
#include <vector>
#include <vulkan/vulkan.h>
#include "VKApp.h"
#include "VKDevices.h"
//class VKApp;

class VKPushDescriptor
{
public:
    VKPushDescriptor() = delete;
    VKPushDescriptor(VKApp* vkApp) :app(vkApp){
        vkCmdPushDescriptorSetKHR = (PFN_vkCmdPushDescriptorSetKHR)vkGetDeviceProcAddr(app->getDevice()->getLogicalDevice(), "vkCmdPushDescriptorSetKHR");
    }
public:
    void addDescriptor(const VkWriteDescriptorSet& descriptor) {
        descriptors.push_back(descriptor);
    }
    void push(VkCommandBuffer commandBuffer, VkPipelineLayout layout) {
        if (vkCmdPushDescriptorSetKHR) {
            vkCmdPushDescriptorSetKHR(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, descriptors.data());
         }
    }

private:
    VKApp* app = nullptr;
    PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR = nullptr;
    std::vector<VkWriteDescriptorSet> descriptors;
};

#endif // PUSHDESCRIPTOR_H