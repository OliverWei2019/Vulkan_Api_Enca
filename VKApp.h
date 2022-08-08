#ifndef _VK_APP_H
#define _VK_APP_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include "VKAppConfig.h"
#include "VKAllocator.h"
#include "VMAllocator.h"

class VKValidationLayer;
class VKDevices;
class VKCmdPool;
class PoolBase;
class VKPipelineCache;
class VKRenderPass;
class VKSwapChain;
class PipeLineBase;
class Pipeline;
class VKSampler;
class VKShaderSet;
class Vertex;
class VertexBuffer;
class BufferBase;
class VKQueryPool;
class VKSecondaryCommandBufferCallback;
class VKImageView;
class VKImages;



class VKApp {

public:
    virtual bool initWindow()  = 0;
    virtual void initVulkanRHI(const Config config)  = 0;
    virtual void initVulkanRenderFrame() = 0;
    virtual void createInstance()  = 0;
    virtual void createSurface()  = 0;
    virtual Pipeline* createPipeline(VKShaderSet* shaderSet) = 0;
    virtual VKShaderSet* createShaderSet() = 0;
    
    virtual BufferBase* createVertexBuffer(const std::vector<float>& vertices, uint32_t count,
        const std::vector<uint32_t>& indices, bool indirectDraw) = 0;
    virtual BufferBase* createVertexBuffer(const std::vector<Vertex>& vertices,
        const std::vector<uint32_t>& indices, bool indirectDraw) = 0;
    virtual BufferBase* createVertexBuffer(const std::string& filename, bool zero,
        bool indirectDraw) = 0;

    
    virtual bool createGraphicsPipeline() = 0;
    virtual VKQueryPool* createQueryPool(uint32_t count, VkQueryPipelineStatisticFlags flag,
        std::function<void(const std::vector<uint64_t>&)> callback) = 0;
    virtual bool recordCommandBuffers() = 0;
    virtual bool createSecondaryCommandBuffer(uint32_t secondaryCommandBufferCount,
        std::shared_ptr<VKSecondaryCommandBufferCallback> caller) = 0;
    virtual void createSyncObjects() = 0;

    virtual VKSampler* createSampler(const VkSamplerCreateInfo& samplerInfo) = 0;
    virtual VKImages* createImage(const std::string& file) = 0;
    virtual VKImageView* createImageView(VkImageViewCreateInfo& viewCreateInfo) = 0;

    virtual void cleanupSwapChain() = 0;
    virtual void recreateSwapChain() = 0;
    virtual void cleanup() = 0;
    virtual  bool drawFrame() = 0;
    virtual bool run() = 0;
    virtual void release() = 0;

public:
    virtual VmaAllocator getAllocator() = 0;
    virtual GLFWwindow* getWindow() const = 0;
    virtual VkInstance getInstance() const = 0;
    virtual VKValidationLayer* getValidationLayer() const = 0;
    virtual VkSurfaceKHR getSurface() const = 0;
    virtual VKDevices* getDevice() const = 0;
    virtual VKSwapChain* getSwapChain() const = 0;
    virtual VKPipelineCache* getPipelineCache() const = 0;
    virtual PoolBase* getCmdPool() const = 0;
    virtual VKRenderPass* getRenderPass() const = 0;
    virtual PipeLineBase* getPipeline(uint32_t index) const = 0;
    virtual void addPipeline(Pipeline*) = 0;
    virtual void removeSampler(VKSampler* sampler) = 0;
    virtual void removeImageView(VKImageView* imageView) = 0;
};


#endif// _VK_APP_H