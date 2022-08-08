#pragma once
#ifndef VK_PIPLELINE_H
#define VK_PIPLELINE_H
#include <list>
#include <memory>
#include <optional>
#include <functional>
#include <vulkan/vulkan.h>
#include "VKApp.h"
#include "ShaderSet.h"
#include "Buffer.h"
#include "DynamicState.h"

#include "PipeLineBase.h"

class Pipeline:public PipeLineBase
{
    //friend class VK_PipelineDeriveImpl;
public:
    Pipeline() = delete;
    Pipeline(VKApp* vkApp, VKShaderSet* shaderset,
        Pipeline* inputParent = nullptr):app(vkApp),shaderSet(shaderset),parent(inputParent) {
        if (!shaderSet) {
            if (parent) {
                shaderSet = parent->getShaderSet();
            }
        }
        vkDynamicState = new DynamicState(this);
        descriptorSets = std::make_shared<DescriptorSets>(vkApp);
        pipelineLayout = std::make_shared<VKPipelineLayout>(vkApp);
    }
    ~Pipeline() {
        vkDynamicState->release();
     }
public:
    VKShaderSet* getShaderSet() override {
        return shaderSet;
    }
    DynamicState* getDynamicState(){
        return vkDynamicState;
    }

    void setVertexInputStateCreateInfo(const VkPipelineVertexInputStateCreateInfo& createInfo) override {
        vertexInputStateCreateInfo = std::make_optional(createInfo);
        needUpdate = true;
    }
    VkPipelineVertexInputStateCreateInfo getVertexInputStateCreateInfo()const override {
        if (vertexInputStateCreateInfo.has_value()) {
            return vertexInputStateCreateInfo.value();
        }
        assert(parent);
        return parent->getVertexInputStateCreateInfo();
    }

    void setInputAssemblyStateCreateInfo(const VkPipelineInputAssemblyStateCreateInfo& createInfo)override {
        inputAssemblyStateCreateInfo = std::make_optional(createInfo);
        needUpdate = true;
    }
    VkPipelineInputAssemblyStateCreateInfo getInputAssemblyStateCreateInfo()const override {
        if (inputAssemblyStateCreateInfo.has_value()) {
            return inputAssemblyStateCreateInfo.value();
        }
        assert(parent);
        return parent->getInputAssemblyStateCreateInfo();
    }

    void setRasterizationStateCreateInfo(const VkPipelineRasterizationStateCreateInfo& createInfo)override {
        rasterizationStateCreateInfo = std::make_optional(createInfo);
        needUpdate = true;
    }
    VkPipelineRasterizationStateCreateInfo getRasterizationStateCreateInfo()const override {
        if (rasterizationStateCreateInfo.has_value()) {
            return rasterizationStateCreateInfo.value();
        }
        assert(parent);
        return parent->getRasterizationStateCreateInfo();
    }

    VkPipelineDepthStencilStateCreateInfo getDepthStencilStateCreateInfo()const override {
        if (depthStencilStateCreateInfo.has_value()) {
            return depthStencilStateCreateInfo.value();
        }
        assert(parent);
        return parent->getDepthStencilStateCreateInfo();
    }
    void setDepthStencilStateCreateInfo(const VkPipelineDepthStencilStateCreateInfo& createInfo)override {
        depthStencilStateCreateInfo = std::make_optional(createInfo);
        needUpdate = true;
    }

    void setTessellationStateCreateInfo(const VkPipelineTessellationStateCreateInfo& createInfo)override {
        tessellationStateCreateInfo = std::make_optional(createInfo);
        needUpdate = true;
    }
    VkPipelineTessellationStateCreateInfo getTessellationStateCreateInfo()const override {
        if (tessellationStateCreateInfo.has_value()) {
            return tessellationStateCreateInfo.value();
        }
        assert(parent);
        return parent->getTessellationStateCreateInfo();
    }

    void setMultisampleStateCreateInfo(const VkPipelineMultisampleStateCreateInfo& createInfo)override {
        multiSampleStateCreateInfo = std::make_optional(createInfo);
        needUpdate = true;
    }
    VkPipelineMultisampleStateCreateInfo getMultisampleStateCreateInfo()const override {
        if (multiSampleStateCreateInfo.has_value()) {
            return multiSampleStateCreateInfo.value();
        }
        assert(parent);
        return parent->getMultisampleStateCreateInfo();
    }

    void setColorBlendStateCreateInfo(const VkPipelineColorBlendStateCreateInfo& createInfo)override {
        colorBlendStateCreateInfo = std::make_optional(createInfo);
        needUpdate = true;
    }
    VkPipelineColorBlendStateCreateInfo getColorBlendStateCreateInfo()const override {
        if (colorBlendStateCreateInfo.has_value()) {
            return colorBlendStateCreateInfo.value();
        }
        assert(parent);
        return parent->getColorBlendStateCreateInfo();
    }

    void setViewportStateCreateInfo(const VkPipelineViewportStateCreateInfo& createInfo)override {
        viewportStateCreateInfo = std::make_optional(createInfo);
        needUpdate = true;
    }
    VkPipelineViewportStateCreateInfo getViewportStateCreateInfo()const override {
        if (viewportStateCreateInfo.has_value()) {
            return viewportStateCreateInfo.value();
        }
        assert(parent);
        return parent->getViewportStateCreateInfo();
    }

public:
    void prepare() {
        initVertexInputStateCreateInfo(shaderSet);
        initMultisampleStateCreateInfo(VK_SAMPLE_COUNT_1_BIT);
        initInputAssemblyStateCreateInfo();
        initRasterizationStateCreateInfo();
        initColorBlendAttachmentState();
        initColorBlendStateCreateInfo();
        initDepthStencilStateCreateInfo();
        auto extent = app->getSwapChain()->getSwapChainExtent();
        initViewportStateCreateInfo(extent);
        return;
    }
    void addPushConstant(const VkPushConstantRange& constantRange, const char* data)override {
        pipelineLayout->addPushConstant(constantRange, data);
    }
    void addPushDescriptor(const VkWriteDescriptorSet& descriptor)override {
        if (!pushDescriptors) {
            pushDescriptors = std::make_shared<VKPushDescriptor>(app);
        }
        pushDescriptors->addDescriptor(descriptor);
    }
    bool create() override {
        return createPipeline(VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT);
    }
    void addRenderBuffer(BufferBase* buffer)override {
        if (buffer) {
            buffers.push_back(buffer);
        }
    }
    //based parent pipeline create new pipeline
    Pipeline* fork(VKShaderSet* shaderSet)override {
        if (parent) {
            std::cerr << "Pipeline drive failed,current pipeline has parent!" << std::endl;
            return nullptr;
        }
        auto child = new Pipeline(app, shaderSet, this);
        app->addPipeline(child);
        return child;
    }

    bool needRecreate()override {
        return needUpdate;
    }
    void setNeedRecreate()override {
        needUpdate = true;
    }

    void render(VkCommandBuffer buffer, uint32_t index) override {
        //绑定描述符集合
        descriptorSets->bind(buffer, pipelineLayout->getPipelineLayout(), index);
        //推送常量
        pipelineLayout->pushConst(buffer);
        //将动态状态应用到cmdbuffer
        vkDynamicState->apply(buffer);
        //绑定管线
        vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        //推送描述符
        if(pushDescriptors)
            pushDescriptors->push(buffer, pipelineLayout->getPipelineLayout());
        //根据当前的顶点or索引buffer 进行顶点和索引绑定和绘制调用
        for (auto currBuffer : buffers) {
            currBuffer->render(buffer);
        }
    }
    void render(VkCommandBuffer buffer, uint32_t index,
        std::shared_ptr<VKSecondaryCommandBufferCallback> caller,
        uint32_t current, uint32_t total) override {
        //绑定描述符集合
        descriptorSets->bind(buffer, pipelineLayout->getPipelineLayout(), index);
        //推送常量
        pipelineLayout->pushConst(buffer);
        //将动态状态应用到cmdbuffer
        vkDynamicState->apply(buffer);
        //绑定管线
        vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        //推送描述符
        if (pushDescriptors)
            pushDescriptors->push(buffer, pipelineLayout->getPipelineLayout());
        //根据当前的顶点or索引buffer 进行顶点和索引绑定和绘制调用
        if (caller) {
            caller->execute(app, buffer, current, total);
        }
        for (auto currBuffer : buffers) {
            currBuffer->render(buffer);
        }
    }

    void release()override {
        if (pipeline) {
            vkDestroyPipeline(app->getDevice()->getLogicalDevice(), pipeline, nullptr);
            pipelineLayout->release();
            descriptorSetLayout->release();
            descriptorPool->release();
        }
        //pipeline = nullptr;
    }

public:
    void initVertexInputStateCreateInfo(VKShaderSet* shaderSet)override {
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexAttributeDescriptionCount = shaderSet->getVertexAttributeDescriptionCount();
        vertexInputInfo.pVertexAttributeDescriptions = shaderSet->getVertexAttributeDescriptionData();
        vertexInputInfo.vertexBindingDescriptionCount = shaderSet->getVertexInputBindingDescriptionCount();
        vertexInputInfo.pVertexBindingDescriptions = shaderSet->getVertexInputBindingDescriptionData();
        vertexInputInfo.flags = 0;
        setVertexInputStateCreateInfo(vertexInputInfo);
    }
    void initMultisampleStateCreateInfo(VkSampleCountFlagBits sampleCount) override {
        VkPipelineMultisampleStateCreateInfo multisampling = {};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = sampleCount;
        //multisampling.minSampleShading = 1.0f; // Optional
        //multisampling.pSampleMask = nullptr; // Optional
        //multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
        //multisampling.alphaToOneEnable = VK_FALSE; // Optional
        setMultisampleStateCreateInfo(multisampling);
    }
    void initColorBlendStateCreateInfo() override {
        VkPipelineColorBlendStateCreateInfo colorBlending = {};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f; // Optional
        colorBlending.blendConstants[1] = 0.0f; // Optional
        colorBlending.blendConstants[2] = 0.0f; // Optional
        colorBlending.blendConstants[3] = 0.0f; // Optional
        setColorBlendStateCreateInfo(colorBlending);
    }
    void initInputAssemblyStateCreateInfo() override {
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;
        setInputAssemblyStateCreateInfo(inputAssembly);
    }
    void initRasterizationStateCreateInfo() override {
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f; // Optional
        rasterizer.depthBiasClamp = 0.0f; // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
        setRasterizationStateCreateInfo(rasterizer);
    }
    void initDepthStencilStateCreateInfo() override {
        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;//新深度更小才更新
        depthStencil.depthBoundsTestEnable = VK_FALSE; //绑定深度测试范围，不用
        depthStencil.maxDepthBounds = 1.0f;
        depthStencil.minDepthBounds = 0.0f;
        depthStencil.stencilTestEnable = VK_FALSE;//模板测试，不用
        depthStencil.back = {};
        depthStencil.front = {};
        setDepthStencilStateCreateInfo(depthStencil);
    }
    void initColorBlendAttachmentState() override {
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
    }
    void initViewportStateCreateInfo(VkExtent2D extent)override {
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)extent.width;
        viewport.height = (float)extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = extent;

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;
        setViewportStateCreateInfo(viewportState);
        return;
    }
    
    bool createPipeline(VkPipelineCreateFlagBits flag) override {
        descriptorSetLayout = new DescriptorSetLayout(app, shaderSet);
        pipelineLayout->create(descriptorSetLayout->getDescriptorSetLayout());
        
        //size_t poolSize = app->getSwapChain()->getSwapChainSize();
        //shaderSet->updateDescriptorPoolSize(poolSize);
        descriptorPool = new DescriptorPool(app);
        descriptorPool->create(shaderSet);

        descriptorSets->init(descriptorPool->getDescriptorPool(),
            descriptorSetLayout->getDescriptorSetLayout());
        
        shaderSet->updateDescriptorSet(descriptorSets);

        auto vertexInputState = getVertexInputStateCreateInfo();
        auto inputAssemblyState = getInputAssemblyStateCreateInfo();
        auto rasterizationState = getRasterizationStateCreateInfo();
        auto colorBlendState = getColorBlendStateCreateInfo();
        auto depthStencilState = getDepthStencilStateCreateInfo();
        auto viewportState = getViewportStateCreateInfo();
        auto multiSampleState = getMultisampleStateCreateInfo();

        auto dynamicState = getDynamicState()->createDynamicStateCreateInfo(0);
        auto parentPipeline =  parent ? parent->pipeline : nullptr;

        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.flags = flag;
        pipelineCreateInfo.stageCount = shaderSet->getCreateInfoCount();
        pipelineCreateInfo.pStages = shaderSet->getCreateInfoData();

        pipelineCreateInfo.pVertexInputState = &vertexInputState;
        pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
        pipelineCreateInfo.pRasterizationState = &rasterizationState;
        pipelineCreateInfo.pColorBlendState = &colorBlendState;
        pipelineCreateInfo.pDepthStencilState = &depthStencilState;
        pipelineCreateInfo.pViewportState = &viewportState;
        pipelineCreateInfo.pMultisampleState = &multiSampleState;
        pipelineCreateInfo.pDynamicState = &dynamicState;

        pipelineCreateInfo.layout = pipelineLayout->getPipelineLayout();
        pipelineCreateInfo.basePipelineHandle = parentPipeline;
        pipelineCreateInfo.basePipelineIndex = parentPipeline == VK_NULL_HANDLE ? 0:-1;
        pipelineCreateInfo.renderPass = app->getRenderPass()->getRenderPass();
        pipelineCreateInfo.subpass = 0;
        //销毁着色器模型
        VkDevice device = app->getDevice()->getLogicalDevice();
        auto pipelineCache = app->getPipelineCache()->getPipelineCache();
        if (vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }
        return true;
    }

public:
    VKApp* app = nullptr;
    VKShaderSet* shaderSet = nullptr;
    Pipeline* parent = nullptr;

    DescriptorSetLayout* descriptorSetLayout = nullptr;
    std::shared_ptr<VKPipelineLayout> pipelineLayout;
    DescriptorPool* descriptorPool = nullptr;
    std::shared_ptr<DescriptorSets> descriptorSets;

    VkPipeline pipeline = nullptr;
    std::shared_ptr<VKPushDescriptor> pushDescriptors;

    std::optional<VkPipelineVertexInputStateCreateInfo> vertexInputStateCreateInfo;
    std::optional<VkPipelineInputAssemblyStateCreateInfo> inputAssemblyStateCreateInfo;
    std::optional<VkPipelineRasterizationStateCreateInfo> rasterizationStateCreateInfo;
    std::optional<VkPipelineDepthStencilStateCreateInfo> depthStencilStateCreateInfo;
    std::optional<VkPipelineMultisampleStateCreateInfo> multiSampleStateCreateInfo;
    std::optional<VkPipelineViewportStateCreateInfo> viewportStateCreateInfo;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    std::optional<VkPipelineColorBlendStateCreateInfo> colorBlendStateCreateInfo;

    std::optional<VkPipelineTessellationStateCreateInfo> tessellationStateCreateInfo;
    DynamicState* vkDynamicState = nullptr;

    VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
    bool needUpdate = true;

    std::list<BufferBase*> buffers;
};

#endif // VK_PIPLELINE_H