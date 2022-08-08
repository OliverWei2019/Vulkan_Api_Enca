#pragma once
#ifndef VK_INITIALIZERS_H
#define VK_INITIALIZERS_H
#include <vulkan/vulkan.h>

namespace initializers{
    VkImageCreateInfo createImageInfo() {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        //imageInfo.extent.width = width;
        //imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        //imageInfo.usage = usage;
        imageInfo.arrayLayers = 1;
        //imageInfo.mipLevels = miplevels;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        //imageInfo.format = format;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;//Æ½ÆÌ
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0;
        return imageInfo;
    }


    VkImageViewCreateInfo createImageViewCreateInfo()
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.pNext = nullptr;
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        //viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        //viewInfo.format = format;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        return viewInfo;
    }

    VkAttachmentDescription createAttachsDescription() {
        VkAttachmentDescription attachsDescription;
        attachsDescription.flags = 1;
        attachsDescription.format = VK_FORMAT_R8G8B8A8_UNORM;
        attachsDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        attachsDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachsDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachsDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachsDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachsDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachsDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        return attachsDescription;
    }

    VkSamplerCreateInfo createSamplerCreateInfo(uint32_t mipLevels)
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        //deviceProperties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.minLod = 0;
        samplerInfo.maxLod = static_cast<float>(mipLevels);
        samplerInfo.mipLodBias = 0;
        return samplerInfo;
    }
};
#endif // !VK_INITIALIZERS_H
