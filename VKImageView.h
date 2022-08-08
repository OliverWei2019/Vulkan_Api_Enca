#pragma once
#ifndef VK_IMAGEVIEW_H
#define VK_IMAGEVIEW_H
#include "VKApp.h"
#include "initializers.h"


class VKImageView
{
public:
    VKImageView(VKApp* vkApp) :app(vkApp) {

    }
    ~VKImageView() {
        if (imageView)
            vkDestroyImageView(app->getDevice()->getLogicalDevice(), imageView, nullptr);
    }
public:
    VkImageViewCreateInfo createImageViewCreateInfo(VkImage image, 
        VkFormat format, 
        uint32_t mipLevels)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.pNext = nullptr;
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
        viewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
        viewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
        viewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        return viewInfo;
    }
    bool createImageView(VkImage image, uint32_t mipLevels,
        VkImageLayout imageLayout,
        VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
        VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT)
    {
        createInfo = initializers::createImageViewCreateInfo();
        createInfo.image = image;
        createInfo.format = format;
        createInfo.subresourceRange.levelCount = mipLevels;
        ///createInfo = createImageViewCreateInfo(image, format, mipLevels);
        createInfo.subresourceRange.aspectMask = aspectFlags;


        if (vkCreateImageView(app->getDevice()->getLogicalDevice(), 
            &createInfo, 
            nullptr, 
            &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view !");
        }
        imageInfo.imageLayout = imageLayout;
        imageInfo.imageView = imageView;
        imageInfo.sampler = nullptr;
        return true;
    }
    bool createImageView(VkImageViewCreateInfo& viewInfo) {
        if (vkCreateImageView(app->getDevice()->getLogicalDevice(),
            &viewInfo,
            nullptr,
            &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view !");
        }
        //imageInfo.imageLayout = nullptr;
        imageInfo.imageView = imageView;
        //imageInfo.sampler = nullptr;
        return true;
    }
    void setLayout(VkImageLayout layout) {
        imageInfo.imageLayout = layout;
    }
    void setSampler(VkSampler sampler) {
        imageInfo.sampler = sampler;
    }
    void release() {
        if (imageView)
            vkDestroyImageView(app->getDevice()->getLogicalDevice(), imageView, nullptr);
        imageView = VK_NULL_HANDLE;
        //delete this;
    }
public:
    VkWriteDescriptorSet createWriteDescriptorSet(VkDescriptorSet descriptorSet, uint32_t binding = 1) {
        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = binding;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pImageInfo = &imageInfo;

        return descriptorWrite;
    }
    VkImageView getImageView() {
        return imageView;
    }
private:
    VKApp* app = nullptr;
    VkImageViewCreateInfo createInfo;
    VkDescriptorImageInfo imageInfo;
    VkImageView imageView;
};

#endif // VK_IMAGEVIEW_H