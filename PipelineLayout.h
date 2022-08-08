#pragma once
#ifndef VK_PIPELINELAYOUT_H
#define VK_PIPELINELAYOUT_H
#include <vector>
#include <vulkan/vulkan.h>
#include "VKApp.h"


//class VK_ContextImpl;

class VKPipelineLayout
{
public:
    VKPipelineLayout(VKApp* vkApp) :app(vkApp) {

    }
    ~VKPipelineLayout() {

    }
public:
    void addPushConstant(const VkPushConstantRange& constantRange, const char* data) {
        if (data) {
            return;
        }
        pushConstantRange.push_back(constantRange);
        pushConstantData.push_back(data);
    }
    VkPipelineLayout getPipelineLayout()const
    {
        return pipelineLayout;
    }
    void create(VkDescriptorSetLayout setLayout) {
        destroy();
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.pNext = nullptr;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &setLayout;
        if (!pushConstantRange.empty()) {
            pipelineLayoutInfo.pushConstantRangeCount = pushConstantRange.size();
            pipelineLayoutInfo.pPushConstantRanges = pushConstantRange.data();
        }
        else {
            pipelineLayoutInfo.pushConstantRangeCount = 0;
            pipelineLayoutInfo.pPushConstantRanges = nullptr;
        }
        VkDevice device = app->getDevice()->getLogicalDevice();
        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("fail to create pipeline layout!");
            
        }
        return;
    }

    void pushConst(VkCommandBuffer commandBuffer) {
        size_t curr = 0;
        for (auto pushConst : pushConstantRange) {
            vkCmdPushConstants(commandBuffer,
                pipelineLayout,
                pushConst.stageFlags,
                pushConst.offset,
                pushConst.size,
                pushConstantData[curr]);
            curr++;
        }
    }
    void release() {
        destroy();
        //delete this;
    }
private:
    void destroy() {
        if (pipelineLayout) {
            vkDestroyPipelineLayout(app->getDevice()->getLogicalDevice(), pipelineLayout, nullptr);
            pipelineLayout = nullptr;
        }
    }
private:
    VKApp* app = nullptr;

    std::vector<VkPushConstantRange> pushConstantRange;
    std::vector<const char*> pushConstantData;

    VkPipelineLayout pipelineLayout = nullptr;
};

#endif // VK_PIPELINELAYOUT_H