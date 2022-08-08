#pragma once

#ifndef VK_IMAGES_H
#define VK_IMAGES_H
#include <string>
#include <vulkan/vulkan.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include "VKApp.h"
#include "VMAllocator.h"
#include "VKCmdPool.h"
#include "VKDevices.h"
#include "initializers.h"
#include "Buffer.h"
#include "VKCmdPool.h"


class VKImages {

public:
	VKImages(VKApp* vkApp) : app(vkApp) {

	}
    ~VKImages() {
        if (image) {
            //vkDestroyImage(app->getDevice(), image, nullptr);
            vmaDestroyImage(app->getAllocator(), image, alloc);
        }
    }
public:
    VkImageCreateInfo createImageInfo(uint32_t width, uint32_t height,
        VkSampleCountFlagBits sample, uint32_t miplevels,
        VkFormat format, VkImageTiling tiling,
        VkImageUsageFlags usage) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.usage = usage;
        imageInfo.arrayLayers = 1;
        imageInfo.mipLevels = miplevels;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.format = format;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.tiling = tiling;//平铺
        imageInfo.samples = sample;
        imageInfo.flags = 0;
        return imageInfo;
    }
    bool load(const std::string& filename) {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(filename.c_str(),
            &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;
        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }
        auto mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texHeight, texWidth)))) + 1;

        VKBuffer* stagingBuffer = new VKBuffer(app,
            imageSize,
            true,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VMA_MEMORY_USAGE_AUTO);

        VmaAllocator allocator = app->getAllocator();
        void* gpuData;
        vmaMapMemory(allocator, stagingBuffer->getAllocation(), &gpuData);
        memcpy(gpuData, pixels, (size_t)imageSize);
        vmaUnmapMemory(allocator, stagingBuffer->getAllocation());
        
        stbi_image_free(pixels);
        if (!createImage(static_cast<uint32_t>(texWidth),
            static_cast<uint32_t>(texHeight),
            VK_SAMPLE_COUNT_1_BIT,
            static_cast<uint32_t>(mipLevels),
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
            VK_IMAGE_USAGE_TRANSFER_DST_BIT |
            VK_IMAGE_USAGE_SAMPLED_BIT |
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
            std::cerr << "Faild to create texture image!" << std::endl;
            return false;
        }
        //先改变textureImage的布局
        transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, 
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels);
        //从暂存复制图像到VkImage
        copyBufferToImage(stagingBuffer->getBuffer(), image,
            texWidth, texHeight);
        //transitioned to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL while generating mipmaps
        generateMipmaps(image, VK_FORMAT_R8G8B8A8_SRGB,
            static_cast<int32_t>(texWidth), 
            static_cast<int32_t>(texHeight), 
            mipLevels);
        //释放暂存缓冲区
        stagingBuffer->release();
        return true;
    }
    bool createImage(uint32_t width, uint32_t height,
        VkSampleCountFlagBits sample, uint32_t miplevels,
        VkFormat format,VkImageTiling tiling, 
        VkImageUsageFlags usage){
        
        imageInfo = initializers::createImageInfo();
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.usage = usage;
        imageInfo.mipLevels = miplevels;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.samples = sample;

        VmaAllocationCreateInfo allocCreateInfo = {};
        allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        allocCreateInfo.priority = 1.0f;
        
        //VmaAllocation alloc;

        vmaCreateImage(app->getAllocator(), &imageInfo, &allocCreateInfo, &image, &alloc, nullptr);


        //if (vkCreateImage(app->getDevice()->getLogicalDevice(), 
        //    &imageInfo, 
        //    app->getAllocator(), 
        //    &image) != VK_SUCCESS) {
        //    throw std::runtime_error("failed to create texture image!");
        //    return false;
        //}

        //VkMemoryRequirements memRequirements;
        //vkGetImageMemoryRequirements(app->getDevice()->getLogicalDevice(), 
        //    image, 
        //    &memRequirements);

        //VkMemoryAllocateInfo allocInfo{};
        //allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        //allocInfo.allocationSize = memRequirements.size;
        //allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits,properties);

        //if (vkAllocateMemory(app->getDevice()->getLogicalDevice(), 
        //    &allocInfo, 
        //    app->getAllocator(), 
        //    &imageMemory) != VK_SUCCESS) {
        //    throw std::runtime_error("failed to allocate texture image memory!");
        //    return false;
        //}
        //vkBindImageMemory(app->getDevice()->getLogicalDevice(), image, imageMemory, 0);

        return true;
    }
    bool createImage(VkImageCreateInfo& imageInfo, VmaAllocationCreateFlags flags,VmaMemoryUsage usage, float priority) {
        VmaAllocationCreateInfo allocCreateInfo = {};
        //allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        //allocCreateInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
        //allocCreateInfo.priority = 1.0f;
        allocCreateInfo.usage = usage;
        allocCreateInfo.flags = flags;
        allocCreateInfo.priority = priority;
        //VmaAllocation alloc;

        vmaCreateImage(app->getAllocator(), &imageInfo, &allocCreateInfo, &image, &alloc, nullptr);
        return true;
    }

    void release() {
        if (image) {
            //vkDestroyImage(app->getDevice(), image, nullptr);
            vmaDestroyImage(app->getAllocator(), image, alloc);
        }
        image = VK_NULL_HANDLE;
        delete this;
    }

public:
    void generateMipmaps(VkImage image, VkFormat imageFormat,
        int32_t texWidth, int32_t texHeight,
        uint32_t mipLevels) {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(app->getDevice()->getPhysicalDevice(), 
            imageFormat, 
            &formatProperties);

        if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
            throw std::runtime_error("texture image format does not support linear blitting!");
        }

        VkCommandBuffer commandBuffer = app->getCmdPool()->beginSingleTimeCommands();

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = image;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.subresourceRange.levelCount = 1;

        int32_t mipWidth = texWidth;
        int32_t mipHeight = texHeight;

        for (uint32_t i = 1; i < mipLevels; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            VkImageBlit blit = {};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = 1;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = 1;

            vkCmdBlitImage(commandBuffer,
                image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            if (mipWidth > 1)
                mipWidth /= 2;
            if (mipHeight > 1)
                mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        app->getCmdPool()->endSingleTimeCommands(commandBuffer,app->getDevice()->getGraphicsQueue());
        return;
    }
    void transitionImageLayout(VkImageLayout oldLayout,
        VkImageLayout newLayout, uint32_t mipLevels)
    {
        VkCommandBuffer commandBuffer = (app->getCmdPool())->beginSingleTimeCommands();
        adjustImageLayout(commandBuffer, image, oldLayout, newLayout, mipLevels);
        app->getCmdPool()->endSingleTimeCommands(commandBuffer, app->getDevice()->getGraphicsQueue());
    }
    void copyBufferToImage(VkBuffer buffer, VkImage image, 
        uint32_t width,uint32_t height)
    {
        VkCommandBuffer commandBuffer =app->getCmdPool()->beginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
            &region);
        app->getCmdPool()->endSingleTimeCommands(commandBuffer, app->getDevice()->getGraphicsQueue());
    }

    VkImage getImage() {
        return image;
    }
    uint32_t getWidth() {
        return imageInfo.extent.width;
    }
    uint32_t getHeight() {
        return imageInfo.extent.height;
    }
    uint32_t getMipLevel() {
        return imageInfo.mipLevels;
    }
    VkFormat getImageFormat() {
        return imageInfo.format;
    }
    VmaAllocation getImageAllocation() {
        return alloc;
    }

private:
    uint32_t findMemoryType(uint32_t typeFilter,
        VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(app->getDevice()->getPhysicalDevice(), &memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        throw std::runtime_error("failed to find suitable memory type!");
    }
private:
	VKApp* app = nullptr;
    VkImage  image;
    //VkDeviceMemory imageMemory;
    VkImageCreateInfo imageInfo;
    VmaAllocation alloc;
};

#endif //VK_IMAGES_H