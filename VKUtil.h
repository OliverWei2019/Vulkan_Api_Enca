#pragma once
#ifndef VK_UTIL_H
#define VK_UTIL_H
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <cstring>
#include <vector>
#include <optional>
#include <vulkan/vulkan.h>
#include "VKApp.h"


//队列家族索引结构体
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};
//交换链属性结构体
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};
void adjustImageLayout(VkCommandBuffer command, VkImage image, VkImageLayout oldLayout,
    VkImageLayout newLayout, uint32_t levelCount)
{
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = levelCount;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkPipelineStageFlags destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
        && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED
        && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
            VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }

    vkCmdPipelineBarrier(
        command,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
}

//二进制读取数据
std::vector<char> readDataFromFile(const std::string& filename)
{
    std::vector<char> buffer;

    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "failed to open file:" << filename.data() << std::endl;
        return buffer;
    }

    size_t size = (size_t)file.tellg();
    buffer.resize(size);

    file.seekg(0);
    file.read(buffer.data(), size);
    file.close();
    return buffer;
}

//打印PCO唯一标识符 UUID
void printUUID(uint8_t* pipelineCacheUUID)
{
    for (size_t j = 0; j < VK_UUID_SIZE; ++j) {
        std::cout << std::setw(2) << (uint32_t)pipelineCacheUUID[j];
        if (j == 3 || j == 5 || j == 7 || j == 9) {
            std::cout << '-';
        }
    }
}
template<class C>
void cleanVulkanObjectContainer(C& container)
{
    while (true) {
        auto itr = container.begin();
        if (itr == container.end())
            break;
        else if((*itr))
            (*itr)->release();
    }
}
//
//void writeFile(VKApp* app, const std::string& file, VkImage image, uint32_t width,
//    uint32_t height)
//{
//    VkBuffer imageBuffer;
//    VkDeviceMemory imageBufferMemory;
//
//    VkPhysicalDeviceMemoryProperties memoryProperties;
//    vkGetPhysicalDeviceMemoryProperties(context->getPhysicalDevice(), &memoryProperties);
//
//    context->createBuffer(width * height * 4, VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,//0,
//        imageBuffer, imageBufferMemory);
//
//    VkCommandBuffer commandBuffer;
//    {
//        VkCommandBufferAllocateInfo allocateInfo = {
//            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
//            .pNext = NULL,
//            .commandPool = context->getCommandPool()->getCommandPool(),
//            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
//            .commandBufferCount = 1,
//        };
//        assert(vkAllocateCommandBuffers(context->getDevice(), &allocateInfo, &commandBuffer) == VK_SUCCESS);
//    }
//
//    {
//        VkCommandBufferBeginInfo beginInfo = {
//            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
//            .pNext = NULL,
//            .flags = 0,
//            .pInheritanceInfo = NULL,
//        };
//        assert(vkBeginCommandBuffer(commandBuffer, &beginInfo) == VK_SUCCESS);
//
//        VkBufferImageCopy copy = {
//            .bufferOffset = 0,
//            .bufferRowLength = 0,
//            .bufferImageHeight = 0,
//            .imageSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1},
//            .imageOffset = {0, 0, 0},
//            .imageExtent = {
//                .width = width,
//                .height = height,
//                .depth = 1
//            },
//        };
//
//        vkCmdCopyImageToBuffer(commandBuffer, image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
//            imageBuffer, 1, &copy);
//
//        VkBufferMemoryBarrier transferBarrier = {
//            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
//            .pNext = 0,
//            .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
//            .dstAccessMask = VK_ACCESS_HOST_READ_BIT,
//            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
//            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
//            .buffer = imageBuffer,
//            .offset = 0,
//            .size = VK_WHOLE_SIZE,
//        };
//        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 0,
//            NULL, 1, &transferBarrier, 0, NULL);
//        assert(vkEndCommandBuffer(commandBuffer) == VK_SUCCESS);
//    }
//
//    VkFence fence;
//    {
//        VkFenceCreateInfo createInfo = {
//            .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
//            .pNext = 0,
//            .flags = 0,
//        };
//        assert(vkCreateFence(context->getDevice(), &createInfo, NULL, &fence) == VK_SUCCESS);
//    }
//
//    {
//        VkSubmitInfo submitInfo = {
//            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
//            .pNext = NULL,
//            .waitSemaphoreCount = 0,
//            .pWaitSemaphores = NULL,
//            .pWaitDstStageMask = NULL,
//            .commandBufferCount = 1,
//            .pCommandBuffers = &commandBuffer,
//            .signalSemaphoreCount = 0,
//            .pSignalSemaphores = NULL,
//        };
//        assert(vkQueueSubmit(context->getGraphicQueue(), 1, &submitInfo, fence) == VK_SUCCESS);
//
//        assert(vkWaitForFences(context->getDevice(), 1, &fence, VK_TRUE, UINT64_MAX) == VK_SUCCESS);
//        char* imageData;
//        assert(vkMapMemory(context->getDevice(), imageBufferMemory, 0, VK_WHOLE_SIZE,
//            0, (void**)&imageData) == VK_SUCCESS);
//
//        VkMappedMemoryRange flushRange = {
//            .sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
//            .pNext = NULL,
//            .memory = imageBufferMemory,
//            .offset = 0,
//            .size = VK_WHOLE_SIZE,
//        };
//
//        const VkExtent2D renderSize = {
//            .width = width,
//            .height = height
//        };
//
//        assert(vkInvalidateMappedMemoryRanges(context->getDevice(), 1, &flushRange) == VK_SUCCESS);
//        assert(writeTiff(file.data(), imageData, renderSize, 4) == 0);
//        vkUnmapMemory(context->getDevice(), imageBufferMemory);
//    }
//
//    assert(vkQueueWaitIdle(context->getGraphicQueue()) == VK_SUCCESS);
//    vkDestroyFence(context->getDevice(), fence, NULL);
//
//    vkFreeMemory(context->getDevice(), imageBufferMemory, context->getAllocation());
//    vkDestroyBuffer(context->getDevice(), imageBuffer, context->getAllocation());
//}

class VKSecondaryCommandBufferCallback
{
public:
    virtual void execute(VKApp* context, VkCommandBuffer commandBuffer, uint32_t current,
        uint32_t total) = 0;
};
#endif //VK_UTIL_H