#pragma once
#ifndef VK_CMDPOOL_H
#define VK_CMDPOOL_H
#include <vulkan/vulkan.h>
#include "VKApp.h"
#include "VKUtil.h"
#include "VKAllocator.h"
#include "VKSecCmdBuffer.h"
#include "PoolBase.h"
class VKSecCmdBuffer;

class VKCmdPool:public PoolBase
{
public:
    VKCmdPool(VKApp* vkApp, const QueueFamilyIndices& index) :app(vkApp) {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = index.graphicsFamily.value();
        
        if (vkCreateCommandPool(app->getDevice()->getLogicalDevice(), 
            &poolInfo, nullptr, &pool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }
    ~VKCmdPool() {
       // vkDestroyCommandPool(app->getDevice()->getLogicalDevice(), pool, nullptr);
        
    }
public:
    
    VkCommandBuffer beginSingleTimeCommands(uint32_t flag = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) override {
        //录制commanfBuffer起始配置
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = pool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        if (vkAllocateCommandBuffers(app->getDevice()->getLogicalDevice(),
            &allocInfo, 
            &commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = flag;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin record copy buffer command !");
        }
        return commandBuffer;
    }
    void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue) override  {
        //录制commanfBuffer结束配置
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        if (vkQueueSubmit(queue,
            1, 
            &submitInfo, 
            VK_NULL_HANDLE) != VK_SUCCESS) {
            throw std::runtime_error("failed to submite copybuffer command queue !");
        }
        vkQueueWaitIdle(queue);
        //把copyBuffer command 还给 command pool
        vkFreeCommandBuffers(app->getDevice()->getLogicalDevice(), pool, 1, &commandBuffer);
    }
    VkCommandPool getCommandPool() override  {
        return pool;
    }

    VKSecCmdBuffer* createSecondaryCommand(uint32_t count) override  {
        auto commandBuffer = new VKSecCmdBuffer(app, pool);
        commandBuffer->create(count);
        return commandBuffer;
    }
    void release() override {
        if (pool) {
            vkDestroyCommandPool(app->getDevice()->getLogicalDevice(), pool, nullptr);
            pool = VK_NULL_HANDLE;
        }
        delete this;
    }
private:
    VKApp* app = nullptr;
    VkCommandPool pool = nullptr;
};

#endif // VK_CMDPOOL_H