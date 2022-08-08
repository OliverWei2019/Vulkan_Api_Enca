#pragma once
#ifndef VK_PIPLELINEBASE_H
#define VK_PIPLELINEBASE_H
#include <list>
#include <memory>
#include <functional>
#include <vulkan/vulkan.h>
#include "VKApp.h"
#include "ShaderSet.h"
#include "Buffer.h"

#include "VKDescriptorSet.h"
#include "VKDescriptorSetLayout.h"
#include "VKDescriptorPool.h"
#include "PipelineLayout.h"
//#include "DynamicState.h"
#include "VKPipelineCache.h"
#include "VKRenderPass.h"
#include "PushDescriptor.h"

#include "VKSecCmdBuffer.h"
//#include "DynamicState.h"

//class DynamicState;

class PipeLineBase
{
public:
    virtual VKShaderSet* getShaderSet() = 0;
    //virtual DynamicState* getDynamicState() = 0;

    virtual void setVertexInputStateCreateInfo(const VkPipelineVertexInputStateCreateInfo& createInfo) = 0;
    virtual VkPipelineVertexInputStateCreateInfo getVertexInputStateCreateInfo()const = 0;

    virtual void setInputAssemblyStateCreateInfo(const VkPipelineInputAssemblyStateCreateInfo& createInfo) = 0;
    virtual VkPipelineInputAssemblyStateCreateInfo getInputAssemblyStateCreateInfo()const = 0;

    virtual void setRasterizationStateCreateInfo(const VkPipelineRasterizationStateCreateInfo& createInfo) = 0;
    virtual VkPipelineRasterizationStateCreateInfo getRasterizationStateCreateInfo()const = 0;

    virtual VkPipelineDepthStencilStateCreateInfo getDepthStencilStateCreateInfo()const = 0;
    virtual void setDepthStencilStateCreateInfo(const VkPipelineDepthStencilStateCreateInfo& createInfo) = 0;

    virtual void setTessellationStateCreateInfo(const VkPipelineTessellationStateCreateInfo& createInfo) = 0;
    virtual VkPipelineTessellationStateCreateInfo getTessellationStateCreateInfo()const = 0;

    virtual void setMultisampleStateCreateInfo(const VkPipelineMultisampleStateCreateInfo& createInfo) = 0;
    virtual VkPipelineMultisampleStateCreateInfo getMultisampleStateCreateInfo()const = 0;

    virtual void setColorBlendStateCreateInfo(const VkPipelineColorBlendStateCreateInfo& createInfo) = 0;
    virtual VkPipelineColorBlendStateCreateInfo getColorBlendStateCreateInfo()const = 0;

    virtual void setViewportStateCreateInfo(const VkPipelineViewportStateCreateInfo& createInfo) = 0;
    virtual VkPipelineViewportStateCreateInfo getViewportStateCreateInfo() const = 0;

public:
    virtual void prepare() = 0;
    virtual void addPushConstant(const VkPushConstantRange& constantRange, const char* data) = 0;
    virtual void addPushDescriptor(const VkWriteDescriptorSet& descriptor) = 0;
    virtual bool create() = 0;
    virtual void addRenderBuffer(BufferBase* buffer) = 0;
    //based parent pipeline create new pipeline
    virtual Pipeline* fork(VKShaderSet* shaderSet) = 0;

    virtual bool needRecreate() = 0;
    virtual void setNeedRecreate() = 0;

    virtual void render(VkCommandBuffer buffer, uint32_t index) = 0;
    virtual void render(VkCommandBuffer buffer, uint32_t index,
        std::shared_ptr<VKSecondaryCommandBufferCallback> caller,
        uint32_t current, uint32_t total) = 0;

    virtual void release() = 0;
public:
    virtual void initVertexInputStateCreateInfo(VKShaderSet* shaderSet) = 0;
    virtual void initMultisampleStateCreateInfo(VkSampleCountFlagBits sampleCount) = 0;
    virtual void initColorBlendStateCreateInfo() = 0;
    virtual void initInputAssemblyStateCreateInfo() = 0;
    virtual void initRasterizationStateCreateInfo() = 0;
    virtual void initDepthStencilStateCreateInfo() = 0;
    virtual void initColorBlendAttachmentState() = 0;

    virtual void initViewportStateCreateInfo(VkExtent2D extent) = 0;

    virtual bool createPipeline(VkPipelineCreateFlagBits flag) = 0;
};

#endif // VK_PIPLELINEBASE_H