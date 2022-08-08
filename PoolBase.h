#pragma once
#ifndef VK_POOLBASE_H
#define VK_POOLBASE_H
#include <vulkan/vulkan.h>
#include "VKApp.h"
#include "VKUtil.h"
#include "VKAllocator.h"
#include "VKSecCmdBuffer.h"
class VKSecCmdBuffer;

class PoolBase
{
public:
    virtual VkCommandBuffer beginSingleTimeCommands(uint32_t flag = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) = 0;
    virtual void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue) = 0;
    virtual VkCommandPool getCommandPool() = 0;
    virtual VKSecCmdBuffer* createSecondaryCommand(uint32_t count) = 0;
    virtual void release() = 0;
};

#endif // VK_POOLBASE_H