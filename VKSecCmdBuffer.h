#pragma once
#ifndef VK_SECMDBUFFER_H
#define VK_SECMDBUFFER_H
#include <vector>
#include <vulkan/vulkan.h>
#include "VKApp.h"
#include "VKRenderPass.h"
#include "VKDevices.h"
#include "VKSwapChain.h"
#include "VKCmdPool.h"
#include "vk_mem_alloc.h"

class VKSecCmdBuffer
{
public:
    VKSecCmdBuffer(VKApp* vk_app, VkCommandPool pool) :app(vk_app), commandPool(pool){

    }
    ~VKSecCmdBuffer() {
        vkFreeCommandBuffers(app->getDevice()->getLogicalDevice(), commandPool, buffers.size(), buffers.data());
        buffers.clear();
        
    }
public:
    bool create(uint32_t count) {
        VkCommandBufferAllocateInfo cmdAlloc = {};
        cmdAlloc.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdAlloc.pNext = NULL;
        cmdAlloc.commandPool = commandPool;
        cmdAlloc.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        cmdAlloc.commandBufferCount = count;

        buffers.resize(count);
        return vkAllocateCommandBuffers(app->getDevice()->getLogicalDevice(),
            &cmdAlloc, 
            buffers.data()) == VK_SUCCESS;
    }
    void executeCommandBuffer(VkCommandBuffer command, VkFramebuffer frameBuffer) {
        VkRenderPassBeginInfo rpBegin;
        rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpBegin.pNext = NULL;
        rpBegin.renderPass = app->getRenderPass()->getRenderPass();
        rpBegin.framebuffer = frameBuffer;
        rpBegin.renderArea.offset.x = 0;
        rpBegin.renderArea.offset.y = 0;
        rpBegin.renderArea.extent.width = app->getSwapChain()->getSwapChainExtent().width;
        rpBegin.renderArea.extent.height = app->getSwapChain()->getSwapChainExtent().height;
        //Òª¸Ä
        VkClearValue cv[2] = {};
        rpBegin.clearValueCount = 2;
        cv[1] = { 1.0f,0 };
        rpBegin.pClearValues = cv;

        vkCmdBeginRenderPass(command, &rpBegin, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
        vkCmdExecuteCommands(command, buffers.size(), buffers.data());
        vkCmdEndRenderPass(command);
    }
    VkCommandBuffer At(uint32_t index) {
        if (buffers.empty()) {
            std::cerr << " secondery command buffer vector was empty!" << std::endl;
            return VK_NULL_HANDLE;
        }
        if (index >= buffers.size()) {
            std::cerr << "input index over secondery command buffer vector size!" << std::endl;
            return VK_NULL_HANDLE;
        }
        return buffers[index];
    }
    void release() {
        vkFreeCommandBuffers(app->getDevice()->getLogicalDevice(), commandPool,static_cast<uint32_t>(buffers.size()), buffers.data());
        buffers.clear();
        delete this;
    }
private:
    VKApp* app = nullptr;
    VkCommandPool commandPool = nullptr;
    std::vector<VkCommandBuffer> buffers;
};

#endif // VK_SECMDBUFFER_H