#pragma once
#ifndef VK_SAMPLER_H
#define VK_SAMPLER_H
#include "VKApp.h"
#include "VKDevices.h"
//#include "VK_Texture.h"
//#include "VK_ContextImpl.h"

class VKSampler
{
public:
    VKSampler(VKApp* vkApp):app(vkApp) {

    }
    ~VKSampler() {

    }
public:
    bool create(const VkSamplerCreateInfo& samplerInfo) {
        if (vkCreateSampler(app->getDevice()->getLogicalDevice(), 
            &samplerInfo, 
            nullptr, 
            &textureSampler) != VK_SUCCESS) {
            std::cerr << "failed to create sampler!" << std::endl;
            return false;
        }
        return true;
    }
    void release() {
        if (textureSampler) {
            app->removeSampler(this);
            vkDestroySampler(app->getDevice()->getLogicalDevice(), textureSampler, nullptr);
            textureSampler = nullptr;
        }
        delete this;
    }

    VkSampler* getSampler() {
        return &textureSampler;
    }
private:
    VKApp* app = nullptr;
    VkSampler textureSampler = 0;
};

#endif