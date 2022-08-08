#pragma once
#ifndef _APPLICATION_H
#define _APPLICATION_H
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <sstream>
#include "VKApp.h"
#include "VMAllocator.h"
#include "VKUtil.h"
#include "VKAppConfig.h"
#include "VKAllocator.h"
#include "VKValidateLayer.h"
#include "VKDevices.h"
#include "VKCmdPool.h"
#include "PoolBase.h"
#include "VKSwapChain.h"
#include "VKPipelineCache.h"
#include "VKRenderPass.h"
#include "PipeLineBase.h"
#include "Pipeline.h"
#include "VertexBuffer.h"
#include "QueryPool.h"
#include "VKImages.h"
#include "VKImageView.h"
#include "Sampler.h"
#include "LoadObjects.h"


class App:public VKApp {

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }
    static void mouseButtonCallback(GLFWwindow* window, int button, int state, int mods) {
        auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
        if (app->appConfig.mouseCallback)
            app->appConfig.mouseCallback(button, state, mods);
    }
public:
    App(const VKAppConfig& Appconfig, const Config& vkconfig) :appConfig(Appconfig),config(vkconfig) {
        glfwInit();
        //vkAllocator = new VKAllocator();
        //deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }
    ~App() {
        glfwTerminate();
    }
    void release() override
    {
        vmaDestroyAllocator(Allocator);
        //delete vkAllocator;
        delete this;
    }
    bool run() override {
        //static double time = glfwGetTime();
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            if (!drawFrame())
                break;
            //time = glfwGetTime();
        }

        vkDeviceWaitIdle(Device->getLogicalDevice());
        cleanup();
        return true;
    }

private:
    //app配置
    VKAppConfig appConfig;
    Config config;
    //memory allocate
    //VKAllocator* vkAllocator = nullptr;
    //窗口；
    GLFWwindow* window = nullptr;
    bool resize = true;
    bool framebufferResized = false;
    //vk实例
    VkInstance instance = nullptr;
    //验证层
    VKValidationLayer* vkValidationLayer = nullptr;
    //表面
    VkSurfaceKHR surface = nullptr;
    //设备
    VKDevices* Device = nullptr;
    //VMA 
    VmaAllocator Allocator = nullptr;
    //指令池
    VKCmdPool* CmdPool = nullptr;
    //交换链
    VKSwapChain* swapChain = nullptr;
    //Pipeline Cache Object
    VKPipelineCache* PCO = nullptr;

    VKRenderPass* renderpass = nullptr;
    /*std::vector<VKImageView*> swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;*/

    //pipeline数组
    std::vector<Pipeline*> PipeLines;
    //shader集合
    std::vector<VKShaderSet*> shaders;
    //vertex buffer vector
    std::vector<BufferBase*> vkBuffers;
    //cmdbuffer vector and secondery cmd buffer  vector
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VKSecCmdBuffer*> secondaryCommandBuffers;
    uint32_t secondaryCommandBufferCount = 0;
    //secondery cmd buffer caller function must coded by user
    std::shared_ptr<VKSecondaryCommandBufferCallback> secondaryCommandBufferCaller;

    //query pool
    VKQueryPool* queryPool = nullptr;
    //clear value
    VkClearValue vkClearValue = { {{0.0f, 0.0f, 0.0f, 0.0f}} };

    //SyncObjects
    //swap chain image availabel semaphores and render finished semaphores
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;

    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    std::list<VKSampler*> SamplerList;
    std::list< VKImageView*> ImageViewList;
    std::list<VKImages*> ImagesList;

    size_t currentFrame = 0;
    bool needUpdateSwapChain = false;
    bool captureScreen = false;
    uint32_t currentFrameIndex = 0;
private:
    bool initWindow() override {
        if (window)
            return false;
        //禁用opengl的api
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        //窗口是否可变
        glfwWindowHint(GLFW_RESIZABLE, resize);

        window = glfwCreateWindow(appConfig.width, appConfig.height, appConfig.name.data(), nullptr, nullptr);
        if (!window) {
            std::cerr << "create glfw window failed\n";
            return false;
        }

        //glfwSwapInterval(1);

        glfwSetWindowUserPointer(window, this);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        return window != nullptr;
    }
    void initVulkanRHI(const Config config) override {
        vkValidationLayer = new VKValidationLayer(this, appConfig.debug);
        createInstance();
        createSurface();
        Device = new VKDevices(this);

        Allocator = getVMAlocator(instance, Device->getPhysicalDevice(), Device->getLogicalDevice());

        QueueFamilyIndices index = Device->getQueueFamiliesIndices();
        CmdPool = new VKCmdPool(this,index);
        swapChain = new VKSwapChain(this);
        PCO = new VKPipelineCache(Device->getLogicalDevice(), nullptr, Device->getPhysicalDeviceProp());
        PCO->create(config.pipelineCacheFile,appConfig.debug);

        /* pickPhysicalDevice();
         createLogicalDevice();
         createCommandPool();
         createSwapChain();
         createSwapChainImageViews();
         createPipelineCache();*/
    }
    void initVulkanRenderFrame()override {
        renderpass = new  VKRenderPass(this);
        renderpass->create();
        renderpass->createFrameBuffers();
        createSyncObjects();
    }
    void createInstance() override {
        //1 检查验证层是否支持
        if (appConfig.debug && !vkValidationLayer->appendValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = appConfig.name.data();
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        //2 依据验证层是否被启用，返回请求的扩展的列表,根据返回列表绑定creatInfo
        auto extension = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extension.size());
        createInfo.ppEnabledExtensionNames = extension.data();
        //3 是否启用验证层
        vkValidationLayer->adjustVkInstanceCreateInfo(createInfo);
        //4 调用实例化函数
        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

        //5 是否创建实例成功
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
        if (!vkValidationLayer->setupDebugMessenger(instance)) {
            throw std::runtime_error("failed to setup debug messenger!");
        }
    }
    void createSurface() override {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    Pipeline* createPipeline(VKShaderSet* shaderSet) override
    {
        auto pipeLine = new Pipeline(this, shaderSet);
        pipeLine->prepare();
        PipeLines.push_back(pipeLine);
        return pipeLine;
    }
    VKShaderSet* createShaderSet() override
    {
        auto shaderSet = new VKShaderSet(this);
        shaders.push_back(shaderSet);
        return shaderSet;
    }
    BufferBase* createVertexBuffer(const std::vector<float>& vertices, uint32_t count,
        const std::vector<uint32_t>& indices, bool indirectDraw) override
    {
        auto vertexBuffer = new VertexBuffer(this);
        vertexBuffer->create(vertices, count, indices, indirectDraw);
        vkBuffers.push_back(vertexBuffer);
        return vertexBuffer;
    }

    BufferBase* createVertexBuffer(const std::vector<Vertex>& vertices,
        const std::vector<uint32_t>& indices, bool indirectDraw) override
    {
        auto vertexBuffer = new VertexBuffer(this);
        vertexBuffer->create(vertices, indices, indirectDraw);
        vkBuffers.push_back(vertexBuffer);
        return vertexBuffer;
    }
    BufferBase* createVertexBuffer(const std::string& filename, bool zero,
        bool indirectDraw) override{
        VKOBJLoader* loader = new VKOBJLoader(this);
        if (!loader->load(filename, zero)) {
            loader->release();
            return nullptr;
        }

        auto data = loader->getData();
        if (!data.empty()) {
            loader->create(data[0], 8, std::vector<uint32_t>(), indirectDraw);
        }
        vkBuffers.push_back(loader);
        return loader;
    }


    bool createGraphicsPipeline() override
    {
        for (auto pipeline : PipeLines)
            pipeline->create();
        return true;
    }
    VKQueryPool* createQueryPool(uint32_t count, VkQueryPipelineStatisticFlags flag,
        std::function<void(const std::vector<uint64_t>&)> callback) override
    {
        if (queryPool)
            return queryPool;
        queryPool = new VKQueryPool(this, count, flag);
        queryPool->setQueryCallback(callback);
        return queryPool;
    }
    
    bool recordCommandBuffers() override
    {
        commandBuffers.resize(this->getSwapChain()->getSwapChainSize());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = this->getCmdPool()->getCommandPool();
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();
        allocInfo.pNext = nullptr;

        if (vkAllocateCommandBuffers(Device->getLogicalDevice(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
            std::cerr << "failed to allocate command buffers!" << std::endl;
            return false;
        }

        for (size_t i = 0; i < commandBuffers.size(); i++) {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                std::cerr << "failed to begin recording command buffer!" << std::endl;
            }

            if (!secondaryCommandBuffers.empty()) {
                auto current = secondaryCommandBuffers.at(i);
                current->executeCommandBuffer(commandBuffers[i], renderpass->getFrameBuffer(i));
            }
            else {
                VkRenderPassBeginInfo renderPassInfo{};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassInfo.renderPass = renderpass->getRenderPass();
                renderPassInfo.framebuffer = renderpass->getFrameBuffer(i);
                renderPassInfo.renderArea.offset = { 0, 0 };
                renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent();

                std::array<VkClearValue, 2> clearValues{};
                memcpy((char*)&clearValues[0], &vkClearValue, sizeof(vkClearValue));
                clearValues[1].depthStencil = { 1.0f, 0 };

                renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
                renderPassInfo.pClearValues = clearValues.data();

                //queryPool reset must in front of render pass begining 
                if (queryPool)
                    queryPool->reset(commandBuffers[i]);

                vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                if (queryPool) {
                    queryPool->startQeury(commandBuffers[i]);
                }

                for (auto pipeline : PipeLines) {
                    pipeline->render(commandBuffers[i], i);
                }

                if (queryPool)
                    queryPool->endQuery(commandBuffers[i]);

                vkCmdEndRenderPass(commandBuffers[i]);
            }

            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
                std::cout << "failed to record command buffer!" << std::endl;
            }
        }
        return true;
    }
    bool createSecondaryCommandBuffer(uint32_t secondaryCommandBufferCount,
        std::shared_ptr<VKSecondaryCommandBufferCallback> caller) override
    {
        if (secondaryCommandBufferCount == 0)
            return false;

        this->secondaryCommandBufferCount = secondaryCommandBufferCount;
        secondaryCommandBufferCaller = caller;

        secondaryCommandBuffers.resize(swapChain->getSwapChainSize());
        for (uint32_t i = 0; i <renderpass->getFrameBufferSize(); i++) {
            secondaryCommandBuffers[i] = new VKSecCmdBuffer(this,CmdPool->getCommandPool());
            secondaryCommandBuffers[i]->create(secondaryCommandBufferCount);

            VkCommandBufferInheritanceInfo info = {};
            info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
                info.pNext = NULL;
            info.renderPass = renderpass->getRenderPass();
            info.subpass = 0;
            info.framebuffer = renderpass->getFrameBuffer(i);
            info.occlusionQueryEnable = VK_FALSE;
            info.queryFlags = 0;
            info.pipelineStatistics = 0;

            VkCommandBufferBeginInfo secondaryCommandBufferBeginInfo = {};
            secondaryCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            secondaryCommandBufferBeginInfo.pNext = NULL;
            secondaryCommandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT |
                VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
            secondaryCommandBufferBeginInfo.pInheritanceInfo = &info;

            for (uint32_t j = 0; j < secondaryCommandBufferCount; j++) {
                auto commandBuffer = secondaryCommandBuffers[i]->At(j);
                vkBeginCommandBuffer(commandBuffer, &secondaryCommandBufferBeginInfo);

                for (auto pipeline : PipeLines) {
                    pipeline->render(commandBuffer, i, caller, j, secondaryCommandBufferCount);
                }

                vkEndCommandBuffer(commandBuffer);
            }
        }
        return true;
    }
    void createSyncObjects() override
    {
        auto count = config.maxFramsInFlight;
        imageAvailableSemaphores.resize(count);
        renderFinishedSemaphores.resize(count);
        inFlightFences.resize(count);
        imagesInFlight.resize(swapChain->getSwapChainSize(), VK_NULL_HANDLE);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (int i = 0; i < config.maxFramsInFlight; i++) {
            if (vkCreateSemaphore(Device->getLogicalDevice(), &semaphoreInfo, nullptr,
                &imageAvailableSemaphores[i]) != VK_SUCCESS
                ||
                vkCreateSemaphore(Device->getLogicalDevice(), &semaphoreInfo, nullptr,
                    &renderFinishedSemaphores[i]) != VK_SUCCESS 
                ||
                vkCreateFence(Device->getLogicalDevice(), &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
                std::cerr << "failed to create synchronization objects for a frame!" << std::endl;
            }
        }
    }
    VKSampler* createSampler(const VkSamplerCreateInfo& samplerInfo) override
    {
        auto sampler = new VKSampler(this);
        if (!sampler->create(samplerInfo)) {
            delete sampler;
            return nullptr;
        }
        SamplerList.push_back(sampler);
        return sampler;
    }

    void removeSampler(VKSampler* sampler) override
    {
        SamplerList.remove(sampler);
    }

    VKImageView* createImageView(VkImageViewCreateInfo& viewCreateInfo) override
    {
        auto imageView = new VKImageView(this);
        
        if (!imageView->createImageView(viewCreateInfo)) {
            delete imageView;
            return nullptr;
        }
        ImageViewList.push_back(imageView);
        return imageView;
    }
    VKImages* createImage(const std::string& file) override {
        auto textureImage = new VKImages(this);
        if (!textureImage->load(file)) {
            textureImage->release();
            return nullptr;
        }
        ImagesList.push_back(textureImage);
        return textureImage;
    }
    void removeImageView(VKImageView* imageView) override 
    {
        ImageViewList.remove(imageView);
    }

    void cleanupSwapChain() override {
        renderpass->clearAttachSet();
        if (!secondaryCommandBuffers.empty()) {
            for (auto SecCmdBuffer : secondaryCommandBuffers)
            {
                SecCmdBuffer->release();
            }
            secondaryCommandBuffers.clear();
        }
        vkFreeCommandBuffers(Device->getLogicalDevice(),
            CmdPool->getCommandPool(), (uint32_t)commandBuffers.size(), 
            commandBuffers.data());
        for (auto pipeline : PipeLines)
            pipeline->release();

        renderpass->release();
        swapChain->clearup();
        for (auto shader : shaders) {
            shader->clearUniformBuffer();
        }
    }
    void cleanup() override {
        if (queryPool) {
            queryPool->release();
            queryPool = nullptr;
        }
        if (!vkBuffers.empty()) {
            for (size_t i = 0; i < vkBuffers.size(); i++) {
                if (vkBuffers[i]) {
                    vkBuffers[i]->release();
                }
            }
        }
        vkBuffers.clear();
        PCO->saveGraphicsPiplineCache(config.pipelineCacheFile);
        PCO->release();

        cleanupSwapChain();
        if (!shaders.empty()) {
            for (size_t i = 0; i < shaders.size(); i++) {
                if (shaders[i]) {
                    shaders[i]->release();
                }
            }
        }
        shaders.clear();
        cleanVulkanObjectContainer(SamplerList);
        cleanVulkanObjectContainer(ImageViewList);
        //cleanVulkanObjectContainer(ImageList);

        for (int i = 0; i < config.maxFramsInFlight; i++) {
            vkDestroySemaphore(Device->getLogicalDevice(), renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(Device->getLogicalDevice(), imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(Device->getLogicalDevice(), inFlightFences[i],nullptr);
        }
        CmdPool->release();
        Device->release();
        vkValidationLayer->cleanup(instance);
        vkValidationLayer->release();
        vkDestroySurfaceKHR(instance, surface,nullptr);
        vkDestroyInstance(instance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    void recreateSwapChain() override {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(window, &width, &height);
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(Device->getLogicalDevice());

        cleanupSwapChain();
        //swapChain = new VKSwapChain(this);
        swapChain->create();
        for (auto shaderSet : shaders)
            shaderSet->initUniformBuffer();

        //swapChain = new VKSwapChain(this);
        renderpass = new VKRenderPass(this);
        renderpass->create();
        renderpass->createFrameBuffers();

        createGraphicsPipeline();

        createSecondaryCommandBuffer(secondaryCommandBufferCount, secondaryCommandBufferCaller);
        recordCommandBuffers();

        imagesInFlight.resize(swapChain->getSwapChainSize(), VK_NULL_HANDLE);
    }

    bool drawFrame() override {
        for (auto pipeline : PipeLines) {
            if (pipeline->needRecreate() || needUpdateSwapChain) {
                recreateSwapChain();
                needUpdateSwapChain = false;
            }
        }

        if (vkWaitForFences(Device->getLogicalDevice(),
            1,
            &inFlightFences[currentFrame],
            VK_TRUE,
            UINT64_MAX) != VK_SUCCESS) {
            std::cerr << "waiting fences faild!" << std::endl;
            return false;
        }

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(Device->getLogicalDevice(), 
            swapChain->getSwapChain(), 
            UINT64_MAX,
            imageAvailableSemaphores[currentFrame],
            VK_NULL_HANDLE, 
            &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return true;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            std::cerr << "failed to acquire swap chain image!" << std::endl;
            return false;
        }

        if (queryPool) {
            queryPool->query();
        }

        //currentFrameIndex++;

        //captureImage(imageIndex);

        for (auto shaderSet : shaders)
            shaderSet->update(imageIndex);

        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
            vkWaitForFences(Device->getLogicalDevice(), 
                1, 
                &imagesInFlight[imageIndex], 
                VK_TRUE, 
                UINT64_MAX);
        }

        imagesInFlight[imageIndex] = inFlightFences[currentFrame];

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

        VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        vkResetFences(Device->getLogicalDevice(), 1, &inFlightFences[currentFrame]);

        if (vkQueueSubmit(Device->getGraphicsQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
            std::cerr << "failed to submit draw command buffer!" << std::endl;
            return false;
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { swapChain->getSwapChain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;

        result = vkQueuePresentKHR(Device->getPresentQueue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR ||
            result == VK_SUBOPTIMAL_KHR ||
            framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            std::cerr << "failed to present swap chain image!" << std::endl;
            return false;
        }

        currentFrame = (currentFrame + 1) % config.maxFramsInFlight;
        return true;
    }


public:
    VmaAllocator getAllocator() override {
        //return vkAllocator->getAllocator();
        return Allocator;
    }
    GLFWwindow* getWindow() const override {
        return window;
    }
    VkInstance getInstance() const override {
        return instance;
    }
    VKValidationLayer* getValidationLayer() const override {
        return vkValidationLayer;
    }
    VkSurfaceKHR getSurface() const override {
        return surface;
    }
    VKDevices* getDevice() const override {
        return Device;
    }
    VKSwapChain* getSwapChain() const override {
        return swapChain;
    }
    VKPipelineCache* getPipelineCache() const override {
        return PCO;
    }
    PoolBase* getCmdPool() const override {
        return CmdPool;
    }

    VKRenderPass* getRenderPass() const override {
        return renderpass;
    }
    PipeLineBase* getPipeline(uint32_t index) const override {
        if(!PipeLines.empty())
            return PipeLines[index];
        return nullptr;
    }
    void addPipeline(Pipeline* pipeline) {
        if (pipeline) {
            PipeLines.push_back(pipeline);
        }
        return;
    }
    
private:
    std::vector<const char*> getRequiredExtensions() {
        //利用GLFW创建窗口系统交互的扩展
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        //申请一个扩展数组 vector( vector&& other, const Allocator& alloc );
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (appConfig.debug) {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;

    }
};

VKApp* creatApp(const VKAppConfig& Appconfig, const Config& vkconfig)
{
    return new App(Appconfig, vkconfig);
}

#endif// _APPLICATION_H