#pragma once
#ifndef VK_DESCRIPTORSETS_H
#define VK_DESCRIPTORSETS_H
#include <vector>
#include <list>
#include <map>
#include <vulkan/vulkan.h>
#include "VKApp.h"
#include "VKUniformBuffer.h"
#include "VKImageView.h"


class DescriptorSets
{
public:
    DescriptorSets() = delete;
    DescriptorSets(VKApp* vkApp) :app(vkApp) {

    }
    ~DescriptorSets() {

    }
public:
    void init(VkDescriptorPool pool, VkDescriptorSetLayout setLayout) {
        //分配描述符集数组
        uint32_t frames = (app->getSwapChain())->getSwapChainSize();
        std::vector<VkDescriptorSetLayout> layouts(frames, setLayout);
        VkDescriptorSetAllocateInfo allcoInfo{};
        allcoInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allcoInfo.descriptorPool = pool;
        allcoInfo.descriptorSetCount = frames;
        allcoInfo.pSetLayouts = layouts.data();

        descriptorSets.resize(frames);
        if (vkAllocateDescriptorSets(app->getDevice()->getLogicalDevice(), &allcoInfo, descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptoe sets !");
        }
    }
    void update(const std::list<VKUniformBuffer*>& uniformBuffers,
        const std::map<VKImageView*, uint32_t>& imageViews) {
        uint32_t frames = (app->getSwapChain())->getSwapChainSize();
        VkDevice device = app->getDevice()->getLogicalDevice();
        for (size_t i = 0; i < frames; i++) {
            std::vector<VkWriteDescriptorSet> descriptorWrite;
            for (auto uniform : uniformBuffers) {
                descriptorWrite.push_back(uniform->createWriteDescriptorSet(static_cast<uint32_t>(i), descriptorSets[i]));
            }
            for (auto imageview : imageViews) {
                descriptorWrite.push_back(imageview.first->createWriteDescriptorSet(descriptorSets[i],static_cast<uint32_t>(imageview.second)));
            }
            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrite.size()), &descriptorWrite[0], 0, nullptr);
        }
    }

    void bind(VkCommandBuffer command, VkPipelineLayout pipelineLayout, uint32_t index) {
        //绑定某一帧的描述符集道指定的图形管线的指令缓冲区
        vkCmdBindDescriptorSets(command, VK_PIPELINE_BIND_POINT_GRAPHICS,
            pipelineLayout, 0, 1, &descriptorSets[index], 0, nullptr);
    }
private:
    VKApp* app = nullptr;
    std::vector<VkDescriptorSet> descriptorSets;
};

#endif // VK_DESCRIPTORSETS_H